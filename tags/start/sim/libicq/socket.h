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

#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#if HAVE_INTTYPES_H
#include <inttypes.h>
#else
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "buffer.h"

class Socket;

#define PROXY_NONE		0
#define PROXY_SOCKS4	1
#define PROXY_SOCKS5	2
#define PROXY_HTTP		3

class Sockets
{
public:
    Sockets();
    virtual ~Sockets();
    virtual void createSocket(Socket*);
    virtual void closeSocket(Socket*);
    virtual void setHaveData(Socket*);
    void addSocket(Socket*);
    void delSocket(Socket*);
    static unsigned short proxyType;
    static string proxyHost;
    static unsigned short proxyPort;
    static bool proxyAuth;
    static string proxyUser;
    static string proxyPasswd;
protected:
    void process(unsigned timeout);
    list<Socket*> socket_list;
};

class Socket
{
public:
    enum SocketError {
        ErrorNone,
        ErrorConnected,
        ErrorCreate,
        ErrorBind,
        ErrorResolve,
        ErrorConnect,
        ErrorAccept,
        ErrorListen,
        ErrorDisconnect,
        ErrorRead,
        ErrorWrite,
        ErrorProtocol
    };

    enum ResolveState {
        None,
        Resolving,
        Done
    };

    enum ConnectState {
        NoConnect,
        Connected,
        ProxyResolve,
        HostResolve,
        Connecting,
        Socks4_Wait,
        Socks5_Wait,
        Socks5_WaitAuth,
        Socks5_WaitConnect,
        Http_Wait
    };

    Socket(int m_fd, const char *host, unsigned short port);
    virtual ~Socket();
    virtual void close();

    SocketError error() { return m_err; }
    const char *errorText();

    const char *host() { return m_szHost; }
    unsigned short port() { return m_nPort; }

    void remove();
    void setHost(const char*);

    bool getLocalAddr(char *&host, unsigned short &port);
    bool connected() { return (m_fd != -1); }

    void dumpPacket(Buffer &b, unsigned start, const char *oper);
    void *intData;
    int m_fd;
    virtual bool have_data() { return false; }
protected:
    virtual void idle() {}
    virtual void error_state();
    virtual void read_ready() = 0;
    virtual void write_ready() = 0;
    virtual void proxy_connect_ready();
    virtual void connect_ready();

    void error(SocketError err);

    char *m_szHost;
    char *m_szResolveHost;
    unsigned short m_nPort;
    SocketError m_err;
    bool m_delete;
    bool m_bConnecting;

    ResolveState m_resolving;
    ConnectState m_connecting;

    friend class Sockets;
};

class ServerSocket : public Socket
{
public:
    ServerSocket();
    bool listen(int minPort, int maxPort=0, const char *host=NULL);
protected:
    virtual void read_ready();
    virtual void write_ready();
    virtual void accept(int fd, const char *host, unsigned short port) = 0;
};

class ClientSocket : public Socket
{
public:
    ClientSocket(int fd, const char *host, unsigned short port);
    ~ClientSocket();
    virtual void close();
    void connect(const char *host, unsigned short port);
protected:
    unsigned long m_remoteAddr;
    unsigned long m_proxyAddr;
    void proxy_connect_ready();
    virtual void resolve();
    virtual void create_socket();
    Buffer readBuffer;
    Buffer writeBuffer;
    Buffer connectBuffer;
    virtual void packet_ready() = 0;
    virtual void read_ready();
    virtual void write_ready();
    virtual bool have_data();
    void setOpt();
    void s5connect();
};

#endif
