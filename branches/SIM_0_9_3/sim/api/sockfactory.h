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
#include "stl.h"
#include "socket.h"

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
    virtual void connect(const char *host, unsigned short port);
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    virtual void close();
protected slots:
    void slotConnected();
    void slotConnectionClosed();
    void slotReadReady();
    void slotBytesWritten(int);
    void slotBytesWritten();
    void slotError(int);
    void slotLookupFinished(int);
    void resolveReady(unsigned long addr, const char *host);
    void timeout();
protected:
    void timerStop();
    unsigned short port;
    string host;
    QSocket *sock;
    QTimer  *timer;
    bool bInWrite;
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

class SIMResolver : public QObject
{
    Q_OBJECT
public:
    SIMResolver(QObject *parent, const char *host);
    ~SIMResolver();
    QTimer *timer;
    QDns   *dns;
    bool   bDone;
    bool   bTimeout;
    unsigned long addr();
    string host();
protected slots:
    void   resolveTimeout();
    void   resolveReady();
};

class SIMSockets : public SocketFactory
{
    Q_OBJECT
public:
    SIMSockets();
    ~SIMSockets();
    virtual Socket *createSocket();
    virtual ServerSocket *createServerSocket();
    void resolve(const char *host);
signals:
    void resolveReady(unsigned long res, const char*);
public slots:
    void resultsReady();
    void idle();
    void checkState();
protected:
    list<SIMResolver*> resolvers;
};

class IP
{
public:
    IP();
    ~IP();
    void set(unsigned long ip, const char *host);
    void resolve();
    unsigned long ip() { return m_ip; }
    const char *host() { return m_host; }
protected:
    unsigned long m_ip;
    char *m_host;
};

class IPResolver : public QObject
{
    Q_OBJECT
public:
    IPResolver();
    ~IPResolver();
    list<IP*> queue;
    void start_resolve();
protected slots:
    void resolve_ready();
protected:
    unsigned long m_addr;
    QDns *resolver;
};

};

#endif


