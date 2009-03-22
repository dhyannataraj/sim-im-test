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

#include <vector>
#include <qobject.h>
#include "buffer.h"
#include "contacts.h"
#include "event.h"
#include "simapi.h"

class QTimer;

#ifdef ENABLE_OPENSSL
typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
typedef struct bio_st BIO;
#endif

namespace SIM
{

class EXPORT SocketNotify
{
public:
    SocketNotify() {}
    virtual ~SocketNotify() {}
    virtual void connect_ready() = 0;
    virtual void read_ready() = 0;
    virtual void write_ready() = 0;
    virtual void error_state(const QString &err_text, unsigned code = 0) = 0;
    virtual void resolve_ready(unsigned long) {}
};

class EXPORT Socket
{
protected:
    QString m_host;
    unsigned short m_port;
    SocketNotify *notify;
public:
    Socket() : m_port(0), notify(NULL) {}
    virtual ~Socket() {}
    virtual int read(char *buf, unsigned int size) = 0;
    virtual void write(const char *buf, unsigned int size) = 0;
    virtual void connect(const QString &host, unsigned short port)
    {
        m_host = host;
        m_port = port;
    }
    virtual void close() = 0;
    virtual unsigned long localHost() = 0;
    virtual void pause(unsigned) = 0;
    void error(const QString &err_text, unsigned code=0);
    void setNotify(SocketNotify *n) { notify = n; }
    enum Mode
    {
        Direct,
        Indirect,
        Web
    };
    virtual Mode mode() const { return Direct; }
    const QString &getHost() const { return m_host; }
    unsigned short getPort() const { return m_port; }
    const SocketNotify *getNotify() const { return notify; }
	virtual int getFd() { return 0;}; // kind of hack
};

class ServerSocket;
class TCPClient;

class EXPORT ServerSocketNotify
{
public:
    ServerSocketNotify();
    virtual ~ServerSocketNotify();
    virtual bool accept(Socket*, unsigned long ip) = 0;
    virtual void bind_ready(unsigned short port) = 0;
    virtual bool error(const QString &err) = 0;
    virtual void bind(unsigned short mixPort, unsigned short maxPort, TCPClient *client);
#ifndef WIN32
    virtual void bind(const char *path);
#endif
    void setListener(ServerSocket *listener);
    ServerSocket *m_listener;
};

class EXPORT ServerSocket
{
public:
    ServerSocket();
    virtual ~ServerSocket() {}
    void setNotify(ServerSocketNotify *n) { notify = n; }
    virtual void bind(unsigned short mixPort, unsigned short maxPort, TCPClient *client) = 0;
#ifndef WIN32
    virtual void bind(const char *path) = 0;
#endif
    virtual void close() = 0;
protected:
    ServerSocketNotify *notify;
};

class ClientSocket;

class EXPORT SocketFactory : public QObject
{
    Q_OBJECT
public:
    SocketFactory(QObject *parent);
    virtual ~SocketFactory();
    virtual Socket *createSocket() = 0;
    virtual ServerSocket *createServerSocket() = 0;
    void remove(Socket*);
    void remove(ServerSocket*);
    bool add(ClientSocket *s);
    bool erase(ClientSocket *s);
    void setActive(bool);
    bool isActive() const;
    virtual void checkState() {}
protected slots:
    void idle();
private:
    struct SocketFactoryPrivate *d;

    COPY_RESTRICTED(SocketFactory)
};

SocketFactory EXPORT *getSocketFactory();

class EXPORT ClientSocketNotify
{
public:
    ClientSocketNotify() {}
    virtual ~ClientSocketNotify() {}
    virtual bool error_state(const QString &err, unsigned code = 0) = 0;
    virtual void connect_ready() = 0;
    virtual void packet_ready() = 0;
    virtual void write_ready() {}
    virtual void resolve_ready(unsigned long) {}
};

class TCPClient;

class EXPORT ClientSocket : public SocketNotify
{
public:
    ClientSocket(ClientSocketNotify*, Socket *sock=NULL);
    ~ClientSocket();

