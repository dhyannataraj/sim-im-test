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
#include "buffer.h"
#include "socket.h"
#include "stl.h"

#include <qthread.h>
#include <qtimer.h>

typedef map<my_string, string> HEADERS_MAP;

class FetchThread;

class FetchClientPrivate : public ClientSocketNotify
{
public:
    FetchClientPrivate(FetchClient*);
    ~FetchClientPrivate();
protected:
    void fetch(const char *url, const char *headers = NULL, Buffer *postData = NULL, bool bRedirect = true);;
    void stop();
    FetchClient *m_client;
    void		fail();
    string		m_uri;
    string		m_hIn;
    HEADERS_MAP	m_hOut;
    unsigned	m_code;
    bool		m_bRedirect;
#ifdef WIN32
    FetchThread	*m_thread;
    string		m_err;
    unsigned	m_errCode;
#endif
    virtual bool error_state(const char *err, unsigned code);
    virtual void connect_ready();
    virtual void packet_ready();
    virtual void write_ready();
    bool read_line(string&);
    ClientSocket *m_socket;
    unsigned	m_postSize;
    unsigned	m_received;
    unsigned	m_id;
    bool		m_bDone;
    bool		crackUrl(const char *url, string &proto, string &host, unsigned short &port, string &user, string &pass, string &uri, string &extra);
    unsigned	m_size;
    void		addHeader(const char *key, const char *value);
    bool		findHeader(const char *key);
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
static BOOL (WINAPI *_HttpSendRequestEx)(HINTERNET hRequest,
        LPINTERNET_BUFFERSA lpBuffersIn, LPINTERNET_BUFFERSA lpBuffersOut,
        DWORD dwFlags, DWORD dwContext);
static BOOL (WINAPI *_HttpQueryInfo)(HINTERNET hRequest, DWORD dwInfoLevel,
                                     LPVOID lpBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex);
static BOOL (WINAPI *_InternetReadFile)(HINTERNET hFile, LPVOID lpBuffer,
                                        DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead);
static BOOL (WINAPI *_InternetWriteFile)(HINTERNET hFile, LPVOID lpBuffer,
        DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead);
static BOOL (WINAPI *_InternetQueryOption)(HINTERNET hInternet, DWORD dwOption,
        LPVOID lpBuffer, LPDWORD lpdwBufferLength);
static BOOL (WINAPI *_HttpEndRequest)(HINTERNET hRequest, LPINTERNET_BUFFERS lpBuffersOut,
                                      DWORD dwFlags, DWORD dwContext);

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
        _InternetCloseHandle(hReq);
        hReq = NULL;
    }
    if (hCon){
        _InternetCloseHandle(hCon);
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
	log(L_DEBUG, "fetch");
    string headers;
    DWORD flags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_AUTH;
    if (!m_client->m_bRedirect)
        flags |= INTERNET_FLAG_NO_AUTO_REDIRECT;
    const char *verb = "GET";
    unsigned postSize = m_client->m_client->post_size();
    if (postSize != NO_POSTSIZE)
        verb = "POST";
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
        error("InternetCrackUrl");
        return;
    }
    if (url.nScheme == INTERNET_SCHEME_HTTPS){
        flags |= INTERNET_FLAG_SECURE;
    }else if (url.nScheme != INTERNET_SCHEME_HTTP){
        error("Unsupported scheme");
        return;
    }
    hCon = _InternetConnect(hInet, url.lpszHostName, url.nPort,
                            url.lpszUserName, url.lpszPassword, INTERNET_SERVICE_HTTP, 0, 0);
    if (hCon == NULL){
        error("InternetConnect");
        return;
    }
    string uri = url.lpszUrlPath;
    if (url.lpszExtraInfo)
        uri += url.lpszExtraInfo;
    hReq = _HttpOpenRequest(hCon, verb, uri.c_str(), NULL, NULL, NULL, flags, 0);
    if (hReq == NULL){
        error("HttpOpenRequest error");
        return;
    }
    if (postSize != NO_POSTSIZE){
        INTERNET_BUFFERSA BufferIn;
        memset(&BufferIn, 0, sizeof(BufferIn));
        BufferIn.dwStructSize    = sizeof(INTERNET_BUFFERSA);
        BufferIn.lpcszHeader     = headers.c_str();
        BufferIn.dwHeadersLength = headers.length();
        BufferIn.dwHeadersTotal  = headers.length();
        BufferIn.dwBufferTotal   = (postSize != NO_POSTSIZE) ? postSize : 0;
        if (!_HttpSendRequestEx(hReq, &BufferIn, NULL, HSR_INITIATE | HSR_SYNC, 0)){
            error("HttpSendRequestEx");
            return;
        }
        while (postSize){
            char buff[4096];
            unsigned tail = postSize;
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
            if (!_InternetWriteFile(hReq, (void*)data, tail, &res)){
                error("InternetWriteFile");
                return;
            }
            postSize -= tail;
        }
        if (m_bClose)
            return;
        if(!_HttpEndRequest(hReq, NULL, 0, 0)){
            error("HttpEndRequest");
            return;
        }
    }else{
        if (!_HttpSendRequest(hReq, headers.c_str(), headers.length(), NULL, 0)){
            error("HttpSendRequest");
            return;
        }
    }
    DWORD size = 0;
    DWORD err  = 0;
    _HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &size, 0);
    err = GetLastError();
    if (err != ERROR_INSUFFICIENT_BUFFER ){
        error("HttpQueryInfo");
        return;
    }
    Buffer in_headers;
    in_headers.init(size);
    if (!_HttpQueryInfo(hReq, HTTP_QUERY_RAW_HEADERS_CRLF, in_headers.data(), &size, 0)){
        error("HttpQueryInfo");
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
        error("Bas answer");
        return;
    }
    for (;;){
        DWORD readn;
        char buff[4096];
        if (!_InternetReadFile(hReq, buff, sizeof(buff), &readn)){
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
    m_client->m_state = FetchClientPrivate::Done;
    close();
    FetchManager::manager->done(m_client->m_client);
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

static list<FetchClientPrivate*> *m_done = NULL;
static string *user_agent = NULL;

FetchManager::FetchManager()
{
    m_done = new list<FetchClientPrivate*>;
    user_agent = new string;
    *user_agent = PACKAGE "/" VERSION;
    *user_agent += " (";
#ifdef WIN32
    *user_agent += "Windows ";
    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);
    switch (osvi.dwPlatformId){
    case VER_PLATFORM_WIN32_NT:
        if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 2)){
            *user_agent += "2003";
        }else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 1)){
            *user_agent += "XP";
        }else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0 )){
            *user_agent += "2000";
        }else{
            *user_agent += number(osvi.dwMajorVersion);
            *user_agent += ".";
            *user_agent += number(osvi.dwMinorVersion);
        }
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        if (osvi.dwMajorVersion == 4){
            if (osvi.dwMinorVersion == 0){
                *user_agent += "95";
                if ((osvi.szCSDVersion[1] == 'C') || (osvi.szCSDVersion[1] == 'B'))
                    *user_agent += " OSR2";
            }else if (osvi.dwMinorVersion == 10){
                *user_agent += "98";
                if ( osvi.szCSDVersion[1] == 'A' )
                    *user_agent += " SE";
            }else if (osvi.dwMinorVersion == 90){
                *user_agent += "Millennium";
            }
        }
        break;
    case VER_PLATFORM_WIN32s:
        *user_agent += "32s";
        break;
    }
