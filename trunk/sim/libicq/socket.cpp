/***************************************************************************
                          socket.cpp  -  description
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

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "socket.h"
#include "log.h"

Socket::Socket(SocketNotify *n)
{
    notify = n;
}

ServerSocket::ServerSocket(ServerSocketNotify *n)
{
    notify = n;
}

ClientSocket::ClientSocket(ClientSocketNotify *n, SocketFactory *f, int fd)
{
    notify = n;
    factory = f;
    bRawMode = false;
    sock = f->createSocket(this, fd);
}

ClientSocket::~ClientSocket()
{
    if (sock) delete sock;
}

void ClientSocket::error()
{
    log(L_WARN, "Protocol error");
}

void ClientSocket::connect(const char *host, int port)
{
    sock->connect(host, port);
}

void ClientSocket::write()
{
    if (writeBuffer.size() == 0) return;
    sock->write(writeBuffer.Data(0), writeBuffer.size());
    writeBuffer.init(0);
}

bool ClientSocket::created()
{
    return (sock != NULL);
}

void ClientSocket::connect_ready()
{
    notify->connect_ready();
}

void ClientSocket::setRaw(bool mode)
{
    bRawMode = mode;
    read_ready();
    if (mode) readBuffer.init(0);
}

void ClientSocket::read_ready()
{
    if (bRawMode){
        for (;;){
            char b[2048];
            int readn = sock->read(b, sizeof(b));
            log(L_DEBUG, "Read ready %X %u", this, readn);
            if (readn == 0) break;
            readBuffer.setWritePos(readBuffer.writePos() + readn);
            if (readn < sizeof(b)) break;
        }
        notify->packet_ready();
        return;
    }
    for (;;){
        int readn = sock->read(readBuffer.Data(readBuffer.writePos()),
                               readBuffer.size() - readBuffer.writePos());
        log(L_DEBUG, "Read ready %X %u", this, readn);
        if (readn == 0) break;
        readBuffer.setWritePos(readBuffer.writePos() + readn);
        if (readBuffer.writePos() < readBuffer.size()) break;
        notify->packet_ready();
    }
}

void ClientSocket::write_ready()
{
    notify->write_ready();
}

void ClientSocket::error_state()
{
    notify->error_state();
}

void ClientSocket::remove()
{
    sock->close();
    factory->removedNotifies.push_back(this);
}

unsigned long ClientSocket::localHost()
{
    return sock->localHost();
}

void ClientSocket::pause(unsigned n)
{
    sock->pause(n);
}

void SocketFactory::idle()
{
    for (list<SocketNotify*>::iterator itNot = removedNotifies.begin(); itNot != removedNotifies.end(); ++itNot)
        delete (*itNot);
    removedNotifies.clear();
}

