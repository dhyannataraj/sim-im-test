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
#include "fetch.h"

#include <qthread.h>
#include <qtimer.h>

#ifdef WIN32
#include <windows.h>
#include <wininet.h>

static BOOL (WINAPI *_InternetGetConnectedState)(LPDWORD lpdwFlags, DWORD dwReserved) = NULL;
static HINTERNET (WINAPI *_InternetOpen)(LPCSTR lpszAgent, DWORD dwAccessType,
        LPCSTR lpszProxy, LPCSTR lpszProxyBypass, DWORD dwFlags) = NULL;
static BOOL (WINAPI *_InternetCloseHandle)(HINTERNET hInternet) = NULL;
static BOOL (WINAPI *_InternetCrackUrl)(LPCSTR lpszUrl, DWORD dwUrlLength, DWORD dwFlags,
                                        LPURL_COMPONENTSA lpUrlComponents);
static HINTERNET (WINAPI *_InternetConnect)(HINTERNET hInternet, LPCSTR lpszServerName,
        INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword,
        DWORD dwService, DWORD dwFlags, DWORD dwContext);
static HINTERNET (WINAPI *_HttpOpenRequest)(HINTERNET hConnect, LPCSTR lpszVerb,
        LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer,
        LPCSTR *lplpszAcceptTypes, DWORD dwFlags, DWORD dwContext);
static BOOL (WINAPI *_HttpSendRequest)(HINTERNET hRequest, LPCSTR lpszHeaders,
                                       DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);
static BOOL (WINAPI *_HttpQueryInfo)(HINTERNET hRequest, DWORD dwInfoLevel,
                                     LPVOID lpBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex);
static BOOL (WINAPI *_InternetReadFile)(HINTERNET hFile, LPVOID lpBuffer,
                                        DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead);
static BOOL (WINAPI *_InternetQueryOption)(HINTERNET hInternet, DWORD dwOption,
        LPVOID lpBuffer, LPDWORD lpdwBufferLength);

static HINTERNET hInet = NULL;

class FetchThread : public QThread
{
public:
    FetchThread(FetchClient *client);
protected:
    void run();
    FetchClient *m_client;
};

FetchThread::FetchThread(FetchClient *client)
{
    m_client = client;
}

