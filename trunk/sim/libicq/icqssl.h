/***************************************************************************
                          icqssl.h  -  description
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

#ifndef _ICQSSL_H
#define _ICQSSL_H

#ifdef USE_OPENSSL
#include "socket.h"

class SSLClient : public SocketNotify, public Socket
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
    bool init();
    bool initHTTPS();
    void accept();
    void connect();
    void shutdown();
    void process(bool bInRead=false);
    void write();
protected:
    void initSSL();
    void initSSL_HTTPS();
    bool initBIO();
    Buffer wBuffer;
    virtual void connect_ready();
    virtual void read_ready();
    virtual void write_ready();
    virtual void error_state(SocketError);
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
    void *mpSSL;
    void *mrBIO;
    void *mwBIO;
#define pSSL	((SSL*)mpSSL)
#define rBIO	((BIO*)mrBIO)
#define wBIO	((BIO*)mwBIO)
};

#endif
#endif

