/***************************************************************************
                          socket.h  -  description
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

#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "buffer.h"

enum SocketError
{
    ErrorSocket,
    ErrorConnect,
    ErrorRead,
    ErrorWrite,
    ErrorConnectionClosed,
    ErrorProtocol,
    ErrorProxyAuth,
    ErrorProxyConnect,
};

class SocketNotify
{
public:
    SocketNotify() {}
    virtual ~SocketNotify() {}
    virtual void connect_ready() = 0;
    virtual void read_ready() = 0;
    virtual void write_ready() = 0;
    virtual void error_state(SocketError) = 0;
};

class Socket
{
public:
    Socket(SocketNotify*);
    virtual ~Socket() {}
    virtual int read(char *buf, unsigned int size) = 0;
    virtual void write(const char *buf, unsigned int size) = 0;
    virtual void connect(const char *host, int port) = 0;
    virtual void close() = 0;
    virtual unsigned long localHost() = 0;
    virtual void pause(unsigned) = 0;
    void error(SocketError err=ErrorProtocol);
    void setNotify(SocketNotify *n) { notify = n; }
protected:
    SocketNotify *notify;
};

class ServerSocketNotify
{
public:
    ServerSocketNotify() {}
    virtual ~ServerSocketNotify() {}
    virtual void accept(int fd) = 0;
};

class ServerSocket
{
public:
    ServerSocket(ServerSocketNotify*);
    virtual ~ServerSocket() {}
    virtual unsigned short port() = 0;
protected:
    ServerSocketNotify *notify;
};

class SocketFactory
{
public:
    SocketFactory() {}
    virtual ~SocketFactory() {}
    virtual Socket *createSocket(SocketNotify*, int fd=-1) = 0;
    virtual ServerSocket *createServerSocket(ServerSocketNotify*) = 0;
    void removeSocket(Socket*);
    void idle();
    list<SocketNotify*> removedNotifies;
};

class ClientSocketNotify
{
public:
    ClientSocketNotify() {}
    virtual ~ClientSocketNotify() {}
    virtual void error_state(SocketError) = 0;
    virtual void connect_ready() = 0;
    virtual void packet_ready() = 0;
    virtual void write_ready() {}
};

class Proxy;

class ClientSocket : public SocketNotify
{
public:
    ClientSocket(ClientSocketNotify*, SocketFactory*, int fd=-1);
    ~ClientSocket();
    Buffer readBuffer;
    Buffer writeBuffer;
    void error(SocketError err=ErrorProtocol);
    void connect(const char *host, int port);
    void write();
    void pause(unsigned);
    unsigned long localHost();
    bool created();
    virtual void read_ready();
    void close();
    void remove();
    void setRaw(bool mode);

    Socket *socket() { return m_sock; }

    void setSocket(Socket *s);
    void setProxy(Proxy *proxy);
    void setProxyConnected();

protected:
    virtual void connect_ready();
    virtual void write_ready();
    virtual void error_state(SocketError);

    Socket *m_sock;
    Proxy  *m_proxy;

    ClientSocketNotify *notify;
    SocketFactory *factory;
    bool bRawMode;
};

#endif