void FetchThread::run()
{
    string headers;
    DWORD flags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_AUTH;
    if (!m_client->m_bRedirect)
        flags |= INTERNET_FLAG_NO_AUTO_REDIRECT;
    for (HEADERS_MAP::iterator it = m_client->m_hOut.begin(); it != m_client->m_hOut.end(); ++it){
        if (!headers.empty())
            headers += "\r\n";
        string h = (*it).first.c_str();
        headers += h;
        headers += ": ";
        headers += (*it).second.c_str();
    }
    log(L_DEBUG, "URL: %s", m_client->m_uri.c_str());
    URL_COMPONENTSA url;
    memset(&url, 0, sizeof(url));
    url.dwStructSize = sizeof(url);
    char host[256];
    url.lpszHostName      = host;
    url.dwHostNameLength  = sizeof(host);
    char user[256];
    url.lpszUserName      = user;
    url.dwUserNameLength  = sizeof(user);
    char pass[256];
    url.lpszPassword	  = pass;
    url.dwPasswordLength  = sizeof(pass);
    char urlPath[1024];
    url.lpszUrlPath		  = urlPath;
    url.dwUrlPathLength   = sizeof(urlPath);
    char extra[1024];
    url.lpszExtraInfo	  = extra;
    url.dwExtraInfoLength = sizeof(extra);
    if (!_InternetCrackUrl(m_client->m_uri.c_str(), 0, ICU_DECODE, &url)){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "InternetCrackUrl";
        FetchManager::manager->remove(m_client);
        return;
    }
    if (url.nScheme == INTERNET_SCHEME_HTTPS){
        flags |= INTERNET_FLAG_SECURE;
    }else if (url.nScheme != INTERNET_SCHEME_HTTP){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "Unsupported scheme";
        FetchManager::manager->remove(m_client);
        return;
    }
    HINTERNET hCon = _InternetConnect(hInet, url.lpszHostName, url.nPort,
                                      url.lpszUserName, url.lpszPassword, INTERNET_SERVICE_HTTP, 0, 0);
    if (hCon == NULL){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "InternetConnect error";
        FetchManager::manager->remove(m_client);
        return;
    }
    const char *verb = "GET";
    char *post_data = NULL;
    DWORD post_size = 0;
    if (m_client->m_post){
        verb = "POST";
        post_data = m_client->m_post->data();
        post_size = m_client->m_post->size();
    }
    string uri = url.lpszUrlPath;
    if (url.lpszExtraInfo)
        uri += url.lpszExtraInfo;
    HINTERNET hReq = _HttpOpenRequest(hCon, verb, uri.c_str(), NULL, NULL, NULL,
                                      flags, 0);
    if (hReq == NULL){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "HttpOpenRequest error";
        _InternetCloseHandle(hCon);
        FetchManager::manager->remove(m_client);
        return;
    }
    if (!_HttpSendRequest(hReq, headers.c_str(), headers.length(), post_data, post_size)){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "HttpSendRequest";
        _InternetCloseHandle(hReq);
        _InternetCloseHandle(hCon);
        FetchManager::manager->remove(m_client);
        return;
    }
    DWORD size = 0;
    DWORD err  = 0;
    _HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &size, 0);
    err = GetLastError();
    if (err != ERROR_INSUFFICIENT_BUFFER ){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "HttpQueryInfo";
        _InternetCloseHandle(hReq);
        _InternetCloseHandle(hCon);
        FetchManager::manager->remove(m_client);
        return;
    }
    Buffer in_headers;
    in_headers.init(size);
    if (!_HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS_CRLF, in_headers.data(), &size, 0)){
        m_client->m_errCode = GetLastError();
        m_client->m_err     = "HttpQueryInfo";
        _InternetCloseHandle(hReq);
        _InternetCloseHandle(hCon);
        FetchManager::manager->remove(m_client);
        return;
    }
    in_headers.setWritePos(size);
    string line;
    bool bFirst = true;
    for (; in_headers.readPos() < in_headers.writePos(); ){
        if (!in_headers.scan("\r\n", line)){
            line = "";
            unsigned size = in_headers.writePos() - in_headers.readPos();
            line.append(size, '\x00');
            in_headers.unpack((char*)line.c_str(), size);
        }
        if (bFirst){
            bFirst = false;
            getToken(line, ' ');
            m_client->m_code = atol(line.c_str());
            continue;
        }
        m_client->m_hIn += line;
        m_client->m_hIn += '\x00';
    }
    if (bFirst){
        m_client->m_errCode = 0;
        m_client->m_err     = "Bas answer";
        _InternetCloseHandle(hReq);
        _InternetCloseHandle(hCon);
        FetchManager::manager->remove(m_client);
        return;
    }
    for (;;){
        DWORD readn;
        char buff[4096];
        if (!_InternetReadFile(hReq, buff, sizeof(buff), &readn)){
            m_client->m_errCode = GetLastError();
            m_client->m_err     = "InternetReadFile";
            _InternetCloseHandle(hReq);
            _InternetCloseHandle(hCon);
            FetchManager::manager->remove(m_client);
            return;
        }
        if (readn == 0)
            break;
        m_client->m_res.pack(buff, readn);
    }
    _InternetCloseHandle(hReq);
    _InternetCloseHandle(hCon);
    FetchManager::manager->remove(m_client);
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
    X509_set_default_verify_paths(pCTX->cert);
#else
    SSLeay_add_ssl_algorithms();
    mpCTX = SSL_CTX_new(SSLv23_client_method());
    SSL_CTX_set_options(pCTX, SSL_OP_ALL);
    SSL_CTX_set_default_verify_paths(pCTX);
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
    mpSSL = SSL_new(pCTX);
    if(!mpSSL)
        return false;
#if SSLEAY_VERSION_NUMBER >= 0x0900
    pSSL->options|=SSL_OP_NO_TLSv1;
#endif
    return true;
}

#endif

FetchManager *FetchManager::manager = NULL;

const unsigned HTTPPacket   = 0x100;
const unsigned UNKNOWN_SIZE = (unsigned)(-1);

static char _HTTP[] = "HTTP";

