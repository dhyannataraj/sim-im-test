/***************************************************************************
                          fetch.cpp  -  description
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

#include "simapi.h"

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#endif
#include <time.h>
#include <qthread.h>
#include <qtimer.h>

#include "fetch.h"
#include "buffer.h"
#include "log.h"
#include "misc.h"
#include "socket.h"

using namespace std;
using namespace SIM;

const unsigned HTTPPacket   = 0x100;

typedef std::map<my_string, QString> HEADERS_MAP;

class FetchThread;

class FetchClientPrivate : public ClientSocketNotify, public QObject
{
public:
    FetchClientPrivate(FetchClient*);
    ~FetchClientPrivate();
protected:
    void fetch(const QString &url, const QString &headers, Buffer *postData, bool bRedirect);
    void _fetch(const QString &headers = QString::null, Buffer *postData = NULL, bool bRedirect = false);
    void stop();
    FetchClient *m_client;
    void		fail();
    QString		m_uri;
    QString		m_hIn;
    HEADERS_MAP	m_hOut;
    unsigned	m_code;
    bool        m_bRedirect;
    unsigned    m_sendTime;
    unsigned    m_sendSize;
    unsigned	m_speed;
#ifdef WIN32
    FetchThread	*m_thread;
    QString		m_err;
    unsigned	m_errCode;
    bool event(QEvent* event);
#endif
    virtual bool error_state(const QString &err, unsigned code = 0);
    virtual void connect_ready();
    virtual void packet_ready();
    virtual void write_ready();
    bool read_line(QCString &line);
    ClientSocket *m_socket;
    unsigned	m_postSize;
    unsigned	m_received;
    unsigned	m_id;
    bool		m_bDone;
    unsigned	m_size;
    void		addHeader(const QString &key, const QString &value);
    bool		findHeader(const QString &key);
    Buffer		m_data;
    Buffer		*m_postData;
    enum State{
#ifdef USE_OPENSSL
        SSLConnect,
#endif
        None,
        Header,
        Data,
        Done,
        Redirect
    };
    State		m_state;
#ifdef USE_OPENSSL
    bool		m_bHTTPS;
#endif
    friend class FetchManager;
    friend class FetchThread;
    friend class FetchClient;
};

#ifdef WIN32
#include <windows.h>
#include <wininet.h>

static HINTERNET hInet = NULL;

class FetchThread : public QThread
{
public:
    FetchThread(FetchClientPrivate *client);
    ~FetchThread();
    bool m_bClose;
    void close();
protected:
    HINTERNET hCon;
    HINTERNET hReq;
    void error(const char *name);
    void run();
    FetchClientPrivate *m_client;
};

FetchThread::FetchThread(FetchClientPrivate *client)
{
    m_client = client;
    m_bClose = false;
    hCon = NULL;
    hReq = NULL;
}

FetchThread::~FetchThread()
{
    close();
}

void FetchThread::close()
{
    if (hReq){
        InternetCloseHandle(hReq);
        hReq = NULL;
    }
    if (hCon){
        InternetCloseHandle(hCon);
        hCon = NULL;
    }
}

void FetchThread::error(const char *name)
{
    m_client->m_errCode = GetLastError();
    m_client->m_err     = name;
    close();
    FetchManager::manager->done(m_client->m_client);
}

void FetchThread::run()
{
    QString headers;
    DWORD flags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_COOKIES;
    if (!m_client->m_bRedirect)
        flags |= INTERNET_FLAG_NO_AUTO_REDIRECT;
    QString verb = "GET";
    unsigned postSize = m_client->m_client->post_size();
    if (postSize != NO_POSTSIZE)
        verb = "POST";
    URL_COMPONENTS url;
    memset(&url, 0, sizeof(url));
    url.dwStructSize = sizeof(url);
    WCHAR host[256];
    url.lpszHostName      = host;
    url.dwHostNameLength  = sizeof(host)/sizeof(WCHAR);
    WCHAR user[256];
    url.lpszUserName      = user;
    url.dwUserNameLength  = sizeof(user)/sizeof(WCHAR);
    WCHAR pass[256];
    url.lpszPassword	  = pass;
    url.dwPasswordLength  = sizeof(pass)/sizeof(WCHAR);
    WCHAR urlPath[1024];
    url.lpszUrlPath		  = urlPath;
    url.dwUrlPathLength   = sizeof(urlPath)/sizeof(WCHAR);
    WCHAR extra[1024];
    url.lpszExtraInfo	  = extra;
    url.dwExtraInfoLength = sizeof(extra)/sizeof(WCHAR);
    if (!InternetCrackUrl((LPCWSTR)m_client->m_uri.ucs2(), 0, ICU_DECODE, &url)){
        error("InternetCrackUrl");
        return;
    }
    if (url.nScheme == INTERNET_SCHEME_HTTPS){
        flags |= INTERNET_FLAG_SECURE;
    }else if (url.nScheme != INTERNET_SCHEME_HTTP){
        error("Unsupported scheme");
        return;
    }
    hCon = InternetConnect(hInet, url.lpszHostName, url.nPort,
                            url.lpszUserName, url.lpszPassword, INTERNET_SERVICE_HTTP, 0, 0);
    if (hCon == NULL){
        error("InternetConnect");
        return;
    }
    QString uri = QString::fromUcs2((unsigned short*)url.lpszUrlPath);
    if (url.lpszExtraInfo)
        uri += QString::fromUcs2((unsigned short*)url.lpszExtraInfo);
    hReq = HttpOpenRequest(hCon, (LPCWSTR)verb.ucs2(), (LPCWSTR)uri.ucs2(), NULL, NULL, NULL, flags, 0);
    if (hReq == NULL){
        error("HttpOpenRequest error");
        return;
    }
    for (HEADERS_MAP::iterator it = m_client->m_hOut.begin(); it != m_client->m_hOut.end(); ++it){
        QString name = (*it).first.str();
        if (!headers.isEmpty())
            headers += "\r\n";
        headers += name;
        headers += ": ";
        headers += (*it).second;
    }
    Buffer b;
    b.packetStart();
    b << verb.latin1() << " " << uri.latin1() << " HTTP/1.0\r\n" << headers.latin1() << "\r\n";
    EventLog::log_packet(b, true, HTTPPacket);
    for (;;){
        if (postSize != NO_POSTSIZE){
            INTERNET_BUFFERS BufferIn;
            memset(&BufferIn, 0, sizeof(BufferIn));
            BufferIn.dwStructSize    = sizeof(INTERNET_BUFFERS);
            BufferIn.lpcszHeader     = (LPCWSTR)headers.ucs2();
            BufferIn.dwHeadersLength = headers.length();
            BufferIn.dwHeadersTotal  = headers.length();
            BufferIn.dwBufferTotal   = (postSize != NO_POSTSIZE) ? postSize : 0;
            if (!HttpSendRequestEx(hReq, &BufferIn, NULL, HSR_INITIATE | HSR_SYNC, 0)){
                error("HttpSendRequestEx");
                return;
            }
            unsigned size = postSize;
            while (size){
                char buff[4096];
                unsigned tail = size;
                if (tail > sizeof(buff))
                    tail = sizeof(buff);
                const char *data = m_client->m_client->read_data(buff, tail);
                if (data == NULL){
                    error("ReadStreamError");
                    return;
                }
                DWORD res;
                if (m_bClose)
                    return;
                if (!InternetWriteFile(hReq, (void*)data, tail, &res)){
                    error("InternetWriteFile");
                    return;
                }
                if (m_client->m_speed){
                    m_client->m_sendSize += tail;
                    time_t now = time(NULL);
                    if ((unsigned)now != m_client->m_sendTime){
                        m_client->m_sendTime = now;
                        m_client->m_sendSize = 0;
                    }
                    if (m_client->m_sendSize > (m_client->m_speed << 18)){
                        Sleep(1000);
                        return;
                    }
                }
                size -= tail;
            }
            if (m_bClose)
                return;
            if(!HttpEndRequest(hReq, NULL, 0, 0)){
                error("HttpEndRequest");
                return;
            }
        }else{
            if (!HttpSendRequest(hReq, (LPCWSTR)headers.ucs2(), headers.length(), NULL, 0)){
                error("HttpSendRequest");
                return;
            }
        }
        DWORD dwCode;
        DWORD dwSize = sizeof(dwCode);
        HttpQueryInfo (hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwCode, &dwSize, NULL);
        if (dwCode != HTTP_STATUS_PROXY_AUTH_REQ)
            break;
        DWORD res = InternetErrorDlg ( GetDesktopWindow(),
                                       hReq, ERROR_INTERNET_INCORRECT_PASSWORD,
                                       FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                                       FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
                                       FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL);
        if (res != ERROR_INTERNET_FORCE_RETRY)
            break;
    }

    DWORD size = 0;
    DWORD err  = 0;
    HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &size, 0);
    err = GetLastError();
    if (err != ERROR_INSUFFICIENT_BUFFER ){
        error("HttpQueryInfo");
        return;
    }
    QByteArray ba(size);
    if (!HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS_CRLF, ba.data(), &size, 0)){
        error("HttpQueryInfo");
        return;
    }
    QString str = QString::fromUcs2((unsigned short*)ba.data());
    QCString cstr = str.latin1();
    Buffer buf(cstr.length() + 1);
    memcpy(buf.data(), cstr.data(), cstr.length() + 1);
    EventLog::log_packet(buf, false, HTTPPacket);
    getToken(str, ' '); // HTTP/1.1
    QString line = getToken(str, ' ');  // 200 (or an error code)
    m_client->m_code = line.toLong();
    m_client->m_hIn = str;
    if (line.isEmpty()){
        error("Bad answer");
        return;
    }
    for (;;){
        DWORD readn;
        char buff[4096];
        if (!InternetReadFile(hReq, buff, sizeof(buff), &readn)){
            error("InternetReadFile");
            return;
        }
        if (m_bClose)
            return;
        if (readn == 0)
            break;
        if (!m_client->m_client->write_data(buff, readn)){
            error("Write error");
            return;
        }
    }
    /* Signals / Slots aren't thread-safe - need to use an event */
    close();
    QCustomEvent* ce = new QCustomEvent(Q_EVENT_SIM_FETCH_DONE);
    ce->setData(m_client->m_client);
    postEvent(m_client, ce);
}

