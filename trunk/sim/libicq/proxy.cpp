/***************************************************************************
                          proxy.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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
#include "icqssl.h"
#include "log.h"

Proxy::Proxy() : Socket(NULL)
{
}

Proxy::~Proxy()
{
}

void Proxy::setSocket(Socket *s)
{
    sock = s;
    sock->setNotify(this);
}

int Proxy::read(char*, unsigned int)
{
    return 0;
}

void Proxy::write(const char *b, unsigned int size)
{
    writeBuffer.pack(b, size);
}

void Proxy::close()
{
    sock->close();
}

unsigned long Proxy::localHost()
{
    return sock->localHost();
}

void Proxy::pause(unsigned n)
{
    sock->pause(n);
}

void Proxy::write_ready()
{
}

void Proxy::error_state(SocketError)
{
    notify->error_state(ErrorProxyConnect);
}

void Proxy::read(unsigned size, unsigned minsize)
{
    bIn.init(size);
    int readn = sock->read(bIn.Data(0), size);
    if ((readn != size) || (minsize && (readn < minsize))){
        notify->error_state(ErrorProxyConnect);
        return;
    }
    dumpPacket(bIn, 0, "Proxy read");
}

void Proxy::write()
{
    dumpPacket(bOut, 0, "Proxy write");
    sock->write(bOut.Data(0), bOut.size());
    bOut.init(0);
}

void Proxy::proxy_connect_ready()
{
    if (writeBuffer.size())
        sock->write(writeBuffer.Data(0), writeBuffer.size());
    notify->connect_ready();
}

SOCKS4_Proxy::SOCKS4_Proxy(const char *host, unsigned short port)
{
    m_host = host;
    m_port = port;
    state = None;
}

void SOCKS4_Proxy::connect(const char *host, int port)
{
    if (state != None){
        log(L_WARN, "Proxy::connect in bad state");
        notify->error_state(ErrorProxyConnect);
        return;
    }
    m_connectHost = host;
    m_connectPort = port;
    log(L_DEBUG, "Connect to proxy %s:%u", m_host.c_str(), m_port);
    sock->connect(m_host.c_str(), m_port);
    state = Connect;
}

void SOCKS4_Proxy::connect_ready()
{
    if (state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        notify->error_state(ErrorProxyConnect);
        return;
    }
    unsigned long addr = inet_addr(m_connectHost.c_str());
    if (addr == INADDR_NONE){
        struct hostent *hp = gethostbyname(m_connectHost.c_str());
        if (hp) addr = *((unsigned long*)(hp->h_addr_list[0]));
    }
    bOut << (char)4
    << (char)1
    << m_connectPort
    << (unsigned long)htonl(addr)
    << (char)0;
    state = WaitConnect;
}

void SOCKS4_Proxy::read_ready()
{
    if (state != WaitConnect) return;
    read(9, 4);
    char b1, b2;
    bIn >> b1 >> b2;
    if (b2 != 90){
        notify->error_state(ErrorProxyConnect);
        return;
    }
    proxy_connect_ready();
}

SOCKS5_Proxy::SOCKS5_Proxy(const char *host, unsigned short port, const char *user, const char *passwd)
{
    m_host = host;
    m_port = port;
    m_user = user;
    m_passwd = passwd;
    state = None;
}

void SOCKS5_Proxy::connect(const char *host, int port)
{
    if (state != None){
        log(L_WARN, "Proxy::connect in bad state");
        notify->error_state(ErrorProxyConnect);
        return;
    }
    m_connectHost = host;
    m_connectPort = port;
    log(L_DEBUG, "Connect to proxy %s:%u", m_host.c_str(), m_port);
    sock->connect(m_host.c_str(), m_port);
    state = Connect;
}

void SOCKS5_Proxy::connect_ready()
{
    if (state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        notify->error_state(ErrorProxyConnect);
        return;
    }
    bOut << (char)0x05;
    if (m_user.length()) {
        bOut	<< (char)0x02
        << (char)0x00
        << (char)0x02;
    } else {
        bOut << (char) 0x01
        << (char)0x00;
    }
    state = WaitAnswer;
    write();
}

void SOCKS5_Proxy::read_ready()
{
    char b1, b2;
    switch (state){
    case WaitAnswer:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 == 0xff)) {
            notify->error_state(ErrorProxyConnect);
            return;
        }
        if (b2 == 0x02) {
            bOut.pack((char)0x01);
            bOut.pack((char)m_user.length());
            bOut.pack(m_user.c_str(), m_user.length());
            bOut.pack((char)m_passwd.length());
            bOut.pack(m_passwd.c_str(), m_passwd.length());
            state = WaitAuth;
            write();
            return;
        }
        send_connect();
        return;
    case WaitAuth:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x01) || (b2 != 0x00)) {
            notify->error_state(ErrorProxyAuth);
            return;
        }
        send_connect();
        return;
    case WaitConnect:
        read(10);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 != 0x00)) {
            notify->error_state(ErrorConnect);
            return;
        }
        proxy_connect_ready();
        return;
    default:
        break;
    }
}

void SOCKS5_Proxy::send_connect()
{
    unsigned long addr = inet_addr(m_connectHost.c_str());
    bOut << (char)0x05
    << (char)0x01			/* CONNECT */
    << (char)0x00;			/* reserved */
    if (addr != INADDR_NONE){
        bOut << (char)0x01	    /* address type -- ip addr */
        << (unsigned long)htonl(addr);
    }else{
        bOut << (char)0x03		/* address type -- host name */
        << (char)m_connectHost.length();
        bOut.pack(m_connectHost.c_str(), m_connectHost.length());
    }
    bOut << m_connectPort;
    state = WaitConnect;
    write();
}