    virtual void error_state(const QString &err, unsigned code = 0);
    void connect(const QString &host, unsigned short port, TCPClient *client);
	void connect(unsigned long ip, unsigned short port, TCPClient* client);
    void write();
    void pause(unsigned);
    unsigned long localHost();
    bool created();
    virtual void read_ready();
    void close();
    void setRaw(bool mode);
    Socket *socket() const { return m_sock; }
    void setSocket(Socket *s, bool bClearError = true);
    void setNotify(ClientSocketNotify *n) { m_notify = n; }
    const QString &errorString() const;
    virtual Buffer &readBuffer() { return m_readBuffer; }
    virtual Buffer &writeBuffer() { return m_writeBuffer; }
protected:
    virtual void connect_ready();
    virtual void write_ready();
    virtual void resolve_ready(unsigned long ip);

    Socket *m_sock;
    ClientSocketNotify *m_notify;
    bool bRawMode;
    bool bClosed;

    Buffer m_readBuffer;
    Buffer m_writeBuffer;

    unsigned	errCode;
    QString     errString;
    friend class SocketFactory;
};

const unsigned NO_RECONNECT = (unsigned)(-1);

class EXPORT TCPClient : public QObject, public Client, public ClientSocketNotify, public EventReceiver
{
    Q_OBJECT
public:
    TCPClient(Protocol *protocol, Buffer *cfg, unsigned priority = DefaultPriority);

    virtual void	        setStatus(unsigned status, bool bCommon);

    virtual QString         getServer() const = 0;
    virtual unsigned short  getPort() const = 0;
    unsigned long	        ip() const { return m_ip; }
    virtual ClientSocket   *socket() { return m_clientSocket; }

    unsigned m_reconnect;
protected slots:
    void reconnect();
    void loginTimeout();
protected:
    virtual void	setStatus(unsigned status) = 0;
    virtual void	disconnected() = 0;

    virtual void	resolve_ready(unsigned long);
    virtual void	connect_ready();
    virtual bool	error_state(const QString &err, unsigned code = 0);
    virtual void	socketConnect();
    virtual bool    processEvent(Event *e);
    virtual Socket  *createSocket();
    virtual ClientSocket *createClientSocket();
    void			setClientStatus(unsigned status);
    unsigned		m_logonStatus;
    unsigned		m_ip;
    QTimer			*m_timer;
    QTimer			*m_loginTimer;
    bool			m_bWaitReconnect;
private:
    ClientSocket	*m_clientSocket;
};

class EXPORT InterfaceChecker : public QObject
{
	Q_OBJECT
public:
	InterfaceChecker(int polltime = 5000, bool raiseEvents = false);
	virtual ~InterfaceChecker();

	void setPollTime(int polltime);

signals:
	void interfaceUp(QString ifname);
	void interfaceDown(QString ifname);

protected:
	void timerEvent(QTimerEvent* e);

private:
	typedef struct
	{
		bool present;
		bool state;
	} tIFState;

	int m_pollTime;
	int m_timerID;
	int m_testSocket;
	std::map<std::string, tIFState> m_states;
	bool m_raiseEvents;
};

#ifdef ENABLE_OPENSSL

EXPORT QByteArray md5(const char*, int size = -1);
EXPORT QByteArray sha1(const char*, int size = -1);

class EXPORT SSLClient : public SocketNotify, public Socket
{
public:
    SSLClient(Socket*);
    ~SSLClient();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    virtual void connect(const QString &host, unsigned short port);
    virtual void close();
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    bool connected() const { return m_bSecure; }
    Socket *socket() const { return sock; }
    void setSocket(Socket *s);
    bool init();
    void accept();
    void connect();
    void shutdown();
    void process(bool bInRead=false, bool bWantRead=false);
    void write();
    void clear();
protected:
    virtual bool initSSL() = 0;
    bool initTLS1(bool bDH);
    bool initBIO();
    Buffer wBuffer;
    virtual void connect_ready();
    virtual void read_ready();
    virtual void write_ready();
    virtual void error_state(const QString &err, unsigned code = 0);
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
    SSL_CTX *mpCTX;
    SSL     *mpSSL;
    BIO     *mrBIO;
    BIO     *mwBIO;
};

#endif

}

#endif
