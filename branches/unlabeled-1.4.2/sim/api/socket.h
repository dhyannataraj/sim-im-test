/***************************************************************************
                          socket.h  -  description
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

#ifndef _SOCKET_H
#define _SOCKET_H

#include "simapi.h"
#include "buffer.h"

namespace SIM
{

const unsigned LOGIN_ERROR = 401;

class EXPORT SocketNotify
{
public:
    SocketNotify() {}
    virtual ~SocketNotify() {}
    virtual void connect_ready() = 0;
    virtual void read_ready() = 0;
    virtual void write_ready() = 0;
    virtual void error_state(const char *err_text, unsigned code=0) = 0;
};

class EXPORT Socket
{
public:
    Socket();
    virtual ~Socket() {}
    virtual int read(char *buf, unsigned int size) = 0;
    virtual void write(const char *buf, unsigned int size) = 0;
    virtual void connect(const char *host, int port) = 0;
    virtual void close() = 0;
    virtual unsigned long localHost() = 0;
    virtual void pause(unsigned) = 0;
    void error(const char *err_text, unsigned code=0);
    void setNotify(SocketNotify *n) { notify = n; }
    enum Mode
    {
        Direct,
        Indirect,
        Web
    };
    virtual Mode mode() { return Direct; }
    SocketNotify *notify;
};

class EXPORT ServerSocketNotify
{
public:
    ServerSocketNotify() {}
    virtual ~ServerSocketNotify() {}
    virtual void accept(Socket*, unsigned long ip) = 0;
};

class EXPORT ServerSocket
{
public:
    ServerSocket();
    virtual ~ServerSocket() {}
    virtual unsigned short port() = 0;
    void setNotify(ServerSocketNotify *n) { notify = n; }
protected:
    ServerSocketNotify *notify;
};

class ClientSocket;

class SocketFactoryPrivate;

class EXPORT SocketFactory : public QObject
{
    Q_OBJECT
public:
    SocketFactory();
    virtual ~SocketFactory();
    virtual Socket *createSocket() = 0;
    virtual ServerSocket *createServerSocket() = 0;
    void remove(Socket*);
    void setActive(bool);
    bool isActive();
    unsigned short      MinTCPPort;
    unsigned short      MaxTCPPort;
protected slots:
    void idle();
protected:
    bool m_bActive;
    SocketFactoryPrivate *p;
    friend class ClientSocket;
};

SocketFactory EXPORT *getSocketFactory();

class EXPORT ClientSocketNotify
{
public:
    ClientSocketNotify() {}
    virtual ~ClientSocketNotify() {}
    virtual bool error_state(const char *err, unsigned code) = 0;
    virtual void connect_ready() = 0;
    virtual void packet_ready() = 0;
    virtual void write_ready() {}
};

class EXPORT ClientSocket : public SocketNotify
{
public:
    ClientSocket(ClientSocketNotify*);
    ~ClientSocket();
    Buffer readBuffer;
    Buffer writeBuffer;
    virtual void error_state(const char *err, unsigned code = 0);
    void connect(const char *host, int port, const char *proto);
    void write();
    void pause(unsigned);
    unsigned long localHost();
    bool created();
    virtual void read_ready();
    void close();
    void setRaw(bool mode);
    Socket *socket() { return m_sock; }
    void setSocket(Socket *s);
    void setNotify(ClientSocketNotify*);
    const char *errorString();
protected:
    virtual void connect_ready();
    virtual void write_ready();

    Socket *m_sock;
    ClientSocketNotify *m_notify;
    bool bRawMode;
    bool bClosed;

    unsigned	errCode;
    string		errString;
    friend class SocketFactory;
};

const unsigned NO_RECONNECT = (unsigned)(-1);

class EXPORT TCPClient : public QObject, public Client, public ClientSocketNotify
{
    Q_OBJECT
public:
    TCPClient(Protocol *protocol, const char *cfg);
    virtual const char		*getServer() = 0;
    virtual unsigned long	getPort() = 0;
protected slots:
    void reconnect();
protected:
    virtual void	setStatus(unsigned status) = 0;
    virtual void	disconnected() = 0;

    virtual void	setStatus(unsigned status, bool bCommon);
    virtual void	connect_ready();
    virtual bool	error_state(const char *err, unsigned code);
    void			setClientStatus(unsigned status);
    ClientSocket	*m_socket;
    unsigned		m_reconnectTime;
    unsigned		m_logonStatus;
    QTimer			*m_timer;
    bool			m_bWaitReconnect;
};

#ifdef USE_OPENSSL

class EXPORT SSLClient : public SocketNotify, public Socket
{
public:
    SSLClient(Socket*);
    ~SSLClient();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    virtual void connect(const char *host, int port);
    virtual void close();
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    bool connected() { return m_bSecure; }
    Socket *socket() { return sock; }
    void setSocket(Socket *s);
    bool init();
    void accept();
    void connect();
    void shutdown();
    void process(bool bInRead=false);
    void write();
    void clear();
protected:
    virtual bool initSSL() = 0;
    bool initBIO();
    Buffer wBuffer;
    virtual void connect_ready();
    virtual void read_ready();
    virtual void write_ready();
    virtual void error_state(const char *err, unsigned code);
    Socket *sock;
    enum State
    {
        SSLAccept,
        SSLConnect,
        SSLShutdown,
        SSLWrite,
        SSLConnected
    };
    State state;
    bool m_bSecure;
    void *mpCTX;
    void *mpSSL;
    void *mrBIO;
    void *mwBIO;
#define pCTX	((SSL_CTX*)mpCTX)
#define pSSL	((SSL*)mpSSL)
#define rBIO	((BIO*)mrBIO)
#define wBIO	((BIO*)mwBIO)
};

#endif

};

#endif
