/***************************************************************************
                          msnhttp.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef MSNHTTP_H
#define MSNHTTP_H	1

#include "simapi.h"
#include "socket.h"
#include "fetch.h"

class MSNClient;

class MSNHttpPool : public QObject, public Socket, public FetchClient
{
    Q_OBJECT
public:
    MSNHttpPool(MSNClient *client, bool bSB);
    ~MSNHttpPool();
    virtual void connect(const char *host, unsigned short port);
    virtual int  read(char *buf, unsigned size);
    virtual void write(const char *buf, unsigned size);
    virtual void close();
    virtual Mode mode() { return Web; }
protected slots:
    void idle();
protected:
    string m_session_id;
    string m_host;
    string m_ip;
    Buffer readData;
    Buffer *writeData;
	virtual bool done(unsigned code, Buffer &data, const char *headers);
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    bool m_bSB;
    MSNClient *m_client;
};

#endif
