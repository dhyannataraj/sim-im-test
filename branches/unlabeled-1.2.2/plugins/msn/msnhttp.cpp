/***************************************************************************
                          msnhttp.cpp  -  description
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

#include "msnhttp.h"
#include "msnclient.h"
#include "fetch.h"

#include <qtimer.h>

const unsigned POLL_TIMEOUT	= 10;

// ______________________________________________________________________________________

MSNHttpPool::MSNHttpPool(MSNClient *client, bool bSB)
{
    m_fetch_id = 0;
    m_client   = client;
    m_bSB = bSB;
    writeData = new Buffer;
}

MSNHttpPool::~MSNHttpPool()
{
    delete writeData;
}

int MSNHttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == readData.size())
        readData.init(0);
    return size;
}

static char MSN_HTTP[] = "/gateway/gateway.dll?";

void MSNHttpPool::write(const char *buf, unsigned size)
{
    writeData->pack(buf, size);
    if (m_fetch_id)
        return;
    string url = "http://";
    if (m_session_id.empty()){
        url += "gateway.messenger.hotmail.com";
        url += MSN_HTTP;
        url += "Action=open&Server=";
        url += m_bSB ? "SB" : "NS";
        url += "&IP=";
        url += m_ip;
    }else{
        url += m_host;
        url += MSN_HTTP;
        if (writeData->writePos() == 0)
            url += "Action=poll&";
        url += "SessionID=" + m_session_id;
    }
    const char *headers =
        "Content-Type: application/x-msn-messenger\x00"
        "Proxy-Connection: Keep-Alive\x00";
    m_fetch_id = fetch(url.c_str(), writeData, headers);
    writeData = new Buffer;
}

void MSNHttpPool::close()
{
    delete writeData;
    writeData = new Buffer;
    m_session_id = "";
    m_host = "";
    m_fetch_id = 0;
}

void MSNHttpPool::connect(const char *host, unsigned short)
{
    m_ip = host;
    if (notify)
        notify->connect_ready();
}

void MSNHttpPool::idle()
{
    if ((m_fetch_id == 0) && (m_client->m_fetchId == 0)){
        log(L_DEBUG, "send idle");
        write("", 0);
    }
}

void *MSNHttpPool::processEvent(Event *e)
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
        for (const char *p = d->headers; *p; p += strlen(p) + 1){
            string h = p;
            if (getToken(h, ':') == "X-MSN-Messenger"){
				const char *p1;
				for (p1 = h.c_str(); *p1; p1++){
                    if (*p1 != ' ')
                        break;
                }
                string h = p1;
                while (!h.empty()){
                    string part = getToken(h, ';');
                    const char *p2;
                    for (p2 = part.c_str(); *p2; p2++){
                        if (*p2 != ' ')
                            break;
                    }
                    string v = p2;
                    string k = getToken(v, '=');
                    if (k == "SessionID"){
                        m_session_id = v;
                    }else if (k == "GW-IP"){
                        m_host = v;
                    }
                }
                break;
            }
        }
        if (m_session_id.empty() || m_host.empty()){
            error("No session in answer");
            return e->param();
        }
        readData.pack(d->data->data(), d->data->writePos());
        if (notify)
            notify->read_ready();
        QTimer::singleShot(POLL_TIMEOUT * 1000, this, SLOT(idle()));
        return e->param();
    }
    return NULL;
}

unsigned long MSNHttpPool::localHost()
{
    return 0;
}

void MSNHttpPool::pause(unsigned)
{
}

Socket *MSNClient::createSocket()
{
    m_bHTTP = getUseHTTP();
    if (getAutoHTTP()){
        m_bHTTP = m_bFirstTry;
        if (!m_bFirstTry)
            m_bFirstTry = true;
    }
    if (m_bHTTP)
        return new MSNHttpPool(this, false);
    return NULL;
}

Socket *MSNClient::createSBSocket()
{
    if (m_bHTTP)
        return new MSNHttpPool(this, true);
    return NULL;
}

#ifndef WIN32
#include "msnhttp.moc"
#endif


