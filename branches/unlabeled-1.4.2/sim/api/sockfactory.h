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
    virtual void connect(const char *host, int port);
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
protected:
    unsigned short port;
    string host;
    QSocket *sock;
    bool bInWrite;
};

class SIMServerSocket : public QObject, public ServerSocket
{
    Q_OBJECT
public:
    SIMServerSocket(unsigned short minPort, unsigned short maxPort);
    ~SIMServerSocket();
    virtual unsigned short port() { return m_nPort; }
    bool created() { return (sock != NULL); }
protected slots:
    void activated(int);
    void activated();
protected:
    QSocketDevice   *sock;
    QSocketNotifier *sn;
    unsigned short m_nPort;
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
protected:
    list<SIMResolver*> resolvers;
};

class IP
{
public:
    IP();
    ~IP();
    void set(unsigned long ip);
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


