/***************************************************************************
                          yahoohttp.cpp  -  description
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

#include "yahooclient.h"
#include "fetch.h"

class YahooHttpPool : public Socket, public EventReceiver
{
public:
    YahooHttpPool();
    ~YahooHttpPool();
    virtual void connect(const char *host, unsigned short port);
    virtual int  read(char *buf, unsigned size);
    virtual void write(const char *buf, unsigned size);
    virtual void close();
    virtual Mode mode() { return Web; }
protected:
    void *processEvent(Event *e);
    Buffer readData;
    Buffer *writeData;
    unsigned m_fetch_id;
    virtual unsigned long localHost();
    virtual void pause(unsigned);
};

// ______________________________________________________________________________________

YahooHttpPool::YahooHttpPool()
{
    m_fetch_id = 0;
    writeData = new Buffer;
}

YahooHttpPool::~YahooHttpPool()
{
    delete writeData;
}

int YahooHttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == readData.size())
        readData.init(0);
    return size;
}

static char YAHOO_HTTP[] = "http://shttp.msg.yahoo.com/notify/";

void YahooHttpPool::write(const char *buf, unsigned size)
{
    writeData->pack(buf, size);
    if (m_fetch_id)
        return;
    char headers[] = "Accept: application/octet-stream\x00\x00";
    m_fetch_id = fetch(YAHOO_HTTP, writeData, headers);
    writeData = new Buffer;
}

void YahooHttpPool::close()
{
    delete writeData;
    writeData = new Buffer;
    m_fetch_id = 0;
}

void YahooHttpPool::connect(const char*, unsigned short)
{
    if (notify)
        notify->connect_ready();
}

void *YahooHttpPool::processEvent(Event *e)
{
    if (e->type() == EventFetchDone){
        fetchData *d = (fetchData*)(e->param());
        if (d->req_id != m_fetch_id)
            return NULL;
        m_fetch_id = 0;
        if (d->result != 200){
            log(L_DEBUG, "HTTP result %u", d->result);
            error("Bad result");
            return e->param();
        }
        unsigned long packet_id;
        *d->data >> packet_id;
        log(L_DEBUG, "Packet ID: %u %X");
        readData.pack(d->data->data(d->data->readPos()), d->data->writePos() - d->data->readPos());
        if (notify)
            notify->read_ready();
        return e->param();
    }
    return NULL;
}

unsigned long YahooHttpPool::localHost()
{
    return 0;
}

void YahooHttpPool::pause(unsigned)
{
}

Socket *YahooClient::createSocket()
{
    m_bHTTP = getUseHTTP();
    if (getAutoHTTP()){
        m_bHTTP = m_bFirstTry;
        if (!m_bFirstTry)
            m_bFirstTry = true;
    }
    if (m_bHTTP)
        return new YahooHttpPool;
    return NULL;
}


