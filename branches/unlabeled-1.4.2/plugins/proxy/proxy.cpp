/***************************************************************************
                          proxy.cpp  -  description
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

#include "proxy.h"
#include "proxycfg.h"
#include "socket.h"

#include <qtabwidget.h>
#include <qobjectlist.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifndef INADDR_NONE
#define INADDR_NONE	0xFFFFFFFF
#endif

class Proxy : public Socket, public SocketNotify
{
public:
    Proxy(ProxyPlugin *plugin);
    ~Proxy();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    void setSocket(ClientSocket*);
    virtual void close();
    virtual unsigned long localHost();
    virtual void pause(unsigned);
    virtual Mode mode() { return Indirect; }
protected:
    virtual void write();
    virtual void write_ready();
    virtual void error_state(const char *text, unsigned code);
    virtual void proxy_connect_ready();
    void read(unsigned size, unsigned minsize=0);
    bool		m_bClosed;
    Socket		*m_sock;
    ProxyPlugin	*m_plugin;
    Buffer		bOut;
    Buffer		bIn;
};

Proxy::Proxy(ProxyPlugin *plugin)
{
    m_plugin   = plugin;
    m_sock     = NULL;
    m_bClosed  = false;
    m_plugin->proxies.push_back(this);
    bIn.packetStart();
    bOut.packetStart();
}

Proxy::~Proxy()
{
    if (m_sock)
        delete m_sock;
    for (list<Proxy*>::iterator it = m_plugin->proxies.begin(); it != m_plugin->proxies.end(); ++it){
        if (*it == this){
            m_plugin->proxies.erase(it);
            break;
        }
    }
}

void Proxy::setSocket(ClientSocket *s)
{
    m_sock = s->socket();
    s->setSocket(this);
    m_sock->setNotify(this);
}

int Proxy::read(char*, unsigned int)
{
    return 0;
}

void Proxy::write(const char*, unsigned int)
{
    log(L_WARN, "Proxy can't write");
    if (notify)
        notify->error_state(I18N_NOOP("Error proxy write"));
}

void Proxy::close()
{
    if (m_bClosed)
        return;
    m_bClosed = true;
    if (m_sock){
        m_sock->setNotify(notify);
        m_sock->close();
    }
}

unsigned long Proxy::localHost()
{
    return m_sock->localHost();
}

void Proxy::pause(unsigned n)
{
    m_sock->pause(n);
}

void Proxy::write_ready()
{
}

void Proxy::error_state(const char *err, unsigned code)
{
    if (notify)
        notify->error_state(err, code);
}

void Proxy::read(unsigned size, unsigned minsize)
{
    bIn.init(size);
    bIn.packetStart();
    int readn = m_sock->read(bIn.data(0), size);
    if ((readn != (int)size) || (minsize && (readn < (int)minsize))){
        if (notify) notify->error_state(I18N_NOOP("Error proxy read"));
        return;
    }
    log_packet(bIn, false, m_plugin->ProxyPacket);
}

void Proxy::write()
{
    log_packet(bOut, true, m_plugin->ProxyPacket);
    m_sock->write(bOut.data(0), bOut.size());
    bOut.init(0);
    bOut.packetStart();
}

void Proxy::proxy_connect_ready()
{
    static_cast<ClientSocket*>(notify)->setSocket(m_sock);
    m_sock = NULL;
    if (notify)
        notify->connect_ready();
    getSocketFactory()->remove(this);
}

// ______________________________________________________________________________________

class SOCKS4_Proxy : public Proxy
{
public:
    SOCKS4_Proxy(ProxyPlugin *plugin);
    virtual void connect(const char *host, int port);
protected:
    virtual void connect_ready();
    virtual void read_ready();

    string			m_host;
    unsigned short	m_port;

    enum State
    {
        None,
        Connect,
        WaitConnect
    };
    State m_state;
};

SOCKS4_Proxy::SOCKS4_Proxy(ProxyPlugin *plugin)
        : Proxy(plugin)
{
    m_state = None;
}

void SOCKS4_Proxy::connect(const char *host, int port)
{
    if (m_state != None){
        log(L_WARN, "Proxy::connect in bad state");
        if (notify) notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    m_host = host;
    m_port = port;
    log(L_DEBUG, "Connect to proxy SOCKS4 %s:%u", m_plugin->getHost(), m_plugin->getPort());
    m_sock->connect(m_plugin->getHost(), m_plugin->getPort());
    m_state = Connect;
}

void SOCKS4_Proxy::connect_ready()
{
    if (m_state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        if (notify) notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    unsigned long addr = inet_addr(m_host.c_str());
    if (addr == INADDR_NONE){
        struct hostent *hp = gethostbyname(m_host.c_str());
        if (hp) addr = *((unsigned long*)(hp->h_addr_list[0]));
    }
    bOut
    << (char)4
    << (char)1
    << m_port
    << (unsigned long)htonl(addr)
    << (char)0;
    m_state = WaitConnect;
}

void SOCKS4_Proxy::read_ready()
{
    if (m_state != WaitConnect) return;
    read(9, 4);
    char b1, b2;
    bIn >> b1 >> b2;
    if (b2 != 90){
        if (notify) notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    proxy_connect_ready();
}

// ______________________________________________________________________________________

class SOCKS5_Proxy : public Proxy
{
public:
    SOCKS5_Proxy(ProxyPlugin*);
    virtual void connect(const char *host, int port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    string m_host;
    unsigned short m_port;
    enum State
    {
        None,
        Connect,
        WaitAnswer,
        WaitAuth,
        WaitConnect
    };
    State m_state;
    void send_connect();
};

SOCKS5_Proxy::SOCKS5_Proxy(ProxyPlugin *plugin)
        : Proxy(plugin)
{
    m_state = None;
}

void SOCKS5_Proxy::connect(const char *host, int port)
{
    if (m_state != None){
        log(L_WARN, "Proxy::connect in bad state");
        if (notify) notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    m_host = host;
    m_port = port;
    log(L_DEBUG, "Connect to proxy SOCKS5 %s:%u", m_plugin->getHost(), m_plugin->getPort());
    m_sock->connect(m_plugin->getHost(), m_plugin->getPort());
    m_state = Connect;
}

void SOCKS5_Proxy::connect_ready()
{
    if (m_state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        if (notify)
            notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    bOut << (char)0x05;
    if (m_plugin->getAuth()) {
        bOut	<< (char)0x02
        << (char)0x00
        << (char)0x02;
    } else {
        bOut << (char) 0x01
        << (char)0x00;
    }
    m_state = WaitAnswer;
    write();
}

void SOCKS5_Proxy::read_ready()
{
    char b1, b2;
    switch (m_state){
    case WaitAnswer:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 == (char)0xFF)) {
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy connect"));
            return;
        }
        if (b2 == 0x02) {
            const char *user = m_plugin->getUser();
            const char *pswd = m_plugin->getPassword();
            bOut
            << (char)0x01
            << (char)strlen(user)
            << user
            << (char)strlen(pswd)
            << pswd;
            m_state = WaitAuth;
            write();
            return;
        }
        send_connect();
        return;
    case WaitAuth:
        read(2);
        bIn >> b1 >> b2;
        if ((b1 != 0x01) || (b2 != 0x00)) {
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy authorization"));
            return;
        }
        send_connect();
        return;
    case WaitConnect:
        read(10);
        bIn >> b1 >> b2;
        if ((b1 != 0x05) || (b2 != 0x00)) {
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy connect"));
            return;
        }
        proxy_connect_ready();
        return;
    default:
        break;
    }
}

void SOCKS5_Proxy::send_connect()
{
    unsigned long addr = inet_addr(m_host.c_str());
    bOut << (char)0x05
    << (char)0x01			/* CONNECT */
    << (char)0x00;			/* reserved */
    if (addr != INADDR_NONE){
        bOut << (char)0x01	    /* address type -- ip addr */
        << (unsigned long)htonl(addr);
    }else{
        bOut << (char)0x03		/* address type -- host name */
        << (char)m_host.length();
        bOut.pack(m_host.c_str(), m_host.length());
    }
    bOut << m_port;
    m_state = WaitConnect;
    write();
}

