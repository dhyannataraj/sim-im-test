/***************************************************************************
                          sslclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "simapi.h"

#ifdef USE_OPENSSL

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>

#include "socket.h"

static void ssl_info_callback(const SSL *s, int where, int ret)
{
    const char *str;
    int w;

    w = where & ~SSL_ST_MASK;

    if (w & SSL_ST_CONNECT) str="SSL_connect";
    else if (w & SSL_ST_ACCEPT) str="SSL_accept";
    else str="undefined";

    if (where & SSL_CB_LOOP)
    {
        log(L_DEBUG, "SSL: %s", SSL_state_string_long((SSL*)s));
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        log(L_DEBUG, "SSL: SSL3 alert %s:%s:%s", str,
            SSL_alert_type_string_long(ret),
            SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            log(L_DEBUG, "SSL: %s:failed in %s",
                str,SSL_state_string_long((SSL*)s));
        else if (ret < 0)
        {
            log(L_DEBUG, "SSL: %s:%s", str,SSL_state_string_long((SSL*)s));
        }
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        log(L_DEBUG, "SSL: SSL3 alert %s:%s:%s", str,
            SSL_alert_type_string_long(ret),
            SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            log(L_DEBUG, "SSL: %s:failed in %s",
                str,SSL_state_string_long((SSL*)s));
        else if (ret < 0)
        {
            log(L_DEBUG, "SSL: %s:error in %s",
                str,SSL_state_string_long((SSL*)s));
        }
    }
}

static bool bInit = false;

static void initLib()
{
    if (!bInit){
        SSL_load_error_strings();
        SSL_library_init();
        bInit = true;
    }
}

SSLClient::SSLClient(Socket *_sock)
{
    sock = _sock;
    sock->setNotify(this);
    mpSSL = NULL;
    mrBIO = NULL;
    mwBIO = NULL;
    mpCTX = NULL;
    m_bSecure = false;
}

SSLClient::~SSLClient()
{
    free();
    if (sock){
        sock->close();
        delete sock;
    }
}

void SSLClient::free()
{
    if (pSSL != NULL)
        SSL_free(pSSL);
    if (pCTX != NULL)
        SSL_CTX_free(pCTX);
    mpSSL = NULL;
    mpCTX = NULL;
    mrBIO = NULL;
    mwBIO = NULL;
    m_bSecure = false;
}

bool SSLClient::initBIO()
{
    mrBIO = BIO_new(BIO_s_mem());
    mwBIO = BIO_new(BIO_s_mem());
    if ((mpSSL == NULL) || (mrBIO == NULL) || (mwBIO == NULL)){
        log(L_WARN, "SSL error");
        return false;
    }
    SSL_set_bio(pSSL, rBIO, wBIO);
    SSL_set_mode(pSSL, SSL_MODE_AUTO_RETRY);
    return true;
}

bool SSLClient::init()
{
    initLib();
    if (!initSSL())
        return false;
#if OPENSSL_VERSION_NUMBER < 0x00907000L        
    SSL_CTX_set_info_callback(pCTX, (void (*)())ssl_info_callback);
#else
    SSL_CTX_set_info_callback(pCTX, ssl_info_callback);
#endif
    return initBIO();
}

void SSLClient::process(bool bInRead)
{
    for (;;){
        switch (state){
        case SSLWrite:
            write();
            break;
        case SSLConnect:
            connect();
            break;
        case SSLAccept:
            accept();
            break;
        case SSLShutdown:
            shutdown();
            break;
        case SSLConnected:
            if (!bInRead && (SSL_pending(pSSL) > 0))
                notify->read_ready();
            break;
        }
        char b[2048];
        int i = BIO_read(wBIO, b, sizeof(b));
        if (i == 0) return;
        if (i > 0){
            sock->write(b, i);
            continue;
        }
        if (i < 0){
            if (!BIO_should_retry(wBIO))
                notify->error_state(I18N_NOOP("SSL write error"));
            return;
        }
    }
}

void SSLClient::connect()
{
    if (pSSL == NULL){
        notify->error_state(I18N_NOOP("SSL connect error"));
        return;
    }
    int i = SSL_connect(pSSL);
    int j = SSL_get_error(pSSL, i);
    if (j == SSL_ERROR_NONE)
    {
        m_bSecure = true;
        state = SSLConnected;
        notify->connect_ready();
        return;
    }
    const char *file;
    int line;
    unsigned long err;
    switch (j)
    {
    case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        log(L_WARN, "SSL: SSL_connect error = %lx, %s:%i", err, file, line);
        ERR_clear_error();
        notify->error_state(I18N_NOOP("SSL connect error"));
        return;
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_READ:
        state = SSLConnect;
        return;
    default:
        log(L_DEBUG, "SSL: SSL_connect error %d, SSL_%d", i, j);
        notify->error_state(I18N_NOOP("SSL connect error"));
    }
}

void SSLClient::shutdown()
{
    if (pSSL == NULL){
        notify->error_state(I18N_NOOP("SSL shutdown error"));
        return;
    }
    int i = SSL_shutdown(pSSL);
    int j = SSL_get_error(pSSL, i);
    if (j == SSL_ERROR_NONE)
    {
        SSL_free(pSSL);
        mpSSL = NULL;
        m_bSecure = false;
        return;
    }
    const char *file;
    int line;
    unsigned long err;
    switch (j)
    {
    case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        log(L_WARN, "SSL: SSL_shutdown error = %lx, %s:%i", err, file, line);
        ERR_clear_error();
        notify->error_state(I18N_NOOP("SSL shuwdown error"));
        return;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
        state = SSLShutdown;
        return;
    default:
        log(L_DEBUG, "SSL: SSL_shutdown error %d, SSL_%d", i, j);
        notify->error_state(I18N_NOOP("SSL shuwdown error"));
    }
}

void SSLClient::accept()
{
    if (pSSL == NULL){
        notify->error_state(I18N_NOOP("SSL accept error"));
        return;
    }
    int i = SSL_accept(pSSL);
    int j = SSL_get_error(pSSL, i);
    if (j == SSL_ERROR_NONE)
    {
        m_bSecure = true;
        notify->connect_ready();
        state = SSLConnected;
        return;
    }
    const char *file;
    int line;
    unsigned long err;
    switch (j)
    {
    case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        log(L_WARN, "SSL: SSL_accept error = %lx, %s:%i", err, file, line);
        ERR_clear_error();
        notify->error_state(I18N_NOOP("SSL accept error"));
        return;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
        state = SSLAccept;
        return;
    default:
        log(L_DEBUG, "SSL: SSL_accept error %d, SSL_%d", i, j);
        notify->error_state(I18N_NOOP("SSL accept error"));
    }
}

int SSLClient::read(char *buf, unsigned int size)
{
    if (state != SSLConnected) return 0;
    int nBytesReceived = SSL_read(pSSL, buf, size);
    int tmp = SSL_get_error(pSSL, nBytesReceived);
    const char *file;
    int line;
    unsigned long err;
    switch (tmp){
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_X509_LOOKUP:
        break;
    case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        log(L_WARN, "SSL: SSL_read error = %lx, %s:%i", err, file, line);
        ERR_clear_error();
        notify->error_state(I18N_NOOP("SSL read error"));
        return -1;
    default:
        log(L_DEBUG, "SSL: SSL_read error %d, SSL_%d", nBytesReceived, tmp);
        notify->error_state(I18N_NOOP("SSL read error"));
        return -1;
    }
    process(true);
    if (nBytesReceived < 0) nBytesReceived = 0;
    return nBytesReceived;
}

void SSLClient::write(const char *buf, unsigned int size)
{
    wBuffer.pack(buf, size);
    state = SSLWrite;
    process();
}

void SSLClient::write()
{
    int nBytesSend = SSL_write(pSSL, wBuffer.data(), wBuffer.size());
    int tmp = SSL_get_error(pSSL, nBytesSend);
    const char *file;
    int line;
    unsigned long err;
    switch (tmp){
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_X509_LOOKUP:
        break;
    case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        log(L_WARN, "SSL: SSL_write error = %lx, %s:%i", err, file, line);
        ERR_clear_error();
        notify->error_state(I18N_NOOP("SSL write error"));
        return;
    default:
        log(L_DEBUG, "SSL: SSL_write error %d, SSL_%d", nBytesSend, tmp);
        notify->error_state(I18N_NOOP("SSL write error"));
        return;
    }
    if (nBytesSend > 0)
        wBuffer.incReadPos(nBytesSend);
    if (wBuffer.readPos() == wBuffer.writePos()){
        wBuffer.init(0);
        state = SSLConnected;
    }
    process();
}

void SSLClient::connect(const char *host, int port)
{
    sock->connect(host, port);
}

void SSLClient::close()
{
    if (pSSL)
        shutdown();
    free();
    sock->close();
}

unsigned long SSLClient::localHost()
{
    return sock->localHost();
}

void SSLClient::pause(unsigned n)
{
    sock->pause(n);
}

void SSLClient::connect_ready()
{
    if (!init())
        if (notify) notify->error_state(I18N_NOOP("SSL init error"));
    connect();
}

void SSLClient::read_ready()
{
    for (;;){
        char b[2048];
        int n = sock->read(b, sizeof(b));
        if (n == -1){
            if (notify) notify->error_state(I18N_NOOP("SSL read error"));
            return;
        }
        if (n == 0) break;
        n = BIO_write(rBIO, b, n);
        if (n == -1)
            if (notify) notify->error_state(I18N_NOOP("SSL read error"));
        process();
    }
    if (state == SSLConnected)
        if (notify) notify->read_ready();
}

void SSLClient::write_ready()
{
}

void SSLClient::error_state(const char *err, unsigned code)
{
    if (notify) notify->error_state(err, code);
}

void SSLClient::setSocket(Socket *s)
{
    sock = s;
}

#endif