#endif

#ifdef USE_OPENSSL

class HTTPSClient : public SSLClient
{
public:
    HTTPSClient(Socket *s);
protected:
    virtual bool initSSL();
};

HTTPSClient::HTTPSClient(Socket *s)
        : SSLClient(s)
{
}

bool HTTPSClient::initSSL()
{
#if SSLEAY_VERSION_NUMBER < 0x0800
    mpCTX = SSL_CTX_new();
    if (mpCTX == NULL)
        return false;
    X509_set_default_verify_paths(mpCTX->cert);
#else
SSLeay_add_ssl_algorithms();
    mpCTX = SSL_CTX_new(SSLv23_client_method());
    SSL_CTX_set_options(mpCTX, SSL_OP_ALL);
    SSL_CTX_set_default_verify_paths(mpCTX);
#endif /* SSLEAY_VERSION_NUMBER < 0x0800 */
#if SSLEAY_VERSION_NUMBER >= 0x00905100
    if (RAND_status() == 0) {
        time_t t;
        long l,seed;
        t = time(NULL);
        RAND_seed((unsigned char *)&t, sizeof(time_t));
        RAND_bytes((unsigned char *)&seed, sizeof(long));
        srand(seed);
        while (RAND_status() == 0) {
            l = rand();
            RAND_seed((unsigned char *)&l, sizeof(long));
        }
    }
#endif /* SSLEAY_VERSION_NUMBER >= 0x00905100 */
    mpSSL = SSL_new(mpCTX);
    if(!mpSSL)
        return false;
#if SSLEAY_VERSION_NUMBER >= 0x0900
    mpSSL->options|=SSL_OP_NO_TLSv1;
#endif
    return true;
}

