/***************************************************************************
                          polling.cpp  -  description
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

#include "simapi.h"
#include "icqclient.h"
#include "fetch.h"

const unsigned short HTTP_PROXY_VERSION = 0x0443;

const unsigned short HTTP_PROXY_HELLO	   = 2;
const unsigned short HTTP_PROXY_LOGIN      = 3;
const unsigned short HTTP_PROXY_UNK1	   = 4;
const unsigned short HTTP_PROXY_FLAP       = 5;
const unsigned short HTTP_PROXY_CONNECT	   = 6;
const unsigned short HTTP_PROXY_UNK2       = 7;

class HttpPacket
{
    COPY_RESTRICTED(HttpPacket)
public:
    HttpPacket(const char *data, unsigned short size, unsigned short type, unsigned long nSock);
    ~HttpPacket();
    char *data;
    unsigned short size;
    unsigned short type;
    unsigned long  nSock;
};

HttpPacket::HttpPacket(const char *_data, unsigned short _size, unsigned short _type, unsigned long _nSock)
{
    size = _size;
    type = _type;
    nSock = _nSock;
    data = NULL;
    if (size){
        data = new char[size];
        memcpy(data, _data, size);
    }
}

HttpPacket::~HttpPacket()
{
    if (data) delete[] data;
}

// ______________________________________________________________________________________

class HttpRequest;

class HttpPool : public Socket
{
public:
    HttpPool();
    ~HttpPool();
    virtual void connect(const char *host, unsigned short port);
    virtual int  read(char *buf, unsigned size);
    virtual void write(const char *buf, unsigned size);
    virtual void close();
    virtual Mode mode() { return Web; }
protected:
    enum State
    {
        None,
        Connected
    };
    State state;

    string sid;
    string m_host;

    list<HttpPacket*> queue;
    unsigned seq;
    unsigned readn;
    Buffer readData;

    HttpRequest *hello;
    HttpRequest *monitor;
    HttpRequest *post;

    unsigned short nSock;
    void   request();
    virtual unsigned long localHost();
    virtual void pause(unsigned);

    friend class HttpRequest;
    friend class HelloRequest;
    friend class MonitorRequest;
    friend class PostRequest;
};

// ___________________________________________________________________________________

static char ANSWER_ERROR[] = "Bad answer";

class HttpRequest : public EventReceiver
{
public:
    HttpRequest(HttpPool *pool);
    void send();
    bool isReady() { return m_fetch_id == 0; }
protected:
    unsigned m_fetch_id;
    void *processEvent(Event*);
    virtual HttpPacket *packet()     = 0;
    virtual const char *url()		 = 0;
    virtual void data_ready(Buffer*) = 0;
    HttpPool *m_pool;
};

HttpRequest::HttpRequest(HttpPool *pool)
{
    m_fetch_id = 0;
    m_pool     = pool;
}

void HttpRequest::send()
{
    HttpPacket *p = packet();
    Buffer *postData = NULL;
    if (p){
        postData = new Buffer;
        unsigned short len = (unsigned short)(p->size + 12);
        *postData
        << len
        << HTTP_PROXY_VERSION
        << p->type
        << 0x00000000L
        << p->nSock;
        if (p->size)
            postData->pack(p->data, p->size);
        m_pool->queue.remove(p);
        delete p;
    }
    char headers[] = "Cache-control: no-store, no-cache\x00"
                     "Pragma: no-cache\x00\x00";
    m_fetch_id = fetch(url(), postData, headers);
}

void *HttpRequest::processEvent(Event *e)
{
    if (e->type() == EventFetchDone){
        fetchData *data = (fetchData*)(e->param());
        if (data->req_id != m_fetch_id)
            return NULL;
        if (data->result != 200){
            log(L_DEBUG, "Res: %u %s", data->result, url());
            m_pool->error(ANSWER_ERROR);
            return e->param();
        }
        m_fetch_id = 0;
        data_ready(data->data);
        return e->param();
    }
    return NULL;
}

unsigned long HttpPool::localHost()
{
    return 0;
}

void HttpPool::pause(unsigned)
{
}

// ______________________________________________________________________________________

class HelloRequest : public HttpRequest
{
public:
    HelloRequest(HttpPool *poll);
protected:
    virtual HttpPacket *packet();
    virtual const char *url();
    virtual void data_ready(Buffer*);
};

HelloRequest::HelloRequest(HttpPool *poll)
        : HttpRequest(poll)
{
    send();
}

HttpPacket *HelloRequest::packet()
{
    return NULL;
}

const char *HelloRequest::url()
{
    return "http://http.proxy.icq.com/hello";
}

void HelloRequest::data_ready(Buffer *bIn)
{
    bIn->incReadPos(12);
    unsigned long SID[4];
    (*bIn) >> SID[0] >> SID[1] >> SID[2] >> SID[3];
    char b[34];
    snprintf(b, sizeof(b), "%08lx%08lx%08lx%08lx", SID[0], SID[1], SID[2], SID[3]);
    m_pool->sid = b;
    bIn->unpack(m_pool->m_host);
    m_pool->request();
}

// ______________________________________________________________________________________

class MonitorRequest : public HttpRequest
{
public:
    MonitorRequest(HttpPool *pool);
protected:
    virtual HttpPacket *packet();
    virtual const char *url();
    virtual void data_ready(Buffer*);
    string sURL;
};

MonitorRequest::MonitorRequest(HttpPool *pool)
        : HttpRequest(pool)
{
    send();
}

HttpPacket *MonitorRequest::packet()
{
    return NULL;
}

const char *MonitorRequest::url()
{
    sURL  = "http://";
    sURL += m_pool->m_host.c_str();
    sURL += "/monitor?sid=";
    sURL += m_pool->sid.c_str();
    return sURL.c_str();
}

void MonitorRequest::data_ready(Buffer *bIn)
{
    m_pool->readn = 0;
    while (bIn->readPos() < bIn->size()){
        unsigned short len, ver, type;
        (*bIn) >> len >> ver >> type;
        bIn->incReadPos(8);
        len -= 12;
        if (len > (bIn->size() - bIn->readPos())){
            m_pool->error(ANSWER_ERROR);
            return;
        }
        if (ver != HTTP_PROXY_VERSION){
            m_pool->error(ANSWER_ERROR);
            return;
        }
        switch (type){
        case HTTP_PROXY_FLAP:
            if (len){
                char *data = bIn->data(bIn->readPos());
                m_pool->readData.pack(data, len);
                m_pool->readn += len;
                bIn->incReadPos(len);
            }
            break;
        case HTTP_PROXY_UNK1:
        case HTTP_PROXY_UNK2:
            if (len)
                bIn->incReadPos(len);
            break;
        default:
            m_pool->error(ANSWER_ERROR);
            return;
        }
    }
    m_pool->request();
}

// ______________________________________________________________________________________

class PostRequest : public HttpRequest
{
public:
    PostRequest(HttpPool *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *url();
    virtual void data_ready(Buffer *b);
    string sURL;
};

PostRequest::PostRequest(HttpPool *proxy)
        : HttpRequest(proxy)
{
    send();
}

HttpPacket *PostRequest::packet()
{
    if (m_pool->queue.size())
        return m_pool->queue.front();
    return NULL;
}

const char *PostRequest::url()
{
    sURL  = "http://";
    sURL += m_pool->m_host.c_str();
    sURL += "/data?sid=";
    sURL += m_pool->sid.c_str();
    sURL += "&seq=";
    char b[15];
    snprintf(b, sizeof(b), "%u", ++m_pool->seq);
    sURL += b;
    return sURL.c_str();
}

void PostRequest::data_ready(Buffer*)
{
    m_pool->request();
}

// ______________________________________________________________________________________

HttpPool::HttpPool()
{
    hello = NULL;
    monitor = NULL;
    post = NULL;
    state = None;
    seq = 0;
    readn = 0;
    nSock = 0;
}

HttpPool::~HttpPool()
{
    if (hello) delete hello;
    if (monitor) delete monitor;
    if (post) delete post;
    for (list<HttpPacket*>::iterator it = queue.begin(); it != queue.end(); ++it)
        delete *it;
}

int HttpPool::read(char *buf, unsigned size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == readData.size())
        readData.init(0);
    return size;
}

void HttpPool::write(const char *buf, unsigned size)
{
    queue.push_back(new HttpPacket(buf, (unsigned short)size, HTTP_PROXY_FLAP, nSock));
    request();
}

void HttpPool::close()
{
}

void HttpPool::connect(const char *host, unsigned short port)
{
    state = None;
    Buffer b;
    unsigned short len = (unsigned short)strlen(host);
    b << len << host << port;
    nSock++;
    queue.push_back(new HttpPacket(b.data(0), (unsigned short)(b.size()), HTTP_PROXY_LOGIN, nSock));
    if (sid.length()){
        unsigned char close_packet[] = { 0x2A, 0x04, 0x14, 0xAB, 0x00, 0x00 };
        queue.push_back(new HttpPacket((char*)close_packet, sizeof(close_packet), HTTP_PROXY_FLAP, 1));
        queue.push_back(new HttpPacket(NULL, 0, HTTP_PROXY_CONNECT, 1));
    }
    request();
}

void HttpPool::request()
{
    if (sid.length() == 0){
        if (hello == NULL) hello = new HelloRequest(this);
        return;
    }
    if (hello){
        delete hello;
        hello = NULL;
    }
    if (monitor && monitor->isReady()){
        delete monitor;
        monitor = NULL;
    }
    if (monitor == NULL)
        monitor = new MonitorRequest(this);
    if (post && post->isReady()){
        delete post;
        post = NULL;
    }
    if (queue.size() && (post == NULL))
        post = new PostRequest(this);
    if (readn && notify){
        if (state == None){
            state = Connected;
            notify->connect_ready();
        }
        readn = 0;
        notify->read_ready();
    }
}

Socket *ICQClient::createSocket()
{
    m_bHTTP = getUseHTTP();
    if (getAutoHTTP()){
        m_bHTTP = m_bFirstTry;
		if (!m_bFirstTry)
			m_bFirstTry = true;
	}
    if (m_bHTTP)
        return new HttpPool();
    return NULL;
}