#else
*user_agent += UNAME;
#endif
    *user_agent += " )";
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
        (DWORD&)_HttpSendRequestEx = (DWORD)GetProcAddress(hLib, "HttpSendRequestExA");
        (DWORD&)_HttpQueryInfo = (DWORD)GetProcAddress(hLib, "HttpQueryInfoA");
        (DWORD&)_HttpEndRequest = (DWORD)GetProcAddress(hLib, "HttpEndRequestA");
        (DWORD&)_InternetReadFile = (DWORD)GetProcAddress(hLib, "InternetReadFile");
        (DWORD&)_InternetWriteFile = (DWORD)GetProcAddress(hLib, "InternetWriteFile");
        (DWORD&)_InternetQueryOption = (DWORD)GetProcAddress(hLib, "InternetQueryOptionA");
    }
    if (_InternetOpen && _HttpSendRequestEx){
        hInet = _InternetOpen(user_agent->c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hInet == NULL)
            log(L_WARN, "Internet open error %u", GetLastError());
        return;
    }
#endif
    getContacts()->addPacketType(HTTPPacket, _HTTP, true);
}

FetchManager::~FetchManager()
{
#ifdef WIN32
    if (hInet)
        _InternetCloseHandle(hInet);
#endif
    getContacts()->removePacketType(HTTPPacket);
    delete m_done;
    delete user_agent;
}

void FetchManager::done(FetchClient *client)
{
    m_done->push_back(client->p);
    QTimer::singleShot(0, this, SLOT(timeout()));
}

