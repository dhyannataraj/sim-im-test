/***************************************************************************
                          icqhttp.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "icqhttp.h"
#include "icqssl.h"
#include "log.h"

#ifndef WIN32
#include <stdio.h>
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b);
#endif

const unsigned short HTTP_PROXY_VERSION = 0x0443;

const unsigned short HTTP_PROXY_HELLO	   = 2;
const unsigned short HTTP_PROXY_LOGIN      = 3;
const unsigned short HTTP_PROXY_UNK1	   = 4;
const unsigned short HTTP_PROXY_FLAP       = 5;
const unsigned short HTTP_PROXY_CONNECT	   = 6;
const unsigned short HTTP_PROXY_UNK2       = 7;

class HttpPacket
{
public:
    HttpPacket(const char *data, unsigned short size, unsigned short type);
    ~HttpPacket();
    char *data;
    unsigned short size;
    unsigned short type;
private:
    HttpPacket(HttpPacket&);
    void operator = (HttpPacket&);
};

HttpPacket::HttpPacket(const char *_data, unsigned short _size, unsigned short _type)
{
    size = _size;
    type = _type;
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

class HttpRequest : public SocketNotify
{
public:
    HttpRequest(ICQ_HTTP_Proxy *proxy);
    ~HttpRequest();
    bool isReady() { return (state == None); }
protected:
    enum State{
        None,
        WaitConnect,
        Connected,
        ReadHeader,
        ReadData
    };
    State state;
    virtual HttpPacket *packet() = 0;
    virtual const char *host() = 0;
    virtual const char *uri() = 0;
    virtual void data_ready() = 0;
    void connect_ready();
    void read_ready();
    void write_ready();
    void error_state(SocketError err);
    bool readLine(string &s);
    Buffer bIn;
    unsigned data_size;
    ICQ_HTTP_Proxy *m_proxy;
    Socket *m_sock;
};

HttpRequest::HttpRequest(ICQ_HTTP_Proxy *proxy)
{
    data_size = 0;
    m_proxy = proxy;
    m_sock = proxy->factory->createSocket();
    m_sock->setNotify(this);
    state = WaitConnect;
    m_sock->connect(proxy->m_host.c_str(), proxy->m_port);
}

HttpRequest::~HttpRequest()
{
    m_sock->close();
    m_proxy->factory->remove(m_sock);
}

string tobase64(const char *text);

void HttpRequest::write_ready()
{
}

void HttpRequest::connect_ready()
{
    if (state == WaitConnect){
        state = Connected;
#ifdef USE_OPENSSL
        bool isSecure = m_proxy->isSecure();
        if (isSecure){
            SSLClient *ssl = new SSLClient(m_sock);
            if (!ssl->initHTTPS()){
                delete ssl;
                ssl = NULL;
                error_state(ErrorProxyConnect);
                return;
            }
            ssl->setNotify(this);
            ssl->connect();
            ssl->process();
            m_sock = ssl;
            return;
        }
#endif
    }

    const char *h = host();
    HttpPacket *p = packet();

    Buffer bOut;
    bOut << (p ? "POST" : "GET") << " http://" << h << uri() << " HTTP/1.1\r\n" <<
    "Host: " << h << "\r\n"
    "User-agent: Mozilla/4.08 [en] (WinNT; U ;Nav)\r\n"
    "Cache-control: no-store, no-cache\r\n"
    "Connection: close\r\n"
    "Pragma: no-cache\r\n";

    if (p){
        char b[15];
        snprintf(b, sizeof(b), "%u", p->size + 14);
        bOut << "Content-Length: " << b << "\r\n";
    }

    if (m_proxy->m_user.length()){
        string s;
        s = m_proxy->m_user.c_str();
        s += ":";
        s += m_proxy->m_passwd.c_str();
        s = tobase64(s.c_str());
        bOut << "Proxy-Auth: basic ";
        bOut << s.c_str();
        bOut << "\r\n";
    }
    bOut << "\r\n";
    if (p){
        unsigned short len = p->size + 12;
        bOut
        << len
        << HTTP_PROXY_VERSION
        << p->type
        << 0x00000000L
        << 0x00000001L;
        if (p->size)
            bOut.pack(p->data, p->size);
        m_proxy->queue.remove(p);
        delete p;
    }
    dumpPacket(bOut, 0, "Proxy write");
    m_sock->write(bOut.Data(0), bOut.size());
    bOut.init(0);
}

bool HttpRequest::readLine(string &s)
{
    for (;;){
        char c;
        int n = m_sock->read(&c, 1);
        if (n < 0){
            error_state(ErrorProxyConnect);
            return false;
        }
        if (n == 0) return false;
        if (c == '\r') continue;
        if (c == '\n') break;
        bIn << c;
    }
    dumpPacket(bIn, 0, "Proxy read");
    s.assign(bIn.Data(0), bIn.size());
    bIn.init(0);
    return true;
}

static char HTTP[] = "HTTP/";
static char CONTENT_LENGTH[] = "Content-Length:";

void HttpRequest::read_ready()
{
    if (state == Connected){
        string s;
        if (!readLine(s)) return;
        if (s.length() < strlen(HTTP)){
            error_state(ErrorRead);
            return;
        }
        const char *r = strchr(s.c_str(), ' ');
        if (r == NULL){
            error_state(ErrorRead);
            return;
        }
        r++;
        int code = atoi(r);
        if (code == 401){
            error_state(ErrorProxyAuth);
            return;
        }
        if (code == 502){
            error_state(ErrorRead);
            return;
        }
        if (code != 200){
            error_state(ErrorProxyConnect);
            return;
        }
        state = ReadHeader;
    }
    if (state == ReadHeader){
        for (;;){
            string s;
            if (!readLine(s)) return;
            if (s.length() == 0){
                state = ReadData;
                break;
            }
            string h = s.substr(0, strlen(CONTENT_LENGTH));
            if (!strcasecmp(h.c_str(), CONTENT_LENGTH)){
                h = s.substr(strlen(CONTENT_LENGTH), s.size());
                for (const char *p = h.c_str(); *p; p++){
                    if ((*p >= '0') && (*p <= '9')){
                        data_size = atol(p);
                        break;
                    }
                }
            }
        }
    }
    if (state == ReadData){
        while (data_size > 0){
            char b[2048];
            unsigned tail = data_size;
            if (tail > sizeof(b)) tail = sizeof(b);
            int n = m_sock->read(b, tail);
            if (n < 0){
                error_state(ErrorProxyConnect);
                return;
            }
            if (n == 0) break;
            bIn.pack(b, n);
            data_size -= n;
        }
        if (data_size == 0){
            state = None;
            data_ready();
        }
    }
}

void HttpRequest::error_state(SocketError err)
{
    if (state == None) return;
    if (m_proxy->notify) m_proxy->notify->error_state(err);
}

// ______________________________________________________________________________________

class HelloRequest : public HttpRequest
{
public:
    HelloRequest(ICQ_HTTP_Proxy *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *host();
    virtual const char *uri();
    virtual void data_ready();
};

HelloRequest::HelloRequest(ICQ_HTTP_Proxy *proxy)
        : HttpRequest(proxy)
{
}

HttpPacket *HelloRequest::packet()
{
    return NULL;
}

const char *HelloRequest::host()
{
    return "http.proxy.icq.com";
}

const char *HelloRequest::uri()
{
    return "/hello";
}

void HelloRequest::data_ready()
{
    bIn.incReadPos(12);
    unsigned long SID[4];
    bIn >> SID[0] >> SID[1] >> SID[2] >> SID[3];
    char b[34];
    snprintf(b, sizeof(b), "%08lx%08lx%08lx%08lx", SID[0], SID[1], SID[2], SID[3]);
    m_proxy->sid = b;
    bIn.unpack(m_proxy->m_proxyHost);
    m_proxy->request();
}

// ______________________________________________________________________________________

class MonitorRequest : public HttpRequest
{
public:
    MonitorRequest(ICQ_HTTP_Proxy *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *host();
    virtual const char *uri();
    virtual void data_ready();
    string sURI;
};

MonitorRequest::MonitorRequest(ICQ_HTTP_Proxy *proxy)
        : HttpRequest(proxy)
{
}

HttpPacket *MonitorRequest::packet()
{
    return NULL;
}

const char *MonitorRequest::host()
{
    return m_proxy->m_proxyHost.c_str();
}

const char *MonitorRequest::uri()
{
    sURI  = "/monitor?sid=";
    sURI += m_proxy->sid.c_str();
    return sURI.c_str();
}

void MonitorRequest::data_ready()
{
    dumpPacket(bIn, 0, "Proxy read");
    m_proxy->readn = 0;
    while (bIn.readPos() < bIn.size()){
        unsigned short len, ver, type;
        bIn >> len >> ver >> type;
        bIn.incReadPos(8);
        len -= 12;
        if (len > (bIn.size() - bIn.readPos())){
            log(L_WARN, "Bad HTTP packet size %u (%u)", len, bIn.size() - bIn.readPos());
            error_state(ErrorRead);
            return;
        }
        if (ver != HTTP_PROXY_VERSION){
            log(L_WARN, "Bad HTTP packet version %X (%X)", ver, HTTP_PROXY_VERSION);
            error_state(ErrorRead);
            return;
        }
        switch (type){
        case HTTP_PROXY_FLAP:
            if (len){
                char *data = bIn.Data(bIn.readPos());
                m_proxy->readData.pack(data, len);
                m_proxy->readn += len;
                bIn.incReadPos(len);
            }
            break;
        case HTTP_PROXY_UNK1:
        case HTTP_PROXY_UNK2:
            if (len)
                bIn.incReadPos(len);
            break;
        default:
            log(L_WARN, "Bad HTTP packet type %u", type);
            error_state(ErrorRead);
            return;
        }
    }
    m_proxy->request();
}

// ______________________________________________________________________________________

class PostRequest : public HttpRequest
{
public:
    PostRequest(ICQ_HTTP_Proxy *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *host();
    virtual const char *uri();
    virtual void data_ready();
    string sURI;
};

PostRequest::PostRequest(ICQ_HTTP_Proxy *proxy)
        : HttpRequest(proxy)
{
}

HttpPacket *PostRequest::packet()
{
    if (m_proxy->queue.size()) return m_proxy->queue.front();
    return NULL;
}

const char *PostRequest::host()
{
    return m_proxy->m_proxyHost.c_str();
}

const char *PostRequest::uri()
{
    sURI  = "/data?sid=";
    sURI += m_proxy->sid.c_str();
    sURI += "&seq=";
    char b[15];
    snprintf(b, sizeof(b), "%u", ++m_proxy->seq);
    sURI += b;
    return sURI.c_str();
}

void PostRequest::data_ready()
{
    if (bIn.size())
        log(L_WARN, "Bad answer size for post data");
    m_proxy->request();
}

// ______________________________________________________________________________________

ICQ_HTTP_Proxy::ICQ_HTTP_Proxy(SocketFactory *_factory, const char *host, unsigned short port, const char *user, const char *passwd)
{
    m_host = host;
    m_port  = port;
    m_user = user;
    m_passwd = passwd;
    factory = _factory;
    hello = NULL;
    monitor = NULL;
    post = NULL;
    state = None;
    seq = 0;
    readn = 0;
}

ICQ_HTTP_Proxy::~ICQ_HTTP_Proxy()
{
    if (hello) delete hello;
    if (monitor) delete monitor;
    if (post) delete post;
    for (list<HttpPacket*>::iterator it = queue.begin(); it != queue.end(); ++it)
        delete *it;
}

void ICQ_HTTP_Proxy::read_ready()
{
}

void ICQ_HTTP_Proxy::connect_ready()
{
}

int ICQ_HTTP_Proxy::read(char *buf, unsigned int size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == readData.size())
        readData.init(0);
    return size;
}

void ICQ_HTTP_Proxy::write(const char *buf, unsigned int size)
{
    queue.push_back(new HttpPacket(buf, size, HTTP_PROXY_FLAP));
    request();
}

void ICQ_HTTP_Proxy::connect(const char *host, int port)
{
    Buffer b;
    unsigned short len = strlen(host);
    b << len << host << port;
    queue.push_back(new HttpPacket(b.Data(0), b.size(), HTTP_PROXY_LOGIN));
    if (sid.length()){
        unsigned char close_packet[] = { 0x2A, 0x04, 0x14, 0xAB, 0x00, 0x00 };
        queue.push_back(new HttpPacket((char*)close_packet, sizeof(close_packet), HTTP_PROXY_FLAP));
        queue.push_back(new HttpPacket(NULL, 0, HTTP_PROXY_CONNECT));
    }
    request();
}

void ICQ_HTTP_Proxy::request()
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

bool ICQ_HTTP_Proxy::isSecure()
{
    return false;
}

#ifdef USE_OPENSSL

ICQ_HTTPS_Proxy::ICQ_HTTPS_Proxy(SocketFactory *factory, const char *host, unsigned short port, const char *user, const char *passwd)
        : ICQ_HTTP_Proxy(factory, host, port, user, passwd)
{
}

bool ICQ_HTTPS_Proxy::isSecure()
{
    return true;
}

#endif
