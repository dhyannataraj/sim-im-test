/***************************************************************************
                          sockets.h  -  description
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

#ifndef _SOCKETS_H
#define _SOCKETS_H

#include "defs.h"
#include "cfg.h"

#include "icqclient.h"

#undef HAVE_KEXTSOCK_H

class QDns;
class QSocket;
class QSocketDevice;
class QSocketNotifier;

#ifdef HAVE_KEXTSOCK_H
class KExtendedSocket;
#endif

class ICQClientSocket : public QObject, public Socket
{
    Q_OBJECT
public:
#ifdef HAVE_KEXTSOCK_H
    ICQClientSocket(KExtendedSocket *s=NULL);
#else
    ICQClientSocket(QSocket *s=NULL);
#endif
    virtual ~ICQClientSocket();
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
    void resolveReady();
    void resolveTimeout();
protected:
#ifdef HAVE_KEXTSOCK_H
    KExtendedSocket *sock;
#else
    unsigned short port;
    QDns	*resolver;
    QSocket *sock;
#endif
    bool bInWrite;
};

class ICQServerSocket : public QObject, public ServerSocket
{
    Q_OBJECT
public:
    ICQServerSocket(unsigned short minPort, unsigned short maxPort);
    ~ICQServerSocket();
    virtual unsigned short port() { return m_nPort; }
    bool created() { return (sock != NULL); }
protected slots:
    void activated(int);
    void activated();
protected:
#ifdef HAVE_KEXTSOCK_H
    KExtendedSocket *sock;
#else
    QSocketDevice   *sock;
    QSocketNotifier *sn;
#endif
    unsigned short m_nPort;
};

class SIMSockets : public SocketFactory
{
public:
    SIMSockets();

    virtual Socket *createSocket();
    virtual ServerSocket *createServerSocket();
};

SocketFactory *getFactory();

#endif

