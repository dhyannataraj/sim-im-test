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
#include <openssl/rand.h>
#endif

#include "fetch.h"

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

bool my_string::operator < (const my_string &a) const
{
    return strcmp(c_str(), a.c_str()) < 0;
}

FetchManager::FetchManager()
{
    getContacts()->addPacketType(HTTPPacket, _HTTP, true);
    m_id    = 0;
}

FetchManager::~FetchManager()
{
    for (list<FetchClient*>::iterator it = m_clients.begin(); it != m_clients.end(); it = m_clients.begin())
        delete *it;
    getContacts()->removePacketType(HTTPPacket);
}

void *FetchManager::processEvent(Event *e)
{
    if (e->type() == EventClientsChanged){
        for (list<FetchClient*>::iterator it = m_clients.begin(); it != m_clients.end();){
            if ((*it)->m_client){
                unsigned i;
                for (i = 0; i < getContacts()->nClients(); i++){
                    if (getContacts()->getClient(i) == (*it)->m_client)
                        break;
                }
                if (i >= getContacts()->nClients()){
                    delete (*it);
                    it = m_clients.begin();
                    continue;
                }
            }
            ++it;
        }
    }
    return NULL;
}

FetchClient::FetchClient(TCPClient *client, const char *url_str, Buffer *postData, const char *headers)
{
    m_id = 0;
    m_socket = NULL;
    m_bDone  = false;
    m_port	 = 80;
    m_post   = postData;
    m_state  = None;
    m_code   = 0;
    m_size   = UNKNOWN_SIZE;
    m_client = client;
#ifdef USE_OPENSSL
    m_bHTTPS = false;
#endif
    string url = url_str;
    string protocol = getToken(url, ':');
    if (protocol != "http"){
#ifdef USE_OPENSSL
        if (protocol == "https"){
            m_bHTTPS = true;
            m_port   = 443;
        }else{
#endif
            log(L_WARN, "Unsupported protocol %s", url_str);
            return;
#ifdef USE_OPENSSL
        }
#endif
    }
    if ((url[0] != '/') && (url[1] != '/')){
        log(L_WARN, "Bad url %s", url_str);
        return;
    }
    url		= url.substr(2);
    m_host	= getToken(url, '/');
    m_uri	= "/";
    m_uri  += url;
    addHeader("Host", m_host.c_str());
    if (postData){
        addHeader("Content-Type", "");
        addHeader("Content-Length", number(postData->size()).c_str());
    }
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
    m_id = ++FetchManager::manager->m_id;
    FetchManager::manager->m_clients.push_back(this);
    m_socket = new ClientSocket(this);
    log(L_DEBUG, "Start connect %s:%u", m_host.c_str(), m_port);
    m_socket->connect(m_host.c_str(), m_port, m_client);
}

FetchClient::~FetchClient()
{
    if (m_id){
        m_hIn += '\x00';
        fetchData data;
        data.req_id  = m_id;
        data.result  = m_code;
        data.data    = NULL;
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
    if (m_socket)
        delete m_socket;
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

bool FetchClient::error_state(const char *err, unsigned)
{
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
    m_socket->writeBuffer
    << (m_post ? "POST " : "GET ")
    << m_uri.c_str()
    << " HTTP/1.0\r\n";
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
            if (getToken(line, ':') == "Content-Length"){
                const char *p;
                for (p = line.c_str(); *p; p++)
                    if (*p != ' ')
                        break;
                m_size = atol(p);
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


unsigned SIM::fetch(TCPClient *cl, const char *url, Buffer *postData, const char *headers)
{
    FetchClient *client = new FetchClient(cl, url, postData, headers);
    if (client->id())
        return client->id();
    delete client;
    return 0;
}
