/***************************************************************************
                          sockfactory.cpp  -  description
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

#include "simapi.h"
#include "sockfactory.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <errno.h>
#include <string>

#include <qsocket.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qdns.h>

#ifndef INADDR_NONE
#define INADDR_NONE     0xFFFFFFFF
#endif

const unsigned CONNECT_TIMEOUT = 60;

namespace SIM
{

SIMSockets::SIMSockets()
{
}

SIMSockets::~SIMSockets()
{
}

void SIMSockets::idle()
{
    SocketFactory::idle();
}

SIMResolver::SIMResolver(QObject *parent, const char *host)
        : QObject(parent)
{
    bDone = false;
    bTimeout = false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(resolveTimeout()));
    timer->start(20000);
    dns = new QDns(host, QDns::A);
    connect(dns, SIGNAL(resultsReady()), this, SLOT(resolveReady()));
}

SIMResolver::~SIMResolver()
{
    delete dns;
    delete timer;
}

void SIMResolver::resolveTimeout()
{
    bDone    = true;
    bTimeout = true;
    getSocketFactory()->setActive(false);
    QTimer::singleShot(0, parent(), SLOT(resultsReady()));
}

void SIMResolver::resolveReady()
{
    bDone = true;
    QTimer::singleShot(0, parent(), SLOT(resultsReady()));
}

unsigned long SIMResolver::addr()
{
    if (dns->addresses().isEmpty())
        return INADDR_NONE;
    return htonl(dns->addresses().first().ip4Addr());
}

string SIMResolver::host()
{
    return dns->label().latin1();
}

void SIMSockets::resolve(const char *host)
{
    SIMResolver *resolver = new SIMResolver(this, host);
    resolvers.push_back(resolver);
}

void SIMSockets::resultsReady()
{
    list<SIMResolver*>::iterator it;
    for (it = resolvers.begin(); it != resolvers.end();){
        SIMResolver *r = *it;
        if (!r->bDone){
            ++it;
            continue;
        }
        bool isActive;
        if (r->bTimeout){
            isActive = false;
        }else{
            isActive = true;
        }
        if (r->addr() == INADDR_NONE)
            isActive = false;
        setActive(isActive);
        emit resolveReady(r->addr(), r->host().c_str());
        resolvers.remove(r);
        delete r;
        it = resolvers.begin();
    }
}

Socket *SIMSockets::createSocket()
{
    return new SIMClientSocket;
}

ServerSocket *SIMSockets::createServerSocket()
{
    return new SIMServerSocket(MinTCPPort, MaxTCPPort);
}

SIMClientSocket::SIMClientSocket(QSocket *s)
{
    sock = s;
    if (sock == NULL)
        sock = new QSocket(this);
    QObject::connect(sock, SIGNAL(connected()), this, SLOT(slotConnected()));
    QObject::connect(sock, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
    QObject::connect(sock, SIGNAL(error(int)), this, SLOT(slotError(int)));
    QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slotReadReady()));
    QObject::connect(sock, SIGNAL(bytesWritten(int)), this, SLOT(slotBytesWritten(int)));
    bInWrite = false;
	timer = NULL;
}

SIMClientSocket::~SIMClientSocket()
{
    close();
    delete sock;
}

void SIMClientSocket::close()
{
	timerStop();
    sock->close();
}

void SIMClientSocket::timerStop()
{
	if (timer){
		delete timer;
		timer = NULL;
	}
}

void SIMClientSocket::slotLookupFinished(int state)
{
    log(L_DEBUG, "Lookup finished %u", state);
    if (state == 0){
        log(L_WARN, "Can't lookup");
        notify->error_state(I18N_NOOP("Connect error"));
        getSocketFactory()->setActive(false);
    }
}

int SIMClientSocket::read(char *buf, unsigned int size)
{
    unsigned available = sock->bytesAvailable();
    if (size > available)
        size = available;
    if (size == 0)
        return size;
    int res = sock->readBlock(buf, size);
    if (res < 0){
        log(L_DEBUG, "QClientSocket::read error %u", errno);
        if (notify) notify->error_state("Read socket error");
        return -1;
    }
    return res;
}

void SIMClientSocket::write(const char *buf, unsigned int size)
{
    bInWrite = true;
    int res = sock->writeBlock(buf, size);
    bInWrite = false;
    if (res != (int)size){
        if (notify) notify->error_state("Write socket error");
        return;
    }
    if (sock->bytesToWrite() == 0)
        QTimer::singleShot(0, this, SLOT(slotBytesWritten()));
}

void SIMClientSocket::connect(const char *_host, int _port)
{
    port = _port;
    host = _host;
    log(L_DEBUG, "Connect to %s:%u", host.c_str(), port);
    if (inet_addr(host.c_str()) == INADDR_NONE){
        log(L_DEBUG, "Start resolve %s", host.c_str());
        SIMSockets *s = static_cast<SIMSockets*>(getSocketFactory());
        QObject::connect(s, SIGNAL(resolveReady(unsigned long, const char*)), this, SLOT(resolveReady(unsigned long, const char*)));
        s->resolve(host.c_str());
        return;
    }
    resolveReady(inet_addr(host.c_str()), host.c_str());
}

void SIMClientSocket::resolveReady(unsigned long addr, const char *_host)
{
    if (strcmp(_host, host.c_str())) return;
    if (addr == INADDR_NONE){
        if (notify) notify->error_state(I18N_NOOP("Can't resolve host"));
        return;
    }
    in_addr a;
    a.s_addr = addr;
    host = inet_ntoa(a);
    log(L_DEBUG, "Resolve ready %s", host.c_str());
	timerStop();
	timer = new QTimer(this);
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	timer->start(CONNECT_TIMEOUT * 1000);
    sock->connectToHost(host.c_str(), port);
}

void SIMClientSocket::slotConnected()
{
    log(L_DEBUG, "Connected");
	timerStop();
    if (notify) notify->connect_ready();
    getSocketFactory()->setActive(true);
}

void SIMClientSocket::slotConnectionClosed()
{
    log(L_WARN, "Connection closed");
	timerStop();
    if (notify) notify->error_state(I18N_NOOP("Connection closed"));
}

void SIMClientSocket::timeout()
{
	QTimer::singleShot(0, this, SLOT(slotConnectionClosed()));
}

void SIMClientSocket::slotReadReady()
{
    if (notify) notify->read_ready();
}

void SIMClientSocket::slotBytesWritten(int)
{
    slotBytesWritten();
}

void SIMClientSocket::slotBytesWritten()
{
    if (bInWrite) return;
    if (sock->bytesToWrite() == 0) notify->write_ready();
}

#ifdef WIN32
#define socklen_t int
#endif

unsigned long SIMClientSocket::localHost()
{
    unsigned long res = 0;
    int s = sock->socket();
    struct sockaddr_in addr;
    memset(&addr, sizeof(addr), 0);
    socklen_t size = sizeof(addr);
    if (getsockname(s, (struct sockaddr*)&addr, &size) >= 0)
        res = addr.sin_addr.s_addr;
    if (res == 0x7F000001){
        char hostName[255];
        if (gethostname(hostName,sizeof(hostName)) >= 0) {
            struct hostent *he = NULL;
            he = gethostbyname(hostName);
            if (he != NULL)
                res = *((unsigned long*)(he->h_addr));
        }
    }
    return res;
}

void SIMClientSocket::slotError(int err)
{
    log(L_DEBUG, "Slot error %u", err);
	timerStop();
    if (notify) notify->error_state(I18N_NOOP("Socket error"));
}

void SIMClientSocket::pause(unsigned t)
{
    QTimer::singleShot(t * 1000, this, SLOT(slotBytesWritten()));
}

SIMServerSocket::SIMServerSocket(unsigned short minPort, unsigned short maxPort)
{
    sn = NULL;
    sock = new QSocketDevice;
    for (m_nPort = minPort; m_nPort <= maxPort; m_nPort++){
        if (sock->bind(QHostAddress(), m_nPort))
            break;
    }
    if ((m_nPort > maxPort) || !sock->listen(50)){
        delete sock;
        sock = NULL;
        return;
    }
    sn = new QSocketNotifier(sock->socket(), QSocketNotifier::Read, this);
    connect(sn, SIGNAL(activated(int)), this, SLOT(activated(int)));
}

SIMServerSocket::~SIMServerSocket()
{
    if (sn) delete sn;
    if (sock) delete sock;
}

void SIMServerSocket::activated(int)
{
    if (sock == NULL) return;
    int fd = sock->accept();
    if (fd >= 0){
        log(L_DEBUG, "accept ready");
        if (notify){
            QSocket *s = new QSocket;
            s->setSocket(fd);
            notify->accept(new SIMClientSocket(s), htonl(s->address().ip4Addr()));
        }else{
#ifdef WIN32
            ::closesocket(fd);
#else
            ::close(fd);
#endif
        }
    }
}

void SIMServerSocket::activated()
{
}

SocketFactory *getSocketFactory()
{
    return PluginManager::factory;
}

// ______________________________________________________________________________________

static IPResolver *pResolver = NULL;

void deleteResolver()
{
    if (pResolver)
        delete pResolver;
}

IP::IP()
{
    m_ip = 0;
    m_host = NULL;
}

IP::~IP()
{
    if (pResolver){
        for (list<IP*>::iterator it = pResolver->queue.begin(); it != pResolver->queue.end(); ++it){
            if ((*it) == this){
                pResolver->queue.erase(it);
                break;
            }
        }
    }
    if (m_host)
        delete[] m_host;
}

void IP::set(unsigned long ip, const char *host)
{
    m_ip = ip;
    if (m_host){
        delete[] m_host;
        m_host = NULL;
    }
    if (host && *host){
        m_host = new char[strlen(host) + 1];
        strcpy(m_host, host);
    }
    resolve();
}

void IP::set(unsigned long ip)
{
    m_ip = ip;
    if (m_host){
        delete[] m_host;
        m_host = NULL;
    }
    if (m_ip == 0)
        return;
    resolve();
}

void IP::resolve()
{
    if (m_host)
        return;
    if (pResolver == NULL)
        pResolver = new IPResolver;
    for (list<IP*>::iterator it = pResolver->queue.begin(); it != pResolver->queue.end(); ++it){
        if ((*it) == this)
            return;
    }
    pResolver->queue.push_back(this);
    pResolver->start_resolve();
}

IPResolver::IPResolver()
{
    resolver = new QDns;
    resolver->setRecordType(QDns::Ptr);
    QObject::connect(resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
}

IPResolver::~IPResolver()
{
    delete resolver;
}

void IPResolver::resolve_ready()
{
    if (queue.empty()) return;
    string m_host;
    if (resolver->hostNames().count())
        m_host = resolver->hostNames().first().latin1();
    struct in_addr inaddr;
    inaddr.s_addr = m_addr;
    log(L_DEBUG, "Resolver ready %s %s", inet_ntoa(inaddr), m_host.c_str());
    for (list<IP*>::iterator it = queue.begin(); it != queue.end(); ){
        if ((*it)->ip() != m_addr){
            ++it;
            continue;
        }
        (*it)->set((*it)->ip(), m_host.c_str());
        queue.erase(it);
        it = queue.begin();
    }
    start_resolve();
}

void IPResolver::start_resolve()
{
    if (resolver->isWorking()) return;
    if (queue.empty())
        return;
    IP *ip = *queue.begin();
    m_addr = ip->ip();
    struct in_addr inaddr;
    inaddr.s_addr = m_addr;
    log(L_DEBUG, "start resolve %s", inet_ntoa(inaddr));
#if QT_VERSION >= 300
    delete resolver;
    resolver = new QDns(QHostAddress(htonl(m_addr)), QDns::Ptr);
    connect(resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
#else
    resolver->setLabel(QHostAddress(htonl(m_addr)));
#endif
}

};

#ifndef WIN32
#include "sockfactory.moc"
#endif