// ______________________________________________________________________________________

class HTTPS_Proxy : public Proxy
{
public:
    HTTPS_Proxy(ProxyPlugin *plugin);
    virtual void connect(const char *host, int port);
protected:
    virtual void connect_ready();
    virtual void read_ready();
    string m_host;
    unsigned short m_port;
    enum State
    {
        None,
        Connect,
        WaitConnect,
        WaitEmpty
    };
    State m_state;
    bool readLine(string &s);
};

HTTPS_Proxy::HTTPS_Proxy(ProxyPlugin *plugin)
        : Proxy(plugin)
{
    m_state = None;
}

void HTTPS_Proxy::connect(const char *host, int port)
{
    if (m_state != None){
        log(L_WARN, "Proxy::connect in bad state");
        if (notify)
            notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    m_host = host;
    m_port = port;
    log(L_DEBUG, "Connect to proxy HTTPS %s:%u", m_plugin->getHost(), m_plugin->getPort());
    m_sock->connect(m_plugin->getHost(), m_plugin->getPort());
    m_state = Connect;
}

static char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "0123456789+/";

string tobase64(const char *text)
{
    string out;
    const char *c;
    unsigned int tmp = 0;
    int n = 0;

    c = text;

    while (*c) {
        tmp = tmp << 8;
        tmp += *c;
        n++;

        if (n == 3) {
            out += alphabet[(tmp >> 18) & 0x3f];
            out += alphabet[(tmp >> 12) & 0x3f];
            out += alphabet[(tmp >> 6) & 0x3f];
            out += alphabet[tmp & 0x3f];
            tmp = 0;
            n = 0;
        }
        c++;
    }

    switch (n) {
    case 2:
        tmp <<= 8;
        out += alphabet[(tmp >> 18) & 0x3f];
        out += alphabet[(tmp >> 12) & 0x3f];
        out += alphabet[(tmp >> 6) & 0x3f];
        out += '=';
        break;
    case 1:
        tmp <<= 16;
        out += alphabet[(tmp >> 18) & 0x3f];
        out += alphabet[(tmp >> 12) & 0x3f];
        out += '=';
        out += '=';
        break;
    }
    return out;
}

void HTTPS_Proxy::connect_ready()
{
    if (m_state != Connect){
        log(L_WARN, "Proxy::connect_ready in bad state");
        if (notify)
            notify->error_state(I18N_NOOP("Error proxy connect"));
        return;
    }
    bIn.packetStart();
    bOut << "CONNECT "
    << m_host.c_str()
    << ":443 HTTP/1.0\r\n"
    << "User-Agent: Mozilla/4.08 [en]] (WinNT; U ;Nav)\r\n";
    if (m_plugin->getAuth()){
        string s;
        s = m_plugin->getUser();
        s += ":";
        s += m_plugin->getPassword();
        s = tobase64(s.c_str());
        bOut << "Proxy-Authorization: basic ";
        bOut << s.c_str();
        bOut << "\r\n";
        bOut << "Auth: basic ";
        bOut << s.c_str();
        bOut << "\r\n";
    }
    bOut << "\r\n";
    m_state = WaitConnect;
    write();
}

static char HTTP[] = "HTTP/";

void HTTPS_Proxy::read_ready()
{
    if (m_state == WaitConnect){
        string s;
        if (!readLine(s)) return;
        if (s.length() < strlen(HTTP)){
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy connect"));
            return;
        }
        const char *r = strchr(s.c_str(), ' ');
        if (r == NULL){
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy connect"));
            return;
        }
        r++;
        int code = atoi(r);
        if (code == 401){
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy authorization"));
            return;
        }
        if (code != 200){
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy connect"));
            return;
        }
        m_state = WaitEmpty;
    }
    if (m_state == WaitEmpty){
        for (;;){
            string s;
            if (!readLine(s)) return;
            if (s.length() == 0) break;
        }
        proxy_connect_ready();
    }
}

bool HTTPS_Proxy::readLine(string &s)
{
    for (;;){
        char c;
        int n = m_sock->read(&c, 1);
        if (n < 0){
            if (notify)
                notify->error_state(I18N_NOOP("Error proxy read"));
            return false;
        }
        if (n == 0) return false;
        if (c == '\r') continue;
        if (c == '\n') break;
        bIn << c;
    }
    log_packet(bIn, false, m_plugin->ProxyPacket);
    s.assign(bIn.data(0), bIn.size());
    bIn.init(0);
    bIn.packetStart();
    return true;
}

// ______________________________________________________________________________________

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
    HttpPacket(const char *data, unsigned short size, unsigned short type, unsigned long nSock);
    ~HttpPacket();
    char *data;
    unsigned short size;
    unsigned short type;
    unsigned long  nSock;
private:
    HttpPacket(HttpPacket&);
    void operator = (HttpPacket&);
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

class HttpPacket;
class HttpRequest;

class HTTP_Proxy : public Proxy
{
public:
    HTTP_Proxy(ProxyPlugin *plugin);
    ~HTTP_Proxy();
    virtual void connect(const char *host, int port);
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    virtual void close();
    virtual Mode mode() { return Web; }
protected:
    enum State
    {
        None,
        Connected
    };
    State state;
    virtual void request();
    virtual void read_ready();
    virtual void connect_ready();

    string sid;
    string m_host;

    list<HttpPacket*> queue;
    unsigned seq;
    unsigned readn;
    Buffer readData;

    HttpRequest *hello;
    HttpRequest *monitor;
    HttpRequest *post;

    unsigned long nSock;

    friend class HttpRequest;
    friend class HelloRequest;
    friend class MonitorRequest;
    friend class PostRequest;
};

class HttpRequest : public SocketNotify
{
public:
    HttpRequest(HTTP_Proxy *proxy);
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
    void error_state(const char *err, unsigned code = 0);
    bool readLine(string &s);
    Buffer bIn;
    unsigned data_size;
    HTTP_Proxy *m_proxy;
    Socket *m_sock;
};

HttpRequest::HttpRequest(HTTP_Proxy *proxy)
{
    data_size = 0;
    m_proxy = proxy;
    m_sock = getSocketFactory()->createSocket();
    m_sock->setNotify(this);
    state = WaitConnect;
    m_sock->connect(proxy->m_plugin->getHost(), proxy->m_plugin->getPort());
    bIn.packetStart();
}

HttpRequest::~HttpRequest()
{
    m_sock->close();
    getSocketFactory()->remove(m_sock);
}

void HttpRequest::write_ready()
{
}

void HttpRequest::connect_ready()
{
    if (state == WaitConnect)
        state = Connected;

    const char *h = host();
    HttpPacket *p = packet();

    Buffer bOut;
    bOut.packetStart();
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

    if (m_proxy->m_plugin->getAuth()){
        string s;
        s = m_proxy->m_plugin->getUser();
        s += ":";
        s += m_proxy->m_plugin->getPassword();
        s = tobase64(s.c_str());
        bOut << "Proxy-Authorization: basic ";
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
        << p->nSock;
        if (p->size)
            bOut.pack(p->data, p->size);
        m_proxy->queue.remove(p);
        delete p;
    }
    log_packet(bOut, true, m_proxy->m_plugin->ProxyPacket);
    m_sock->write(bOut.data(0), bOut.size());
    bOut.init(0);
}

bool HttpRequest::readLine(string &s)
{
    for (;;){
        char c;
        int n = m_sock->read(&c, 1);
        if (n < 0){
            error_state(I18N_NOOP("Error proxy connect"));
            return false;
        }
        if (n == 0) return false;
        bIn << c;
        if (c == '\n') break;
    }
    s = "";
    for (; bIn.readPos() < bIn.writePos(); ){
        char c;
        bIn.unpack(&c, 1);
        if ((c == '\r') || (c == '\n')) continue;
        s += c;
    }
    return true;
}

static char CONTENT_LENGTH[] = "Content-Length:";

void HttpRequest::read_ready()
{
    if (state == Connected){
        string s;
        if (!readLine(s)) return;
        if (s.length() < strlen(HTTP)){
            error_state(I18N_NOOP("Error proxy read"));
            return;
        }
        const char *r = strchr(s.c_str(), ' ');
        if (r == NULL){
            error_state(I18N_NOOP("Error proxy read"));
            return;
        }
        r++;
        int code = atoi(r);
        if (code == 401){
            log_packet(bIn, false, m_proxy->m_plugin->ProxyPacket);
            error_state(I18N_NOOP("Error proxy authorization"), 401);
            return;
        }
        if (code == 502){
            log_packet(bIn, false, m_proxy->m_plugin->ProxyPacket);
            error_state(I18N_NOOP("Error proxy read"));
            return;
        }
        if (code != 200){
            log_packet(bIn, false, m_proxy->m_plugin->ProxyPacket);
            error_state(I18N_NOOP("Error proxy connect"));
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
                log_packet(bIn, false, m_proxy->m_plugin->ProxyPacket);
                error_state(I18N_NOOP("Error proxy connect"));
                return;
            }
            if (n == 0) break;
            bIn.pack(b, n);
            data_size -= n;
        }
        if (data_size == 0){
            log_packet(bIn, false, m_proxy->m_plugin->ProxyPacket);
            state = None;
            data_ready();
        }
    }
}