void FetchManager::timeout()
{
    list<FetchClientPrivate*> done = *m_done;
    m_done->clear();
    for (list<FetchClientPrivate*>::iterator it = done.begin(); it != done.end(); ++it){
        if ((*it)->error_state("", 0))
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

void FetchClient::fetch(const char *url, const char *headers, Buffer *postData, bool bRedirect)
{
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
#ifdef WIN32
    m_thread	= NULL;
#endif
}

void FetchClientPrivate::fetch(const char *url, const char *headers, Buffer *postData, bool bRedirect)
{
    stop();
    m_bDone = false;
    m_data.init(0);
    m_postData  = postData;
    m_bRedirect = bRedirect;
    m_uri	    = url;
    m_postSize  = 0;
#ifdef WIN32
    m_thread	= NULL;
#endif
    if (headers){
        string head = headers;
        while (!head.empty()){
            string header = getToken(head, '\n');
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
    m_received = 0;
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
    m_hIn = "";
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
}

bool FetchClientPrivate::crackUrl(const char *_url, string &protocol, string &host, unsigned short &port, string &user, string &pass, string &uri, string &extra)
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
        if (port == 0)
            return false;
    }
    host = h;
    return true;
}

void FetchClientPrivate::addHeader(const char *key, const char *value)
{
    HEADERS_MAP::iterator it = m_hOut.find(key);
    if (it == m_hOut.end()){
        m_hOut.insert(HEADERS_MAP::value_type(key, value));
    }else{
        (*it).second = value;
    }
}

bool FetchClientPrivate::findHeader(const char *key)
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

bool FetchClientPrivate::error_state(const char *err, unsigned)
{
    if (m_state == Redirect){
        delete m_socket;
        m_socket = NULL;
        m_code = 0;
        m_hIn  = "";
        m_state = None;
        fetch(m_uri.c_str());
        return false;
    }
    if ((m_state != Done) && ((m_state != Data) || (m_size != UNKNOWN_SIZE))){
        m_code = 0;
        log(L_DEBUG, "HTTP: %s", err);
    }
    m_bDone = true;
    m_state = None;
    return m_client->done(m_code, m_data, m_hIn.c_str());
}

void FetchClientPrivate::connect_ready()
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
    unsigned postSize = m_client->post_size();
    m_socket->writeBuffer
    << ((postSize != NO_POSTSIZE) ? "POST " : "GET ")
    << uri.c_str()
    << " HTTP/1.0\r\n";
    if (!findHeader("Host"))
        m_socket->writeBuffer
        << "Host: "
        << host.c_str()
        << "\r\n";
    if (!findHeader("User-Agent"))
        m_socket->writeBuffer
        << "User-Agent: " << user_agent->c_str() << "\r\n";
    if (!findHeader("Authorization") && !user.empty())
        m_socket->writeBuffer
        << "Authorization: basic "
        << basic_auth(user.c_str(), pass.c_str()).c_str()
        << "\r\n";
    if (postSize != NO_POSTSIZE){
        if (!findHeader("Content-Length"))
            m_socket->writeBuffer
            << "Content-Length: "
            << number(postSize).c_str()
            << "\r\n";
        m_postSize = postSize;
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
    log_packet(m_socket->writeBuffer, true, HTTPPacket);
    m_socket->write();
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
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
        m_socket->error_state("Read error");
        return;
    }
    m_postSize -= tail;
    m_socket->writeBuffer.pack(data, tail);
    m_socket->write();
}

void FetchClientPrivate::packet_ready()
{
    if (m_socket->readBuffer.readPos() == m_socket->readBuffer.writePos())
        return;
    log_packet(m_socket->readBuffer, false, HTTPPacket);
    for (;;){
        if (m_state == Data){
            unsigned size = m_socket->readBuffer.writePos() - m_socket->readBuffer.readPos();
            if (size){
                if (!m_client->write_data(m_socket->readBuffer.data(m_socket->readBuffer.readPos()), size)){
                    m_socket->error_state("Write error");
                    return;
                }
            }
            m_received += size;
            if (m_received >= m_size){
                m_state = Done;
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

bool FetchClientPrivate::read_line(string &s)
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

const char *FetchClient::read_data(const char*, unsigned &size)
{
    if (p->m_postData == NULL)
        return 0;
    unsigned tail = p->m_postData->size() - p->m_postData->readPos();
    if (size > tail)
        size = tail;
    return p->m_postData->data(p->m_postData->readPos());
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


