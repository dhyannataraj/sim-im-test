/***************************************************************************
                          sockfactory.h  -  description
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

#ifndef SOCKFACTORY_H
#define SOCKFACTORY_H	1

#include "simapi.h"
#include "socket.h"
#include <qthread.h>

class QDns;
class QTimer;
class QSocket;
class QSocketDevice;
class QSocketNotifier;

namespace SIM
{

class SIMClientSocket : public QObject, public Socket
{
    Q_OBJECT
public:
    SIMClientSocket(QSocket *s=NULL);
    virtual ~SIMClientSocket();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    virtual void connect(const QString &host, unsigned short port);
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    virtual void close();
	virtual int getFd();

signals:
	void interfaceDown(int sockfd); // Probably, sockfd is not needed
	void interfaceUp(int sockfd);

protected slots:
    void slotConnected();
    void slotConnectionClosed();
    void slotReadReady();
    void slotBytesWritten(int);
    void slotBytesWritten();
    void slotError(int);
    void slotLookupFinished(int);
    void resolveReady(unsigned long addr, const QString &host);
    void timeout();
	void error(int errcode);
	void checkInterface();
	void timerEvent(QTimerEvent* ev);
protected:
	int m_carrierCheckTimer;
	bool m_state;
    void timerStop();
    unsigned short port;
    QString host;
    QSocket *sock;
    QTimer  *timer;
    bool bInWrite;
	QString m_interface;
};

class SIMServerSocket : public QObject, public ServerSocket
{
    Q_OBJECT
public:
    SIMServerSocket();
    ~SIMServerSocket();
    virtual unsigned short port() { return m_nPort; }
    bool created() { return (sock != NULL); }
    void bind(unsigned short minPort, unsigned short maxPort, TCPClient *client);
#ifndef WIN32
    void bind(const char *path);
#endif
    void close();
protected slots:
    void activated(int);
    void activated();
protected:
    void listen(TCPClient*);
    void error(const char *err);
    QSocketDevice   *sock;
    QSocketNotifier *sn;
    QString			m_name;
    unsigned short  m_nPort;
};

class IResolver
{
public:
	virtual ~IResolver() {};
	virtual unsigned long addr() = 0;
	virtual QString host() const = 0;
	virtual bool isDone() = 0;
	virtual bool isTimeout() = 0;

};

class SIMResolver : public QObject, public IResolver
{
    Q_OBJECT
public:
    SIMResolver(QObject *parent, const QString &host);
    virtual ~SIMResolver();
    virtual unsigned long addr();
    virtual QString host() const;
	virtual bool isDone();
	virtual bool isTimeout();

protected slots:
    void   resolveTimeout();
    void   resolveReady();

private:
    QTimer *timer;
    QDns   *dns;
    bool   bDone;
    bool   bTimeout;
};

// I'm not sure if this resolver will compile well in win32, hence this ifndef


class StdResolver : public QObject, public QThread, public IResolver
{
	Q_OBJECT
public:
	StdResolver(QObject* parent, const QString& host);
	virtual ~StdResolver();
	virtual unsigned long addr();
	virtual QString host() const;
	virtual bool isDone();
	virtual bool isTimeout();

protected:
	virtual void run();

protected slots:
	void timeout();

private:
    bool m_done;
    bool m_timeout;
	unsigned long m_addr;
	QString m_host;
	QTimer* m_timer;
};



class SIMSockets : public SocketFactory
{
    Q_OBJECT
public:
    SIMSockets(QObject *parent);
    virtual ~SIMSockets();
    virtual Socket *createSocket();
    virtual ServerSocket *createServerSocket();
    void resolve(const QString &host);
signals:
    void resolveReady(unsigned long res, const QString &);
public slots:
    void resultsReady();
    void idle();
    void checkState();
protected:
    std::list<IResolver*> resolvers;

};

class IP
{
public:
    IP();
    ~IP();
    void set(unsigned long ip, const QString &host);
    void resolve();
    unsigned long ip() const { return m_ip; }
    const QString &host() const { return m_host; }
protected:
    unsigned long m_ip;
    QString m_host;
};

class IPResolver : public QObject
{
    Q_OBJECT
public:
    IPResolver();
    ~IPResolver();
    std::list<IP*> queue;
    void start_resolve();
protected slots:
    void resolve_ready();
protected:
    unsigned long m_addr;
    QDns *resolver;
};

}

#endif
