/***************************************************************************
                          socket.cpp  -  description
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

#include "socket.h"
#include "proxy.h"
#include "icqhttp.h"
#include "log.h"

#ifndef INADDR_NONE
#define INADDR_NONE	0xFFFFFFFF
#endif

Socket::Socket()
{
    notify = NULL;
}

ServerSocket::ServerSocket()
{
    notify = NULL;
}

ClientSocket::ClientSocket(ClientSocketNotify *n, SocketFactory *f)
{
    notify = n;
    factory = f;
    bRawMode = false;
    bClosed  = false;
    m_sock = f->createSocket();
    m_sock->setNotify(this);
    m_proxy = NULL;
    mError = ErrorNone;
}

ClientSocket::~ClientSocket()
{
    setProxy(NULL);
    if (m_sock) delete m_sock;
    if (mError == ErrorNone) return;
    factory->errSockets.remove(this);
}

void ClientSocket::close()
{
    setProxy(NULL);
    m_sock->close();
    bClosed = true;
}

void ClientSocket::setProxy(Proxy *p)
{
    if (m_proxy){
        setSocket(m_proxy->socket());
        delete m_proxy;
        m_proxy = NULL;
    }
    if (p){
        m_proxy = p;
        m_proxy->setSocket(m_sock);
        setSocket(m_proxy);
    }
}

bool ClientSocket::isError()
{
    return (mError != ErrorNone);
}

void ClientSocket::setProxyConnected()
{
    setProxy(NULL);
}

void ClientSocket::error_state(SocketError err)
{
    switch (err){
    case ErrorSocket:
        log(L_WARN, "Socket error");
        break;
    case ErrorConnect:
        log(L_WARN, "Connect error");
        break;
    case ErrorRead:
        log(L_WARN, "Read error");
        break;
    case ErrorWrite:
        log(L_WARN, "Write error");
        break;
    case ErrorConnectionClosed:
        log(L_WARN, "Connection closed");
        break;
    case ErrorProtocol:
        log(L_WARN, "Protocol error");
        break;
    case ErrorProxyAuth:
        log(L_WARN, "Proxy auth error");
        break;
    case ErrorProxyConnect:
        log(L_WARN, "Proxy connect error");
        break;
    case ErrorCancel:
        break;
    case ErrorNone:
        return;
    }
    mError = err;
    list<ClientSocket*>::iterator it;
    for (it = factory->errSockets.begin(); it != factory->errSockets.end(); ++it)
        if ((*it) == this) return;
    factory->errSockets.push_back(this);
}

void ClientSocket::connect(const char *host, int port)
{
    m_sock->connect(host, port);
}

void ClientSocket::write()
{
    if (writeBuffer.size() == 0) return;
    m_sock->write(writeBuffer.Data(0), writeBuffer.size());
    writeBuffer.init(0);
}

bool ClientSocket::created()
{
    return (m_sock != NULL);
}

void ClientSocket::connect_ready()
{
    notify->connect_ready();
    bClosed = false;
}

void ClientSocket::setRaw(bool mode)
{
    bRawMode = mode;
    read_ready();
    if (mode) readBuffer.init(0);
}

void ClientSocket::read_ready()
{
    if (bRawMode){
        for (;;){
            char b[2048];
            int readn = m_sock->read(b, sizeof(b));
            if (readn == 0) break;
            readBuffer.setWritePos(readBuffer.writePos() + readn);
            if (readn < (int)sizeof(b)) break;
        }
        if (notify) notify->packet_ready();
        return;
    }
    for (;;){
        if (bClosed || (mError != ErrorNone)) break;
        int readn = m_sock->read(readBuffer.Data(readBuffer.writePos()),
                                 readBuffer.size() - readBuffer.writePos());
        if (readn < 0){
            error_state(ErrorRead);
            return;
        }
        if (readn == 0) break;
        readBuffer.setWritePos(readBuffer.writePos() + readn);
        if (readBuffer.writePos() < readBuffer.size()) break;
        if (notify) notify->packet_ready();
    }
}

void ClientSocket::write_ready()
{
    notify->write_ready();
}

unsigned long ClientSocket::localHost()
{
    return m_sock->localHost();
}

void ClientSocket::pause(unsigned n)
{
    m_sock->pause(n);
}

void ClientSocket::setSocket(Socket *s)
{
    m_sock = s;
    s->setNotify(this);
}

SocketFactory::SocketFactory()
{
    MinTCPPort = 1024;
    MaxTCPPort = 0xFFFF;
    ProxyType = PROXY_NONE;
    ProxyHost = "proxy";
    ProxyPort = 1080;
    ProxyAuth = false;
    isActive = false;
}

bool SocketFactory::isHttpProxy()
{
    if (ProxyType == PROXY_HTTP) return true;
#ifdef USE_OPENSSL
    if (ProxyType == PROXY_HTTPS) return true;
#endif
    return false;
}

Proxy *SocketFactory::getProxy()
{
    switch (ProxyType){
    case PROXY_NONE:
        return 0;
    case PROXY_SOCKS4:
        return new SOCKS4_Proxy(ProxyHost.c_str(), ProxyPort);
    case PROXY_SOCKS5:
        return new SOCKS5_Proxy(ProxyHost.c_str(), ProxyPort,
                                ProxyAuth ? ProxyUser.c_str() : "",
                                ProxyAuth ? ProxyPasswd.c_str() : "");
    case PROXY_HTTP:
        return new ICQ_HTTP_Proxy(this,
                                  ProxyHost.c_str(), ProxyPort,
                                  ProxyAuth ? ProxyUser.c_str() : "",
                                  ProxyAuth ? ProxyPasswd.c_str() : "");
#ifdef USE_OPENSSL
    case PROXY_HTTPS:
        return new ICQ_HTTPS_Proxy(this,
                                   ProxyHost.c_str(), ProxyPort,
                                   ProxyAuth ? ProxyUser.c_str() : "",
                                   ProxyAuth ? ProxyPasswd.c_str() : "");
#endif
    default:
        log(L_WARN, "Unknown proxy type");
    }
    return NULL;
}

void SocketFactory::remove(Socket *s)
{
    s->setNotify(NULL);
    s->close();
    for (list<Socket*>::iterator it = removedSockets.begin(); it != removedSockets.end(); ++it)
        if ((*it) == s) return;
    removedSockets.push_back(s);
}

void SocketFactory::idle()
{
    for (list<ClientSocket*>::iterator it = errSockets.begin(); it != errSockets.end();){
        ClientSocket *s = *it;
        ClientSocketNotify *n = s->notify;
        if (n){
            SocketError err = s->mError;
            s->mError = ErrorNone;
            if (n->error_state(err))
                delete n;
        }else{
            delete s;
        }
        errSockets.remove(s);
        it = errSockets.begin();
    }
    for (list<Socket*>::iterator its = removedSockets.begin(); its != removedSockets.end(); ++its)
        delete *its;
    removedSockets.clear();
}

