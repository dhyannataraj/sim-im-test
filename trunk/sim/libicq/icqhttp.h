/***************************************************************************
                          icqhttp.h  -  description
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

#ifndef _ICQHTTP_H
#define _ICQHTTP_H

#include "proxy.h"

class HttpPacket;
class HttpRequest;

class ICQ_HTTP_Proxy : public Proxy
{
public:
    ICQ_HTTP_Proxy(SocketFactory *factory, const char *host, unsigned short port, const char *user, const char *passwd);
    ~ICQ_HTTP_Proxy();
    virtual void connect(const char *host, int port);
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
protected:
    virtual bool isSecure();
    enum State
    {
        None,
        Connected
    };
    State state;
    virtual void request();
    virtual void read_ready();
    virtual void connect_ready();

    string m_host;
    unsigned short m_port;
    unsigned data_size;
    string m_user;
    string m_passwd;
    string sid;
    string m_proxyHost;
    list<HttpPacket*> queue;
    unsigned seq;
    unsigned readn;
    Buffer readData;

    HttpRequest *hello;
    HttpRequest *monitor;
    HttpRequest *post;
    SocketFactory *factory;

    unsigned long nSock;

    friend class HttpRequest;
    friend class HelloRequest;
    friend class MonitorRequest;
    friend class PostRequest;
};

#ifdef USE_OPENSSL

class ICQ_HTTPS_Proxy : public ICQ_HTTP_Proxy
{
public:
    ICQ_HTTPS_Proxy(SocketFactory *factory, const char *host, unsigned short port, const char *user, const char *passwd);
protected:
    bool isSecure();
};

#endif
#endif