FetchManager::FetchManager()
{
#ifdef WIN32
    HINSTANCE hLib = LoadLibraryA("wininet.dll");
    if (hLib != NULL){
        (DWORD&)_InternetGetConnectedState = (DWORD)GetProcAddress(hLib, "InternetGetConnectedState");
        (DWORD&)_InternetOpen = (DWORD)GetProcAddress(hLib, "InternetOpenA");
        (DWORD&)_InternetCloseHandle = (DWORD)GetProcAddress(hLib, "InternetCloseHandle");
        (DWORD&)_InternetCrackUrl = (DWORD)GetProcAddress(hLib, "InternetCrackUrlA");
        (DWORD&)_InternetConnect = (DWORD)GetProcAddress(hLib, "InternetConnectA");
        (DWORD&)_HttpOpenRequest = (DWORD)GetProcAddress(hLib, "HttpOpenRequestA");
        (DWORD&)_HttpSendRequest = (DWORD)GetProcAddress(hLib, "HttpSendRequestA");
        (DWORD&)_HttpQueryInfo = (DWORD)GetProcAddress(hLib, "HttpQueryInfoA");
        (DWORD&)_InternetReadFile = (DWORD)GetProcAddress(hLib, "InternetReadFile");
        (DWORD&)_InternetQueryOption = (DWORD)GetProcAddress(hLib, "InternetQueryOptionA");
    }
    if (_InternetOpen){
        hInet = _InternetOpen(PACKAGE "/" VERSION, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hInet == NULL)
            log(L_WARN, "Internet open error %u", GetLastError());
        return;
    }
#endif
    getContacts()->addPacketType(HTTPPacket, _HTTP, true);
    m_id    = 0;
}

FetchManager::~FetchManager()
{
#ifdef WIN32
    if (hInet){
        _InternetCloseHandle(hInet);
        return;
    }
#endif
    for (list<FetchClient*>::iterator it = m_clients.begin(); it != m_clients.end(); it = m_clients.begin())
        delete *it;
    getContacts()->removePacketType(HTTPPacket);
}

void *FetchManager::processEvent(Event*)
{
    return NULL;
}

void FetchManager::remove(FetchClient *client)
{
    m_remove.push_back(client);
    QTimer::singleShot(0, this, SLOT(remove()));
}

void FetchManager::remove()
{
    for (list<FetchClient*>::iterator it = m_remove.begin(); it != m_remove.end(); ++it)
        delete *it;
    m_remove.clear();
}

FetchClient::FetchClient(const char *url_str, Buffer *postData, const char *headers, bool bRedirect)
{
    m_id = ++FetchManager::manager->m_id;
    FetchManager::manager->m_clients.push_back(this);
    m_socket = NULL;
    m_bDone  = false;
    m_post   = postData;
    m_state  = None;
    m_code   = 0;
    m_size   = UNKNOWN_SIZE;
    m_bRedirect = bRedirect;
    m_uri	= url_str;
#ifdef WIN32
    m_thread	= NULL;
#endif
    if (headers){
        for (const char *h = headers; *h; h += strlen(h) + 1){
            string header = h;
            string key = getToken(header, ':');
            unsigned n;
            for (n = 0; n < header.length(); n++)
                if (header[n] != ' ')
                    break;
            header = header.substr(n);
            addHeader(key.c_str(), header.c_str());
        }
    }
#ifdef WIN32
    m_errCode = 0;
    if (hInet){
        m_thread = new FetchThread(this);
        m_thread->start();
        return;
    }
#endif
    fetch();
}

void FetchClient::fetch()
{
    m_socket = new ClientSocket(this);
#ifdef USE_OPENSSL
    m_bHTTPS = false;
#endif
    string proto;
    string host;
    string user;
    string pass;
    string uri;
    string extra;
    unsigned short port;
    if (!crackUrl(m_uri.c_str(), proto, host, port, user, pass, uri, extra)){
        m_socket->error_state("Bad URL");
        return;
    }
    if (proto != "http"){
#ifdef USE_OPENSSL
        if (proto == "https"){
            m_bHTTPS = true;
        }else{
#endif
            log(L_WARN, "Unsupported protocol %s", m_uri.c_str());
            return;
#ifdef USE_OPENSSL
        }
#endif
    }
    log(L_DEBUG, "Start connect %s:%u", host.c_str(), port);
    m_socket->connect(host.c_str(), port, (TCPClient*)(-1));
}

