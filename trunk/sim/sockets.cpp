/***************************************************************************
                          sockets.cpp  -  description
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

#include "log.h"
#include "sockets.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <errno.h>
#include <string>

#include <qsocket.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qdns.h>

#ifdef HAVE_KEXTSOCK_H
#include <kextsock.h>
#include <ksockaddr.h>
#endif

SIMSockets::SIMSockets()
{
}

Socket *SIMSockets::createSocket()
{
    return new ICQClientSocket;
}

ServerSocket *SIMSockets::createServerSocket()
{
    return new ICQServerSocket(MinTCPPort, MaxTCPPort);
}

#ifdef HAVE_KEXTSOCK_H
ICQClientSocket::ICQClientSocket(KExtendedSocket *s)
#else
ICQClientSocket::ICQClientSocket(QSocket *s)
#endif
{
    sock = s;
    if (sock == NULL)
#ifdef HAVE_KEXTSOCK_H
        sock = new KExtendedSocket;
    sock->setSocketFlags(KExtendedSocket::outputBufferedSocket );
#else
        sock = new QSocket(this);
    bConnected = false;
#endif
#ifdef HAVE_KEXTSOCK_H
    QObject::connect(sock, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()));
    QObject::connect(sock, SIGNAL(lookupFinished(int)), this, SLOT(slotLookupFinished(int)));
    QObject::connect(sock, SIGNAL(connectionFailed(int)), this, SLOT(slotError(int)));
    QObject::connect(sock, SIGNAL(closed(int)), this, SLOT(slotError(int)));
#else
    QObject::connect(sock, SIGNAL(connected()), this, SLOT(slotConnected()));
    QObject::connect(sock, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
    QObject::connect(sock, SIGNAL(error(int)), this, SLOT(slotError(int)));
#endif
    QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slotReadReady()));
    QObject::connect(sock, SIGNAL(bytesWritten(int)), this, SLOT(slotBytesWritten(int)));
    bInWrite = false;
#ifdef HAVE_KEXTSOCK_H
    if (s) sock->enableRead(true);
#endif
}

ICQClientSocket::~ICQClientSocket()
{
    close();
    delete sock;
}

void ICQClientSocket::close()
{
#ifdef HAVE_KEXTSOCK_H
    sock->closeNow();
#else
    sock->close();
#endif
}

void ICQClientSocket::slotLookupFinished(int state)
{
    log(L_DEBUG, "Lookup finished %u", state);
}

int ICQClientSocket::read(char *buf, unsigned int size)
{
    int res = sock->readBlock(buf, size);
    if (res < 0){
#ifdef HAVE_KEXTSOCK_H
        if ((errno == EWOULDBLOCK) || (errno == 0))
            return 0;
#endif
        log(L_DEBUG, "QClientSocket::read error %u", errno);
        if (notify) notify->error_state(ErrorRead);
        return -1;
    }
    return res;
}

void ICQClientSocket::write(const char *buf, unsigned int size)
{
    bInWrite = true;
    int res = sock->writeBlock(buf, size);
    bInWrite = false;
    if (res != (int)size){
        if (notify) notify->error_state(ErrorWrite);
        return;
    }
    if (sock->bytesToWrite() == 0)
        QTimer::singleShot(0, this, SLOT(slotBytesWritten()));
}

void ICQClientSocket::connect(const char *host, int _port)
{
    port = _port;
    log(L_DEBUG, "Connect to %s:%u", host, port);
#ifdef HAVE_KEXTSOCK_H
    sock->setAddress(host, port);
    if (sock->lookup() < 0){
        log(L_WARN, "Can't lookup");
        if (notify) notify->error_state(ErrorConnect);
    }
    if (sock->startAsyncConnect() < 0){
        log(L_WARN, "Can't connect");
        if (notify) notify->error_state(ErrorConnect);
    }
#else
    bConnected = false;
    QTimer::singleShot(10000, this, SLOT(resolveTimeout()));
    sock->connectToHost(host, port);
#endif
}

void ICQClientSocket::resolveTimeout()
{
    if (!bConnected)
        slotError(1);
}

void ICQClientSocket::slotConnected()
{
    log(L_DEBUG, "Connected");
    if (notify) notify->connect_ready();
#ifdef HAVE_KEXTSOCK_H
    sock->setBlockingMode(false);
    sock->enableRead(true);
#else
    bConnected = true;
#endif
}

void ICQClientSocket::slotConnectionClosed()
{
    log(L_WARN, "Connection closed");
    if (notify) notify->error_state(ErrorConnectionClosed);
}

void ICQClientSocket::slotReadReady()
{
    if (notify) notify->read_ready();
}

void ICQClientSocket::slotBytesWritten(int)
{
    slotBytesWritten();
}

void ICQClientSocket::slotBytesWritten()
{
    if (bInWrite) return;
    if (sock->bytesToWrite() == 0) notify->write_ready();
}

#ifdef WIN32
#define socklen_t int
#endif

unsigned long ICQClientSocket::localHost()
{
#ifdef HAVE_KEXTSOCK_H
    unsigned long res = 0;
    const KSocketAddress *addr = sock->localAddress();
    if (addr && addr->inherits("KInetSocketAddress")){
        const KInetSocketAddress *addr_in = static_cast<const KInetSocketAddress*>(addr);
        const sockaddr_in *a = addr_in->addressV4();
        if (a) res = htonl(a->sin_addr.s_addr);
    }
    return res;
#else
    unsigned long res = 0;
    int s = sock->socket();
    struct sockaddr_in addr;
    memset(&addr, sizeof(addr), 0);
    socklen_t size = sizeof(addr);
    if (getsockname(s, (struct sockaddr*)&addr, &size) >= 0)
        res = htonl(addr.sin_addr.s_addr);
    if (res == 0x7F000001){
        char hostName[255];
        if (gethostname(hostName,sizeof(hostName)) >= 0) {
            struct hostent *he = NULL;
            he = gethostbyname(hostName);
            if (he != NULL)
                res = htonl(*((unsigned long*)(he->h_addr)));
        }
    }
    return res;
#endif
}

void ICQClientSocket::slotError(int err)
{
#ifdef HAVE_KEXTSOCK_H
    if (!(err & KBufferedIO::involuntary)) return;
    log(L_DEBUG, "Connection closed by peer");
#else
    if (!err) return;
    log(L_DEBUG, "Slot error %u", err);
#endif
    if (notify) notify->error_state(ErrorSocket);
}

void ICQClientSocket::pause(unsigned t)
{
    QTimer::singleShot(t * 1000, this, SLOT(slotBytesWritten()));
}

ICQServerSocket::ICQServerSocket(unsigned short minPort, unsigned short maxPort)
{
#ifdef HAVE_KEXTSOCK_H
    sock = new KExtendedSocket;
    connect(sock, SIGNAL(readyAccept()), this, SLOT(activated()));
    for (m_nPort = minPort; m_nPort <= maxPort; m_nPort++){
        sock->reset();
        sock->setBlockingMode(false);
        sock->setSocketFlags(KExtendedSocket::passiveSocket);
        sock->setPort(m_nPort);
        if (sock->listen() == 0)
            break;
    }
    if (m_nPort > maxPort){
        delete sock;
        sock = NULL;
        return;
    }
#else
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
#endif
}

ICQServerSocket::~ICQServerSocket()
{
    if (sn) delete sn;
    if (sock) delete sock;
}

void ICQServerSocket::activated(int)
{
#ifndef HAVE_KEXTSOCK_H
    if (sock == NULL) return;
    int fd = sock->accept();
    if (fd >= 0){
        log(L_DEBUG, "accept ready");
        if (notify){
            QSocket *s = new QSocket;
            s->setSocket(fd);
            notify->accept(new ICQClientSocket(s));
        }else{
#ifdef WIN32
            ::closesocket(fd);
#else
            ::close(fd);
#endif
        }
    }
#endif
}

void ICQServerSocket::activated()
{
#ifdef HAVE_KEXTSOCK_H
    log(L_DEBUG, "accept ready");
    KExtendedSocket *s = NULL;
    sock->accept(s);
    log(L_DEBUG, "Accept: %u", s);
    if (s == NULL) return;
    if (notify){
        notify->accept(new ICQClientSocket(s));
    }else{
        delete s;
    }
#endif
}

static SocketFactory *pFactory = NULL;

SocketFactory *getFactory()
{
    if (pFactory == NULL) pFactory = new SIMSockets;
    return pFactory;
}

#ifndef _WINDOWS
#include "sockets.moc"
#endif