#endif

FetchManager *FetchManager::manager = NULL;

const unsigned UNKNOWN_SIZE = (unsigned)(-1);

static char _HTTP[] = "HTTP";

static list<FetchClientPrivate*> *m_done = NULL;

FetchManager::FetchManager()
{
    m_done = new list<FetchClientPrivate*>;
    user_agent = "Mozilla/4.0 (" PACKAGE "/" VERSION " ";
    user_agent += get_os_version();
    user_agent += ')';
#ifdef WIN32
    hInet = InternetOpen((LPCWSTR)user_agent.ucs2(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hInet == NULL)
        log(L_WARN, "Internet open error %u", GetLastError());
#endif
    getContacts()->addPacketType(HTTPPacket, _HTTP, true);
}

FetchManager::~FetchManager()
{
#ifdef WIN32
    if (hInet)
        InternetCloseHandle(hInet);
#endif
    getContacts()->removePacketType(HTTPPacket);
    delete m_done;
}

void FetchManager::done(FetchClient *client)
{
    m_done->push_back(client->p);
    QTimer::singleShot(0, this, SLOT(timeout()));
}

void FetchManager::timeout()
{
    log(L_DEBUG,"timeout!");
    list<FetchClientPrivate*> done = *m_done;
    m_done->clear();
    for (list<FetchClientPrivate*>::iterator it = done.begin(); it != done.end(); ++it){
        if ((*it)->error_state(""))
            delete *it;
    }
}

FetchClient::FetchClient()
{
    p = new FetchClientPrivate(this);
}

FetchClient::~FetchClient()
{
    if (p){
        p->stop();
        p->m_client = NULL;
        delete p;
    }
}

void FetchClient::set_speed(unsigned speed)
{
    p->m_speed = speed;
}

void FetchClient::fetch(const QString &url, const QString &headers, Buffer *postData, bool bRedirect)
{
    getSocketFactory()->checkState();
    p->fetch(url, headers, postData, bRedirect);
}

void FetchClient::stop()
{
    p->stop();
}

bool FetchClient::isDone()
{
    return p->m_bDone;
}

FetchClientPrivate::FetchClientPrivate(FetchClient *client)
{
    m_client = client;
    m_socket = NULL;
    m_bDone  = true;
    m_postData = NULL;
    m_state  = None;
    m_code   = 0;
    m_size   = UNKNOWN_SIZE;
    m_postSize = 0;
    m_speed  = 0;
#ifdef WIN32
    m_thread	= NULL;
#endif
}

#ifdef WIN32
bool FetchClientPrivate::event(QEvent* e)
{
    if (e->type() == Q_EVENT_SIM_FETCH_DONE) {
        FetchClient *client = (FetchClient*)((QCustomEvent*)e)->data();
        FetchManager::manager->done(client);
        return true;
    }
    return QObject::event(e);
}
#endif

void FetchClientPrivate::fetch(const QString &url, const QString &headers, Buffer *postData, bool bRedirect)
{
    m_uri = url;
    _fetch(headers, postData, bRedirect);
}

void FetchClientPrivate::_fetch(const QString &headers, Buffer *postData, bool bRedirect)
{
    stop();
    m_bDone = false;
    m_data.init(0);
    m_data.packetStart();
    m_postData  = postData;
    m_bRedirect = bRedirect;
    m_postSize  = 0;
    m_sendTime	= 0;
    m_sendSize  = 0;
#ifdef WIN32
    m_thread	= NULL;
#endif
    if (!headers.isEmpty()){
        QString head = headers;
        while (!head.isEmpty()){
            QString header = getToken(head, '\n');
            QString key = getToken(header, ':');
            header = header.stripWhiteSpace();
            addHeader(key, header);
        }
    }
#ifdef WIN32
    m_errCode = 0;
    if (hInet){
        m_state  = Data;
        m_thread = new FetchThread(this);
        m_thread->start();
        return;
    }
#endif
    m_received = 0;
    m_socket = new ClientSocket(this);
#ifdef USE_OPENSSL
    m_bHTTPS = false;
#endif
    QString proto, host, user, pass, uri, extra;
    unsigned short port;
    if (!FetchClient::crackUrl(m_uri, proto, host, port, user, pass, uri, extra)){
        m_socket->error_state(I18N_NOOP("Bad URL"));
        return;
    }
    if (proto != "http"){
#ifdef USE_OPENSSL
        if (proto == "https"){
            m_bHTTPS = true;
        }else{
#endif
            log(L_WARN, "Unsupported protocol %s", (const char*)m_uri.local8Bit());
            return;
#ifdef USE_OPENSSL
        }
#endif
    }
    log(L_DEBUG, "Start connect %s:%u", host.latin1(), port);
    m_socket->connect(host, port, (TCPClient*)(-1));
}

FetchClientPrivate::~FetchClientPrivate()
{
    stop();
    list<FetchClientPrivate*>::iterator it;
    for (it = m_done->begin(); it != m_done->end(); ++it){
        if ((*it) == this){
            m_done->erase(it);
            break;
        }
    }
    if (m_client){
        m_client->p = NULL;
        delete m_client;
    }
}

void FetchClientPrivate::stop()
{
    m_hIn = QString::null;
#ifdef WIN32
    if (m_thread){
        m_thread->close();
        m_thread->m_bClose = true;
        m_thread->wait();
        delete m_thread;
        m_thread = NULL;
    }
#endif
    if (m_socket){
        delete m_socket;
        m_socket = NULL;
    }
    if (m_postData){
        delete m_postData;
        m_postData = NULL;
    }
    m_bDone = true;
    m_state = None;
}

bool FetchClient::crackUrl(const QString &_url, QString &protocol, QString &host, unsigned short &port, QString &user, QString &pass, QString &uri, QString &extra)
{
    port = 80;
    QString url = _url;
    protocol = getToken(url, ':', false);
    if (url.left(2) != "//")
        return false;
    url = url.mid(2);
    host = getToken(url, '/', false);
    extra = "/";
    extra += url;
    uri = getToken(extra, '?', false);
    user = getToken(host, '@', false);
    if (host.isEmpty()){
        host = user;
        user = QString::null;
    }else{
        pass = user;
        user = getToken(pass, ':', false);
    }
    if (protocol == "https")
        port = 443;
    QString h = getToken(host, ':');
    if (!host.isEmpty()){
        port = host.toUShort();
        if (port == 0)
            return false;
    }
    host = h;
    return true;
}

void FetchClientPrivate::addHeader(const QString &key, const QString &value)
{
    HEADERS_MAP::iterator it = m_hOut.find(key);
    if (it == m_hOut.end()){
        m_hOut.insert(HEADERS_MAP::value_type(key, value));
    }else{
        (*it).second = value;
    }
}

bool FetchClientPrivate::findHeader(const QString &key)
{
    HEADERS_MAP::iterator it = m_hOut.find(key);
    return (it != m_hOut.end());
}

QCString basic_auth(const QString &user, const QString &pass)
{
    QString auth = user + ':' + pass;
    Buffer from(auth.local8Bit());
    QCString cstr = Buffer::toBase64(from);
    return cstr;
}

bool FetchClientPrivate::error_state(const QString &err, unsigned)
{
    if (m_state == None)
        return false;
    if (m_state == Redirect){
        if (m_socket){
            delete m_socket;
            m_socket = NULL;
        }
        m_code = 0;
        m_hIn  = QString::null;
        m_state = None;
        _fetch();
        return false;
    }
    if ((m_state != Done) && ((m_state != Data) || (m_size != UNKNOWN_SIZE))){
        m_code = 0;
        log(L_DEBUG, "HTTP: %s", err.local8Bit().data());
    }
    m_bDone = true;
    m_state = None;
    if (m_socket)
        m_socket->close();
    return m_client->done(m_code, m_data, m_hIn);
}

void FetchClientPrivate::connect_ready()
{
#ifdef USE_OPENSSL
    if ((m_state == None) & m_bHTTPS){
        m_socket->setRaw(true);
        m_socket->readBuffer().init(0);
        HTTPSClient *https = new HTTPSClient(m_socket->socket());
        if (!https->init()){
            m_socket->error_state(I18N_NOOP("Can't initialize HTTPS"));
            return;
        }
        m_state = SSLConnect;
        m_socket->setSocket(https);
        https->connect();
        https->process();
        return;
    }
#endif
    log(L_DEBUG, "HTTP connect ready");
    m_socket->setRaw(true);
    m_socket->writeBuffer().packetStart();

    QString proto, host, user, pass, uri, extra;
    unsigned short port;
    FetchClient::crackUrl(m_uri, proto, host, port, user, pass, uri, extra);
    if (!extra.isEmpty()){
        uri += '?';
        uri += extra;
    }
    unsigned postSize = m_client->post_size();
    m_socket->writeBuffer()
    << ((postSize != NO_POSTSIZE) ? "POST " : "GET ")
    << uri.data()
    << " HTTP/1.0\r\n";
    if (!findHeader("Host"))
        m_socket->writeBuffer()
        << "Host: "
        << host.data()
        << "\r\n";
    if (!findHeader("User-Agent"))
        m_socket->writeBuffer()
        << "User-Agent: " << FetchManager::manager->user_agent.latin1() << "\r\n";
    if (!findHeader("Authorization") && !user.isEmpty())
        m_socket->writeBuffer()
        << "Authorization: basic "
        << basic_auth(user.data(), pass.data()).data()
        << "\r\n";
    if (postSize != NO_POSTSIZE){
        if (!findHeader("Content-Length"))
            m_socket->writeBuffer()
            << "Content-Length: "
            << (const char*)QString::number(postSize).latin1()
            << "\r\n";
        m_postSize = postSize;
    }
    for (HEADERS_MAP::iterator it = m_hOut.begin(); it != m_hOut.end(); ++it){
        m_socket->writeBuffer()
        << (*it).first.str().latin1()
        << ": "
        << (*it).second.latin1()
        << "\r\n";
    }
    m_socket->writeBuffer()
    << "\r\n";
    EventLog::log_packet(m_socket->writeBuffer(), true, HTTPPacket);
    m_socket->write();
    m_socket->readBuffer().init(0);
    m_socket->readBuffer().packetStart();
}

void FetchClientPrivate::write_ready()
{
    if (m_postSize == 0)
        return;
    char buff[4096];
    unsigned tail = m_postSize;
    if (tail > sizeof(buff))
        tail = sizeof(buff);
    const char *data = m_client->read_data(buff, tail);
    if (data == NULL){
        m_socket->error_state(I18N_NOOP("Read error"));
        return;
    }
    m_postSize -= tail;
    m_socket->writeBuffer().pack(data, tail);
    m_socket->write();
    if (m_speed){
        m_sendSize += tail;
        time_t now = time(NULL);
        if ((unsigned)now != m_sendTime){
            m_sendTime = now;
            m_sendSize = 0;
        }
        if (m_sendSize > (m_speed << 18)){
            m_socket->pause(1);
            return;
        }
    }
}

void FetchClientPrivate::packet_ready()
{
    if (m_socket->readBuffer().readPos() == m_socket->readBuffer().writePos())
        return;
    for (;;){
        if (m_state == Data){
            unsigned size = m_socket->readBuffer().writePos() - m_socket->readBuffer().readPos();
            if (size){
                if (!m_client->write_data(m_socket->readBuffer().data(m_socket->readBuffer().readPos()), size)){
                    m_socket->error_state(I18N_NOOP("Write error"));
                    return;
                }
            }
            m_received += size;
            if (m_received >= m_size){
                m_state = Done;
                m_socket->error_state("");
                return;
            }
            m_socket->readBuffer().init(0);
            m_socket->readBuffer().packetStart();
            return;
        }
        EventLog::log_packet(m_socket->readBuffer(), false, HTTPPacket);
        QCString line;
        QCString opt;
        if (!read_line(line)){
            m_socket->readBuffer().init(0);
            m_socket->readBuffer().packetStart();
            return;
        }
        switch (m_state){
        case None:
#ifdef USE_OPENSSL
        case SSLConnect:
#endif
            if (getToken(line, ' ').left(5) != "HTTP/"){
                m_socket->error_state(I18N_NOOP("Bad HTTP answer"));
                return;
            }
            m_code = getToken(line, ' ').toUInt();
            m_state = Header;
            break;
        case Header:
            if (line.isEmpty()){
                m_state = Data;
                break;
            }
            m_hIn += line;
            m_hIn += '\n';
            opt = getToken(line, ':');
            if (opt == "Content-Length"){
                const char *p;
                for (p = line.data(); *p; p++)
                    if (*p != ' ')
                        break;
                m_size = atol(p);
            }
            if ((opt == "Location") && m_bRedirect){
                const char *p;
                for (p = line.data(); *p; p++)
                    if (*p != ' ')
                        break;
                QString proto, host, user, pass, uri, extra;
                unsigned short port;
                QString uri_qstring = p;
                if (!FetchClient::crackUrl(uri_qstring, proto, host, port, user, pass, uri, extra)){
                    FetchClient::crackUrl(m_uri, proto, host, port, user, pass, uri, extra);
                    extra = QString::null;
                    if (*p == '/'){
                        uri = p;
                    }else{
                        int n = uri.findRev('/');
                        uri = uri.left(n + 1);
                        uri += p;
                    }
                }
                m_uri = proto;
                m_uri += "://";
                m_uri += host;
                m_uri += ':';
                m_uri += QString::number(port);
                m_uri += uri;
                if (!extra.isEmpty()){
                    m_uri += '?';
                    m_uri += extra;
                }
                m_state = Redirect;
                m_socket->close();
                m_socket->error_state("");
                return;
            }
            break;
        default:
            break;
        }
    }
}

bool FetchClientPrivate::read_line(QCString &line)
{
    while (m_socket->readBuffer().readPos() < m_socket->readBuffer().writePos()){
        char c;
        m_socket->readBuffer() >> c;
        if (c == '\r')
            continue;
        if (c == '\n')
            return true;
        line += c;
    }
    return false;
}

const char *FetchClient::read_data(char*, unsigned &size)
{
    if (p->m_postData == NULL)
        return 0;
    unsigned tail = p->m_postData->size() - p->m_postData->readPos();
    if (size > tail)
        size = tail;
    const char* res = p->m_postData->data(p->m_postData->readPos());
    p->m_postData->incReadPos(size);
    return res;
}

bool FetchClient::write_data(const char *buf, unsigned size)
{
    p->m_data.pack(buf, size);
    return true;
}

unsigned FetchClient::post_size()
{
    return p->m_postData ? p->m_postData->size() : NO_POSTSIZE;
}

#ifdef WIN32

bool get_connection_state(bool &bState)
{
    DWORD flags;
    bState = InternetGetConnectedState(&flags, 0);
    if (bState)
        return true;
    if (flags & 0x40){
        bState = true;
        return true;
    }
    return true;
}

#else

bool get_connection_state(bool&)
{
    return false;
}

#endif

QString get_user_agent()
{
    return FetchManager::manager->user_agent;
}

#ifndef NO_MOC_INCLUDES
#include "fetch.moc"
#endif