FetchClient::~FetchClient()
{
    if (m_id){
        m_hIn += '\x00';
        fetchData data;
        data.req_id  = m_id;
        data.result  = m_code;
        data.data    = &m_res;
        data.headers = m_hIn.c_str();
        Event e(EventFetchDone, &data);
        e.process();
    }
    if (m_id){
        list<FetchClient*>::iterator it;
        for (it = FetchManager::manager->m_clients.begin(); it != FetchManager::manager->m_clients.end(); ++it)
            if ((*it) == this)
                break;
        if (it != FetchManager::manager->m_clients.end())
            FetchManager::manager->m_clients.erase(it);
    }
#ifdef WIN32
    if (!m_err.empty())
        log(L_DEBUG, "Fetch error %s: %u", m_err.c_str(), m_errCode);
    if (m_thread){
        m_thread->wait();
        delete m_thread;
    }
#endif
    if (m_socket)
        delete m_socket;
    if (m_post)
        delete m_post;
}

bool FetchClient::crackUrl(const char *_url, string &protocol, string &host, unsigned short &port, string &user, string &pass, string &uri, string &extra)
{
    port = 80;
    string url(_url);
    protocol = getToken(url, ':', false);
    if (url.substr(0, 2) != "//")
        return false;
    url = url.substr(2);
    host = getToken(url, '/', false);
    extra = "/";
    extra += url;
    uri = getToken(extra, '?', false);
    user = getToken(host, '@', false);
    if (host.empty()){
        host = user;
        user = "";
    }else{
        pass = user;
        user = getToken(pass, ':', false);
    }
    if (protocol == "https")
        port = 443;
    string h = getToken(host, ':');
    if (!host.empty()){
        port = (unsigned short)atol(host.c_str());
        if (port == NULL)
            return false;
    }
    host = h;
    return true;
}

void FetchClient::addHeader(const char *key, const char *value)
{
    HEADERS_MAP::iterator it = m_hOut.find(key);
    if (it == m_hOut.end()){
        m_hOut.insert(HEADERS_MAP::value_type(key, value));
    }else{
        (*it).second = value;
    }
}

bool FetchClient::findHeader(const char *key)
{
    HEADERS_MAP::iterator it = m_hOut.find(key);
    return (it != m_hOut.end());
}

static string tobase64(const char *s)
{
    Buffer from;
    Buffer to;
    from << s;
    to.toBase64(from);
    string res;
    res.append(to.data(), to.size());
    return res;
}

string basic_auth(const char *user, const char *pass)
{
    string auth = user;
    auth += ":";
    if (pass)
        auth += pass;
    return tobase64(auth.c_str());
}

bool FetchClient::error_state(const char *err, unsigned)
{
    if (m_state == Redirect){
        delete m_socket;
        m_socket = NULL;
        m_code = 0;
        m_hIn  = "";
        m_res.init(0);
        if (m_post){
            delete m_post;
            m_post = NULL;
        }
        m_state = None;
        fetch();
        return false;
    }
    if ((m_state == Data) && (m_size == UNKNOWN_SIZE))
        return true;
    if (*err){
        m_code = 0;
        log(L_DEBUG, "HTTP: %s", err);
    }
    return true;
}