HTTP_Proxy::HTTP_Proxy(const char *host, unsigned short port, const char *user, const char *passwd)
{
    m_host = host;
    m_port = port;
    m_user = user;
    m_passwd = passwd;
    state = None;
}

void HTTP_Proxy::connect(const char *host, int port)
{
    if (state != None){
        log(L_WARN, "Proxy::connect in bad state");
        notify->error_state(ErrorProxyConnect);
        return;
    }
    m_connectHost = host;
    m_connectPort = port;
    log(L_DEBUG, "Connect to proxy %s:%u", m_host.c_str(), m_port);
    sock->connect(m_host.c_str(), m_port);
    state = Connect;
}

static char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "0123456789+/";

static string tobase64(const char *text)
{
    string out;
    const char *c;
    unsigned int tmp = 0;
    int n = 0;

    c = text;

    while (*c) {
        tmp = tmp << 8;
        tmp += *c;
        n++;

        if (n == 3) {
            out += alphabet[(tmp >> 18) & 0x3f];
            out += alphabet[(tmp >> 12) & 0x3f];
            out += alphabet[(tmp >> 6) & 0x3f];
            out += alphabet[tmp & 0x3f];
            tmp = 0;
            n = 0;
        }
        c++;
    }

    switch (n) {
    case 2:
        tmp <<= 8;
        out += alphabet[(tmp >> 18) & 0x3f];
        out += alphabet[(tmp >> 12) & 0x3f];
        out += alphabet[(tmp >> 6) & 0x3f];
        out += '=';
        break;
    case 1:
        tmp <<= 16;
        out += alphabet[(tmp >> 18) & 0x3f];
        out += alphabet[(tmp >> 12) & 0x3f];
        out += '=';
        out += '=';
        break;
    }
    return out;
}

void HTTP_Proxy::connect_ready()
{
    if (state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        notify->error_state(ErrorProxyConnect);
        return;
    }
    char b[13];
    snprintf(b, sizeof(b), "%u", m_connectPort);
    bOut << "CONNECT "
    << m_connectHost.c_str()
    << ":"
    << b
    << " HTTP/1.1\r\n"
    << "Host: "
    << m_connectHost.c_str()
    << "\r\n";
    if (m_user.length()){
        string s;
        s = m_user.c_str();
        s += ":";
        s += m_passwd.c_str();
        s = tobase64(s.c_str());
        bOut << "Proxy-Auth: basic ";
        bOut << s.c_str();
        bOut << "\r\n";
    }
    bOut << "\r\n";
    state = WaitConnect;
    write();
}

static char HTTP[] = "HTTP/";

void HTTP_Proxy::read_ready()
{
    if (state == WaitConnect){
        string s;
        if (!readLine(s)) return;
        if (s.length() < strlen(HTTP)){
            notify->error_state(ErrorProxyConnect);
            return;
        }
        const char *r = strchr(s.c_str(), ' ');
        if (r == NULL){
            notify->error_state(ErrorProxyConnect);
            return;
        }
        r++;
        int code = atoi(r);
        if (code == 401){
            notify->error_state(ErrorProxyAuth);
            return;
        }
        if (code != 200){
            notify->error_state(ErrorProxyConnect);
            return;
        }
        state = WaitEmpty;
    }
    if (state == WaitEmpty){
        for (;;){
            string s;
            if (!readLine(s)) return;
            if (s.length() == 0) break;
        }
        proxy_connect_ready();
    }
}

bool HTTP_Proxy::readLine(string &s)
{
    for (;;){
        char c;
        int n = sock->read(&c, 1);
        if (n < 0){
            notify->error_state(ErrorProxyConnect);
            return false;
        }
        if (n == 0) return false;
        if (c == '\r') continue;
        if (c == '\n') break;
        bIn << c;
    }
    dumpPacket(bIn, 0, "Proxy read");
    s.assign(bIn.Data(0), bIn.size());
    bIn.init(0);
    return true;
}

HTTPS_Proxy::HTTPS_Proxy(const char *host, unsigned short port, const char *user, const char *passwd)
        : HTTP_Proxy(host, port, user, passwd)
{
    state = Connect;
}

void HTTPS_Proxy::connect_ready()
{
    if (state == Connect){
        SSLClient *ssl = new SSLClient(sock);
        if (!ssl->initHTTPS()){
            delete ssl;
            ssl = NULL;
            notify->error_state(ErrorProxyConnect);
            return;
        }
        setSocket(ssl);
        state = SSLconnect;
        ssl->connect();
        ssl->process();
        return;
    }
    HTTP_Proxy::connect_ready();
}

void HTTPS_Proxy::proxy_connect_ready()
{
    SSLClient *ssl = static_cast<SSLClient*>(sock);
    setSocket(ssl->socket());
    delete ssl;
    HTTP_Proxy::proxy_connect_ready();
}
