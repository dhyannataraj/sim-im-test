/***************************************************************************
                          proxy.h  -  description
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

#ifndef _PROXY_H
#define _PROXY_H

#include "socket.h"

const unsigned PROXY_NONE = 0;
const unsigned PROXY_SOCKS4	= 1;
const unsigned PROXY_SOCKS5 = 2;
const unsigned PROXY_HTTP = 3;
#ifdef USE_OPENSSL
const unsigned PROXY_HTTPS = 4;
#endif

class Proxy : public SocketNotify, public Socket
{
public:
    Proxy();
    ~Proxy();
    Socket *socket() { return sock; }
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    void setSocket(Socket*);
    virtual void close();
    virtual unsigned long localHost();
    virtual void pause(unsigned);
protected:
    virtual void write_ready();
    virtual void error_state(SocketError);
    virtual void proxy_connect_ready();
    void read(unsigned size, unsigned minsize=0);
    void write();
    Socket *sock;
    Buffer bOut;
    Buffer bIn;
};

class SOCKS4_Proxy : public Proxy
{
public:
    SOCKS4_Proxy(const char *host, unsigned short port);
    virtual void connect(const char *host, int port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    string m_host;
    unsigned short m_port;
    string m_connectHost;
    unsigned short m_connectPort;
    enum State
    {
        None,
        Connect,
        WaitConnect
    };
    State state;
};

class SOCKS5_Proxy : public Proxy
{
public:
    SOCKS5_Proxy(const char *host, unsigned short port, const char *user, const char *passwd);
    virtual void connect(const char *host, int port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    string m_host;
    unsigned short m_port;
    string m_user;
    string m_passwd;
    string m_connectHost;
    unsigned short m_connectPort;
    enum State
    {
        None,
        Connect,
        WaitAnswer,
        WaitAuth,
        WaitConnect
    };
    State state;
    void send_connect();
};

class HTTP_Proxy : public Proxy
{
public:
    HTTP_Proxy(const char *host, unsigned short port, const char *user, const char *passwd);
    virtual void connect(const char *host, int port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    string m_host;
    unsigned short m_port;
    string m_user;
    string m_passwd;
    string m_connectHost;
    unsigned short m_connectPort;
    enum State
    {
        None,
        Connect,
        WaitConnect,
        WaitEmpty
    };
    State state;
    bool readLine(string &s);
};

#ifdef USE_OPENSSL

class HTTPS_Proxy : public HTTP_Proxy
{
public:
    HTTPS_Proxy(const char *host, unsigned short port, const char *user, const char *passwd);
protected:
    virtual void connect_ready();
    virtual void proxy_connect_ready();
    enum State
    {
        Connect,
        SSLconnect
    };
    State state;
};

#endif
#endif