void FetchClient::connect_ready()
{
#ifdef USE_OPENSSL
    if ((m_state == None) & m_bHTTPS){
        m_socket->setRaw(true);
        m_socket->readBuffer.init(0);
        HTTPSClient *https = new HTTPSClient(m_socket->socket());
        if (!https->init()){
            m_socket->error_state("Can't initialize HTTPS");
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
    m_socket->writeBuffer.packetStart();

    string proto;
    string host;
    string user;
    string pass;
    string uri;
    string extra;
    unsigned short port;
    crackUrl(m_uri.c_str(), proto, host, port, user, pass, uri, extra);
    if (!extra.empty()){
        uri += "?";
        uri += extra;
    }
    m_socket->writeBuffer
    << (m_post ? "POST " : "GET ")
    << uri.c_str()
    << " HTTP/1.0\r\n";
    if (!findHeader("Host"))
        m_socket->writeBuffer
        << "Host: "
        << host.c_str()
        << "\r\n";
    if (!findHeader("User-Agent"))
        m_socket->writeBuffer
        << "User-Agent: " PACKAGE "/" VERSION "\r\n";
    if (!findHeader("Authorization") && !user.empty())
        m_socket->writeBuffer
        << "Authorization: basic "
        << basic_auth(user.c_str(), pass.c_str()).c_str()
        << "\r\n";
    if (m_post){
        if (!findHeader("Content-Length"))
            m_socket->writeBuffer
            << "Content-Length: "
            << number(m_post->size()).c_str()
            << "\r\n";
    }
    for (HEADERS_MAP::iterator it = m_hOut.begin(); it != m_hOut.end(); ++it){
        m_socket->writeBuffer
        << (*it).first.c_str()
        << ": "
        << (*it).second.c_str()
        << "\r\n";
    }
    m_socket->writeBuffer
    << "\r\n";
    if (m_post)
        m_socket->writeBuffer.pack(m_post->data(), m_post->size());
    log_packet(m_socket->writeBuffer, true, HTTPPacket);
    m_socket->write();
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
}

void FetchClient::packet_ready()
{
    if (m_socket->readBuffer.readPos() == m_socket->readBuffer.writePos())
        return;
    log_packet(m_socket->readBuffer, false, HTTPPacket);
    for (;;){
        if (m_state == Data){
            unsigned size = m_socket->readBuffer.writePos() - m_socket->readBuffer.readPos();
            if (size)
                m_res.pack(m_socket->readBuffer.data(m_socket->readBuffer.readPos()), size);
            if (m_res.size() >= m_size){
                m_socket->error_state("");
                return;
            }
            m_socket->readBuffer.init(0);
            m_socket->readBuffer.packetStart();
            return;
        }
        string line;
        string opt;
        if (!read_line(line)){
            m_socket->readBuffer.init(0);
            m_socket->readBuffer.packetStart();
            return;
        }
        switch (m_state){
        case None:
#ifdef USE_OPENSSL
        case SSLConnect:
#endif
            if (getToken(line, ' ').substr(0, 5) != "HTTP/"){
                m_socket->error_state("Bad HTTP answer");
                return;
            }
            m_code = atol(getToken(line, ' ').c_str());
            m_state = Header;
            break;
        case Header:
            if (line.empty()){
                m_state = Data;
                break;
            }
            m_hIn += line;
            m_hIn += '\x00';
            opt = getToken(line, ':');
            if (opt == "Content-Length"){
                const char *p;
                for (p = line.c_str(); *p; p++)
                    if (*p != ' ')
                        break;
                m_size = atol(p);
            }
            if ((opt == "Location") && m_bRedirect){
                const char *p;
                for (p = line.c_str(); *p; p++)
                    if (*p != ' ')
                        break;
                string proto;
                string host;
                string user;
                string pass;
                string uri;
                string extra;
                unsigned short port;
                if (!crackUrl(p, proto, host, port, user, pass, uri, extra)){
                    crackUrl(m_uri.c_str(), proto, host, port, user, pass, uri, extra);
                    extra = "";
                    if (*p == '/'){
                        uri = p;
                    }else{
                        int n = uri.find_last_of('/');
                        uri = uri.substr(0, n + 1);
                        uri += p;
                    }
                }
                m_uri = proto;
                m_uri += "://";
                m_uri += host;
                m_uri += ":";
                m_uri += number(port);
                m_uri += uri;
                if (!extra.empty()){
                    m_uri += "?";
                    m_uri += extra;
                }
                m_state = Redirect;
                m_socket->error_state("");
                return;
            }
            break;
        default:
            break;
        }
    }
}

bool FetchClient::read_line(string &s)
{
    while (m_socket->readBuffer.readPos() < m_socket->readBuffer.writePos()){
        char c;
        m_socket->readBuffer >> c;
        if (c == '\r')
            continue;
        if (c == '\n')
            return true;
        s += c;
    }
    return false;
}


unsigned SIM::fetch(const char *url, Buffer *postData, const char *headers, bool bRedirect)
{
#ifdef WIN32
    bool bState;
    if (get_connection_state(bState))
        getSocketFactory()->setActive(bState);
#endif
    FetchClient *client = new FetchClient(url, postData, headers, bRedirect);
    if (client->id())
        return client->id();
    delete client;
    return 0;
}

#ifdef WIN32

bool get_connection_state(bool &bState)
{
    if (_InternetGetConnectedState == NULL)
        return false;
    DWORD flags;
    bState = _InternetGetConnectedState(&flags, 0);
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

#ifndef WIN32
#include "fetch.moc"
#endif