void HttpRequest::error_state(const char *err, unsigned code)
{
    if (state == None) return;
    if (m_proxy->notify)
        m_proxy->notify->error_state(err, code);
}

// ______________________________________________________________________________________

class HelloRequest : public HttpRequest
{
public:
    HelloRequest(HTTP_Proxy *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *host();
    virtual const char *uri();
    virtual void data_ready();
};

HelloRequest::HelloRequest(HTTP_Proxy *proxy)
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
    bIn.unpack(m_proxy->m_host);
    m_proxy->request();
}

// ______________________________________________________________________________________

class MonitorRequest : public HttpRequest
{
public:
    MonitorRequest(HTTP_Proxy *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *host();
    virtual const char *uri();
    virtual void data_ready();
    string sURI;
};

MonitorRequest::MonitorRequest(HTTP_Proxy *proxy)
        : HttpRequest(proxy)
{
}

HttpPacket *MonitorRequest::packet()
{
    return NULL;
}

const char *MonitorRequest::host()
{
    return m_proxy->m_host.c_str();
}

const char *MonitorRequest::uri()
{
    sURI  = "/monitor?sid=";
    sURI += m_proxy->sid.c_str();
    return sURI.c_str();
}

void MonitorRequest::data_ready()
{
    m_proxy->readn = 0;
    while (bIn.readPos() < bIn.size()){
        unsigned short len, ver, type;
        bIn >> len >> ver >> type;
        bIn.incReadPos(8);
        len -= 12;
        if (len > (bIn.size() - bIn.readPos())){
            log(L_WARN, "Bad HTTP packet size %u (%u)", len, bIn.size() - bIn.readPos());
            error_state(I18N_NOOP("Error proxy read"));
            return;
        }
        if (ver != HTTP_PROXY_VERSION){
            log(L_WARN, "Bad HTTP packet version %X (%X)", ver, HTTP_PROXY_VERSION);
            error_state(I18N_NOOP("Error proxy read"));
            return;
        }
        switch (type){
        case HTTP_PROXY_FLAP:
            if (len){
                char *data = bIn.data(bIn.readPos());
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
            error_state(I18N_NOOP("Error proxy read"));
            return;
        }
    }
    m_proxy->request();
}

// ______________________________________________________________________________________

class PostRequest : public HttpRequest
{
public:
    PostRequest(HTTP_Proxy *proxy);
protected:
    virtual HttpPacket *packet();
    virtual const char *host();
    virtual const char *uri();
    virtual void data_ready();
    string sURI;
};

PostRequest::PostRequest(HTTP_Proxy *proxy)
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
    return m_proxy->m_host.c_str();
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
    m_proxy->request();
}

// ______________________________________________________________________________________

HTTP_Proxy::HTTP_Proxy(ProxyPlugin *plugin)
        : Proxy(plugin)
{
    hello = NULL;
    monitor = NULL;
    post = NULL;
    state = None;
    seq = 0;
    readn = 0;
    nSock = 0;
}

HTTP_Proxy::~HTTP_Proxy()
{
    if (hello) delete hello;
    if (monitor) delete monitor;
    if (post) delete post;
    for (list<HttpPacket*>::iterator it = queue.begin(); it != queue.end(); ++it)
        delete *it;
}

void HTTP_Proxy::read_ready()
{
}

void HTTP_Proxy::connect_ready()
{
}

int HTTP_Proxy::read(char *buf, unsigned int size)
{
    unsigned tail = readData.size() - readData.readPos();
    if (size > tail) size = tail;
    if (size == 0) return 0;
    readData.unpack(buf, size);
    if (readData.readPos() == readData.size())
        readData.init(0);
    return size;
}

void HTTP_Proxy::write(const char *buf, unsigned int size)
{
    queue.push_back(new HttpPacket(buf, size, HTTP_PROXY_FLAP, nSock));
    request();
}

void HTTP_Proxy::close()
{
}

void HTTP_Proxy::connect(const char *host, int port)
{
    state = None;
    Buffer b;
    unsigned short len = strlen(host);
    b << len << host << port;
    nSock++;
    queue.push_back(new HttpPacket(b.data(0), b.size(), HTTP_PROXY_LOGIN, nSock));
    if (sid.length()){
        unsigned char close_packet[] = { 0x2A, 0x04, 0x14, 0xAB, 0x00, 0x00 };
        queue.push_back(new HttpPacket((char*)close_packet, sizeof(close_packet), HTTP_PROXY_FLAP, 1));
        queue.push_back(new HttpPacket(NULL, 0, HTTP_PROXY_CONNECT, 1));
    }
    request();
}

void HTTP_Proxy::request()
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

// ______________________________________________________________________________________


Plugin *createProxyPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new ProxyPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Proxy"),
        I18N_NOOP("Plugin provides proxy support (SOCKS4, SOCKS5, HTTPS and HTTP)"),
        VERSION,
        createProxyPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*
typedef struct ProxyData
{
    unsigned long	Type;
	char			*Host;
	unsigned long	Port;
	unsigned		Auth;
	char			*User;
	char			*Password;
} ProxyData;
*/

static DataDef proxyData[] =
    {
        { "Type", DATA_ULONG, 1, PROXY_NONE },
        { "Host", DATA_STRING, 1, (unsigned)"proxy" },
        { "Port", DATA_ULONG, 1, 1080 },
        { "Auth", DATA_BOOL, 1, 0 },
        { "User", DATA_STRING, 1, 0 },
        { "Password", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ProxyPlugin::ProxyPlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(proxyData, &data, config);
    ProxyPacket = registerType();
    getContacts()->addPacketType(ProxyPacket, info.title);
}

ProxyPlugin::~ProxyPlugin()
{
    while (proxies.size()){
        delete proxies.front();
    }
    getContacts()->removePacketType(ProxyPacket);
    free_data(proxyData, &data);
}

void *ProxyPlugin::processEvent(Event *e)
{
    if (e->type() == EventSocketConnect){
        ConnectParam *p = (ConnectParam*)(e->param());
        list<Proxy*>::iterator it;
        for (it = proxies.begin(); it != proxies.end(); ++it){
            if ((*it)->notify == p->socket)
                return NULL;
        }
        Proxy *proxy = NULL;
        switch (getType()){
        case PROXY_SOCKS4:
            proxy = new SOCKS4_Proxy(this);
            break;
        case PROXY_SOCKS5:
            proxy = new SOCKS5_Proxy(this);
            break;
        case PROXY_HTTPS:
            proxy = new HTTPS_Proxy(this);
            break;
        case PROXY_HTTP:
            if (!strcmp(p->proto, "ICQ"))
                proxy = new HTTP_Proxy(this);
        }
        if (proxy){
            proxy->setSocket(p->socket);
            return e->param();
        }
    }
    if (e->type() == EventClientConfig){
        QTabWidget *tab = NULL;
        QWidget *w = (QWidget*)(e->param());
        QObjectList *l = w->queryList("QTabWidget");
        QObjectListIt it(*l);
        if (it.current() != NULL)
            tab = static_cast<QTabWidget*>(it.current());
        delete l;
        if (tab){
            QWidget *w = new ProxyConfig(tab, this, tab);
            QObject::connect(tab->topLevelWidget(), SIGNAL(apply()), w, SLOT(apply()));
        }
    }
    return NULL;
}

string ProxyPlugin::getConfig()
{
    return save_data(proxyData, &data);
}

QWidget *ProxyPlugin::createConfigWindow(QWidget *parent)
{
    return new ProxyConfig(parent, this, NULL);
}

#ifdef WIN32

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif


