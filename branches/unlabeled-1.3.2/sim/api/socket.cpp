/***************************************************************************
                          socket.cpp  -  description
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

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "socket.h"

#include <qtimer.h>

namespace SIM
{

#ifndef INADDR_NONE
#define INADDR_NONE	0xFFFFFFFF
#endif

const unsigned RECONNECT_TIME		= 5;
const unsigned RECONNECT_IFINACTIVE = 60;

class SocketFactoryPrivate
{
public:
    SocketFactoryPrivate() {}
    list<ClientSocket*> errSockets;
    list<Socket*> removedSockets;
};

Socket::Socket()
{
    notify = NULL;
}

ServerSocket::ServerSocket()
{
    notify = NULL;
}

ClientSocket::ClientSocket(ClientSocketNotify *notify)
{
    m_notify = notify;
    bRawMode = false;
    bClosed  = false;
    m_sock = getSocketFactory()->createSocket();
    m_sock->setNotify(this);
}

ClientSocket::~ClientSocket()
{
    if (m_sock)
        delete m_sock;
    getSocketFactory()->p->errSockets.remove(this);
}

void ClientSocket::close()
{
    m_sock->close();
    bClosed = true;
}

const char *ClientSocket::errorString()
{
    if (errString.length())
        return errString.c_str();
    return NULL;
}

void ClientSocket::connect(const char *host, int port, const char *proto)
{
    if (proto){
        ConnectParam p;
        p.socket = this;
        p.host = host;
        p.port = port;
        p.proto = proto;
        Event e(EventSocketConnect, &p);
        e.process();
    }
    m_sock->connect(host, port);
}

void ClientSocket::write()
{
    if (writeBuffer.size() == 0) return;
    m_sock->write(writeBuffer.data(), writeBuffer.size());
    writeBuffer.init(0);
}

bool ClientSocket::created()
{
    return (m_sock != NULL);
}

void ClientSocket::connect_ready()
{
    m_notify->connect_ready();
    bClosed = false;
}

void ClientSocket::setRaw(bool mode)
{
    bRawMode = mode;
    read_ready();
}

void ClientSocket::read_ready()
{
    if (bRawMode){
        for (;;){
            char b[2048];
            int readn = m_sock->read(b, sizeof(b));
            if (readn == 0) break;
            unsigned pos = readBuffer.writePos();
            readBuffer.setWritePos(readBuffer.writePos() + readn);
            memcpy(readBuffer.data(pos), b, readn);
        }
        if (m_notify)
            m_notify->packet_ready();
        return;
    }
    for (;;){
        if (bClosed || errString.length()) break;
        int readn = m_sock->read(readBuffer.data(readBuffer.writePos()),
                                 readBuffer.size() - readBuffer.writePos());
        if (readn < 0){
            error_state(I18N_NOOP("Read socket error"));
            return;
        }
        if (readn == 0) break;
        readBuffer.setWritePos(readBuffer.writePos() + readn);
        if (readBuffer.writePos() < readBuffer.size()) break;
        if (m_notify)
            m_notify->packet_ready();
    }
}

void ClientSocket::write_ready()
{
    if (m_notify)
        m_notify->write_ready();
}

unsigned long ClientSocket::localHost()
{
    return m_sock->localHost();
}

void ClientSocket::pause(unsigned n)
{
    m_sock->pause(n);
}

void ClientSocket::setSocket(Socket *s)
{
    m_sock = s;
    s->setNotify(this);
}

void ClientSocket::setNotify(ClientSocketNotify *notify)
{
    m_notify = notify;
}

void ClientSocket::error_state(const char *err, unsigned code)
{
    list<ClientSocket*>::iterator it;
    for (it = getSocketFactory()->p->errSockets.begin(); it != getSocketFactory()->p->errSockets.end(); ++it)
        if ((*it) == this) return;
    errString = "";
    errCode = code;
    if (err)
        errString = err;
    getSocketFactory()->p->errSockets.push_back(this);
    QTimer::singleShot(0, getSocketFactory(), SLOT(idle()));
}

SocketFactory::SocketFactory()
{
    MinTCPPort = 1024;
    MaxTCPPort = 0xFFFF;
    m_bActive  = true;
    p = new SocketFactoryPrivate;
}

SocketFactory::~SocketFactory()
{
    delete p;
}

bool SocketFactory::isActive()
{
    return m_bActive;
}

void SocketFactory::setActive(bool isActive)
{
    if (isActive == m_bActive)
        return;
    m_bActive = isActive;
    Event e(EventSocketActive, (void*)m_bActive);
    e.process();
}

void SocketFactory::remove(Socket *s)
{
    s->setNotify(NULL);
    s->close();
    for (list<Socket*>::iterator it = p->removedSockets.begin(); it != p->removedSockets.end(); ++it)
        if ((*it) == s) return;
    p->removedSockets.push_back(s);
    QTimer::singleShot(0, this, SLOT(idle()));
}

void SocketFactory::idle()
{
    for (list<ClientSocket*>::iterator it = p->errSockets.begin(); it != p->errSockets.end();){
        ClientSocket *s = *it;
        ClientSocketNotify *n = s->m_notify;
        p->errSockets.remove(s);
        it = p->errSockets.begin();
        if (n){
            string errString;
            if (s->errorString())
                errString = s->errorString();
            s->errString = "";
            if (n->error_state(errString.c_str(), s->errCode))
                delete n;
        }
    }
    for (list<Socket*>::iterator its = p->removedSockets.begin(); its != p->removedSockets.end(); ++its)
        delete *its;
    p->removedSockets.clear();
}

TCPClient::TCPClient(Protocol *protocol, const char *cfg)
        : Client(protocol, cfg)
{
    m_socket = NULL;
    m_timer  = new QTimer(this);
    m_reconnectTime = RECONNECT_TIME;
    m_bWaitReconnect = false;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(reconnect()));
}

bool TCPClient::error_state(const char *err, unsigned code)
{
    log(L_DEBUG, "Socket error %s (%u)", err, code);
    if (m_reconnectTime == NO_RECONNECT){
        m_timer->stop();
        setStatus(STATUS_OFFLINE, getCommonStatus());
        setState((code == LOGIN_ERROR) ? AuthError : Error, err);
        return false;
    }
    if (!m_timer->isActive()){
        unsigned reconnectTime = m_reconnectTime;
        if (!getSocketFactory()->isActive()){
            if (reconnectTime < RECONNECT_IFINACTIVE)
                reconnectTime = RECONNECT_IFINACTIVE;
        }
        setClientStatus(STATUS_OFFLINE);
        setState(Connecting, err);
        m_bWaitReconnect = true;
        log(L_DEBUG, "Wait reconnect %u sec", reconnectTime);
        m_timer->start(reconnectTime * 1000);
    }
    return false;
}

void TCPClient::reconnect()
{
    m_timer->stop();
    if (m_bWaitReconnect)
        setClientStatus(getManualStatus());
}

void TCPClient::setStatus(unsigned status, bool bCommon)
{
    log(L_DEBUG, "Set status %u %u", status, bCommon);
    setClientStatus(status);
    Client::setStatus(status, bCommon);
}

void TCPClient::connect_ready()
{
    m_timer->stop();
    m_bWaitReconnect = false;
}

void TCPClient::setClientStatus(unsigned status)
{
    if (status != STATUS_OFFLINE){
        if (getState() == Connected){
            setStatus(status);
            return;
        }
        m_logonStatus = status;
        if ((getState() != Connecting) || m_bWaitReconnect){
            if (m_socket)
                m_socket->close();
            if (m_socket == NULL)
                m_socket = new ClientSocket(this);
            log(L_DEBUG, "Start connect %s:%u", getServer(), getPort());
            m_socket->connect(getServer(), getPort(), protocol()->description()->text);
            m_reconnectTime = RECONNECT_TIME;
            m_bWaitReconnect = false;
            setState(Connecting);
        }
        return;
    }
    m_bWaitReconnect = false;
    m_timer->stop();
    if (m_socket)
        setStatus(STATUS_OFFLINE);
    m_status = STATUS_OFFLINE;
    setState(Offline);
    disconnected();
    if (m_socket){
        m_socket->close();
        delete m_socket;
        m_socket = NULL;
    }
}

};

#ifndef WIN32
#include "socket.moc"
#endif


