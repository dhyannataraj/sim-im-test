/***************************************************************************
                          socket.cpp  -  description
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

#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif

#include "socket.h"
#include "log.h"

#ifdef WIN32
#include <winsock.h>
#include <fcntl.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <list>

#ifdef WIN32
#define sockclose closesocket
#define socklen_t int
#else
#define sockclose ::close
#endif

#ifndef WIN32

class SignalHandler
{
public:
    SignalHandler(int signal);
    ~SignalHandler();
protected:
    int m_num;
    struct sigaction old_act;
    static void Handler(int);
};

void SignalHandler::Handler(int)
{
}

SignalHandler::SignalHandler(int num) : m_num(num)
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = Handler;
    act.sa_flags = 0;
    if (sigaction(num, &act, &old_act) == -1){
        log(L_WARN, "sigaction failed: %s", strerror(errno));
    }
}

SignalHandler::~SignalHandler()
{
    sigaction(m_num, &old_act, NULL);
}

SignalHandler s(SIGPIPE);

#endif

static Sockets *pSockets = NULL;

unsigned short Sockets::proxyType = PROXY_NONE;
string Sockets::proxyHost;
unsigned short Sockets::proxyPort = 0;
bool Sockets::proxyAuth = false;
string Sockets::proxyUser;
string Sockets::proxyPasswd;

Sockets::Sockets()
{
    pSockets = this;
}

Sockets::~Sockets()
{
    pSockets = NULL;
}

void Sockets::addSocket(Socket *s)
{
    socket_list.push_back(s);
}

void Sockets::delSocket(Socket *s)
{
    socket_list.remove(s);
}

void Sockets::createSocket(Socket*)
{
}

void Sockets::closeSocket(Socket*)
{
}

void Sockets::setHaveData(Socket*)
{
}

void Sockets::process(unsigned timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    for (;;){
        int max_fd = 0;
        fd_set rf;
        fd_set wf;
        fd_set ef;
        FD_ZERO(&rf);
        FD_ZERO(&wf);
        FD_ZERO(&ef);
        list<Socket*>::iterator it;
        for (;;){
            bool ok = true;
            for (it = socket_list.begin(); it != socket_list.end(); it++){
                if ((*it)->m_delete){
                    delete *it;
                    ok = false;
                    break;
                }
            }
            if (ok) break;
        }
        for (it = socket_list.begin(); it != socket_list.end(); it++){
            if ((*it)->m_resolving == Socket::Resolving) continue;
            if ((*it)->m_resolving == Socket::Done)
                (*it)->read_ready();
            int fd = (*it)->m_fd;
            if (fd == -1) continue;
            if (fd > max_fd) max_fd = fd;
            FD_SET(fd, &rf);
            FD_SET(fd, &ef);
            if ((*it)->have_data() || ((*it)->m_connecting == Socket::Connecting))
                FD_SET(fd, &wf);
        }
        int res = select(max_fd + 1, &rf, &wf, &ef, &tv);
        if (res <= 0){
            for (it = socket_list.begin(); it != socket_list.end(); it++){
                (*it)->idle();
                pSockets->setHaveData(*it);
            }
            return;
        }
        for (it = socket_list.begin(); it != socket_list.end(); it++){
            int fd = (*it)->m_fd;
            if ((fd == -1) || ((*it)->m_resolving == Socket::Resolving)){
                (*it)->idle();
                continue;
            }
            if (FD_ISSET(fd, &ef)){
                (*it)->error_state();
                continue;
            }
            if (FD_ISSET(fd, &rf)){
                if ((*it)->m_connecting != Socket::Connecting){
                    (*it)->read_ready();
                    continue;
                }
            }
            if (FD_ISSET(fd, &wf)){
                if ((*it)->m_connecting == Socket::Connecting){
                    (*it)->proxy_connect_ready();
                }else{
                    (*it)->write_ready();
                }
            }
        }
        for (;;){
            bool ok = true;
            for (it = socket_list.begin(); it != socket_list.end(); it++){
                if ((*it)->m_delete){
                    delete *it;
                    ok = false;
                    break;
                }
            }
            if (ok) break;
        }
    }
}


Socket::Socket(int fd, const char *host, unsigned short port)
        : m_fd(fd), m_szHost(NULL), m_nPort(port), m_err(ErrorNone), m_delete(false)
{
    intData = NULL;
    m_szResolveHost = NULL;
    m_resolving = None;
    m_connecting = NoConnect;
    if (host) m_szHost = strdup(host);
    if (pSockets){
        pSockets->addSocket(this);
        if (fd >= 0){
            pSockets->createSocket(this);
            m_connecting = Connected;
        }
    }
}

Socket::~Socket()
{
    if (pSockets)
        pSockets->delSocket(this);
    close();
    if (m_szHost) free(m_szHost);
    if (m_szResolveHost) free(m_szResolveHost);
}

static char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "0123456789+/";

static string tobase64(const char *text)
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

void Socket::proxy_connect_ready()
{
}

void ClientSocket::proxy_connect_ready()
{
    unsigned long offs = connectBuffer.writePos();
    switch (Sockets::proxyType){
    case PROXY_NONE:
        m_connecting = Connected;
        connect_ready();
        return;
    case PROXY_SOCKS4:
        connectBuffer
        << (char)4
        << (char)1
        << Sockets::proxyPort
        << m_remoteAddr
        << (char)0;
        m_connecting = Socks4_Wait;
        dumpPacket(connectBuffer, offs, "SOCKS4 send");
        return;
    case PROXY_SOCKS5:
        connectBuffer << (char)5;
        if (Sockets::proxyAuth) {
            connectBuffer
            << (char)0x02
            << (char)0x00
            << (char)0x02;
        } else {
            connectBuffer
            << (char)0x01
            << (char)0x00;
        }
        m_connecting = Socks5_Wait;
        dumpPacket(connectBuffer, offs, "SOCKS5 send");
        return;
    case PROXY_HTTP:{
            char cmd[1024];
            snprintf(cmd, sizeof(cmd),
                     "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n",
                     m_szHost, m_nPort,
                     m_szHost, m_nPort);
            connectBuffer.pack(cmd, strlen(cmd));
            if (Sockets::proxyAuth){
                strncpy(cmd, Sockets::proxyUser.c_str(), sizeof(cmd));
                strncat(cmd, Sockets::proxyPasswd.c_str(), sizeof(cmd));
                string b = tobase64(cmd);
                char _AUTH[] = "Proxy-Authorization: Basic ";
                connectBuffer.pack(_AUTH, strlen(_AUTH));
                connectBuffer.pack(b.c_str(), b.length());
                connectBuffer.pack("\r\n");
            }
            connectBuffer.pack("\r\n", 2);
            m_connecting = Http_Wait;
            dumpPacket(connectBuffer, offs, "HTTP send");
            return;
        }
    default:
        log(L_ERROR, "Unknown proxy type");
        error(ErrorProtocol);
    }
}

void Socket::connect_ready()
{
}

void Socket::close()
{
    if (m_fd != -1){
        pSockets->closeSocket(this);
        sockclose(m_fd);
        m_fd = -1;
    }
    if (m_szHost) {
        free(m_szHost);
        m_szHost = NULL;
    }
    m_nPort = 0;
    m_resolving = None;
}

const char *Socket::errorText()
{
    log(L_DEBUG, "Error %u", m_err);
    switch (m_err){
    case ErrorNone:
        return "No errors";
    case ErrorConnected:
        return "Socket already conected";
    case ErrorCreate:
        return "Create socket error";
    case ErrorBind:
        return "Bind socket error";
    case ErrorResolve:
        return "Resolve host error";
    case ErrorConnect:
        return "Connect error";
    case ErrorAccept:
        return "Accept error";
    case ErrorListen:
        return "Listen error";
    case ErrorDisconnect:
        return "Client disconnect";
    case ErrorRead:
        return "Read error";
    case ErrorWrite:
        return "Write error";
    case ErrorProtocol:
        return "Protocol error";
    }
    return "Unknown error";
}

void Socket::error(SocketError err)
{
    m_err = err;
    error_state();
}

void Socket::error_state()
{
    log(L_ERROR, "[%s:%u] %s%s%s", host() ? host() : "*", port(), errorText(),
        errno ? ": " : "", errno ? strerror(errno) : "");
}

ServerSocket::ServerSocket()
        : Socket(-1, NULL, 0)
{
}

bool ServerSocket::listen(int minPort, int maxPort, const char *host)
{
    m_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_fd == -1)
    {
        error(ErrorCreate);
        return false;
    }
    if (minPort == 0) minPort = 1024;
    if (maxPort == 0) maxPort = minPort;

#ifndef WIN32
    int fl = fcntl(m_fd, F_GETFL, 0);
    if (fl != -1)
        fcntl(m_fd, F_SETFL, fl | O_NONBLOCK);
#endif

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    int port;
    for (port = minPort; port <= maxPort; port++){
        struct sockaddr_in addr;
        memset(&addr.sin_zero, 0, 8);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (host && ((addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE))
        {
            // try and resolve hostname
            struct hostent *he = gethostbyname(host);
            if (he == NULL){
                error(ErrorResolve);
                close();
                return false;
            }
            addr.sin_addr.s_addr = *((unsigned long *)(he->h_addr));
        }
        if (bind(m_fd, (struct sockaddr *)&addr, sizeof(sockaddr_in)) >= 0)
            break;
    }
    if (port > maxPort){
        error(ErrorBind);
        close();
        return false;
    }
    char *localHost;
    if (getLocalAddr(localHost, m_nPort))
        setHost(localHost);
    if (::listen(m_fd, 256) == -1)
    {
        error(ErrorListen);
        close();
        return false;
    }
    log(L_DEBUG, "Listen on port %u", m_nPort);
    pSockets->createSocket(this);
    return true;
}

void Socket::setHost(const char *host)
{
    if (m_szHost) free(m_szHost);
    m_szHost = strdup(host);
}

void Socket::remove()
{
    m_delete = true;
}

static string ip2s(unsigned long ip)
{
    if (ip == 0) return "";
    in_addr addr;
    addr.s_addr = ip;
    return string(inet_ntoa(addr));
}

bool Socket::getLocalAddr(char *&host, unsigned short &port, unsigned long remote_ip)
{
    struct sockaddr_in addr;
    socklen_t sizeofAddr = sizeof(addr);
    if (getsockname(m_fd, (struct sockaddr *)&addr, &sizeofAddr) < 0)
        return false;
    host = inet_ntoa(addr.sin_addr);
    port = htons(addr.sin_port);
    string s_remote_ip = ip2s(remote_ip);
    if (remote_ip || (addr.sin_addr.s_addr == 0x7F000001) || (addr.sin_addr.s_addr == 0)){
        char hostname[128] ;
        if(gethostname(hostname, sizeof(hostname)) >= 0){
            struct hostent *phe = gethostbyname (hostname);
            if (phe){
                int n_match = 0;
                unsigned long res = 0;
                for (char **p_ip = phe->h_addr_list; *p_ip; p_ip++){
                    unsigned long ip = *((unsigned long*)(*p_ip));
                    string s_ip = ip2s(ip);
                    log(L_DEBUG, "Match: %s %s", s_ip.c_str(), s_remote_ip.c_str());
                    unsigned n = 1 << 31;
                    int n_ip_match;
                    for (n_ip_match = 0; n_ip_match < 32; n_ip_match++, n = (n >> 1))
                        if ((ip & n) != (remote_ip & n)) break;
                    if (n_ip_match >= n_match){
                        n_match = n_ip_match;
                        res = ip;
                    }
                }
                in_addr addr;
                addr.s_addr = res;
                host = inet_ntoa(addr);
            }
        }
    }
    log(L_DEBUG, "Local IP: %s %u (%s)", host, port, s_remote_ip.c_str());
    return true;
}

void ClientSocket::setOpt()
{
#ifndef WIN32
    int fl = fcntl(m_fd, F_GETFL, 0);
    if (fl != -1)
        fcntl(m_fd, F_SETFL, fl | O_NONBLOCK);
#endif
    int one = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
    struct linger linger;
    linger.l_onoff  = 1;
    linger.l_linger = 60;
    setsockopt(m_fd, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));
}

void ServerSocket::read_ready()
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int fd = ::accept(m_fd, (struct sockaddr*)&addr, &addrlen);
    if (fd < -1){
        error(ErrorAccept);
        return;
    }
    log(L_DEBUG, "Accept connection");
    accept(fd, inet_ntoa(addr.sin_addr), htons(addr.sin_port));
}

void ServerSocket::write_ready()
{
}

ClientSocket::ClientSocket(int fd, const char *host, unsigned short port)
        : Socket(fd, host, port)
{
    if (fd != -1) setOpt();
}

ClientSocket::~ClientSocket()
{
    close();
}

void ClientSocket::close()
{
    if ((m_fd != -1) && (writeBuffer.readPos() < writeBuffer.size())){
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = 0;
        fd_set wf;
        FD_ZERO(&wf);
        FD_SET(m_fd, &wf);
        if ((select(m_fd + 1, NULL, &wf, NULL, &tv) > 0) && FD_ISSET(m_fd, &wf)){
            int size = writeBuffer.writePos() - writeBuffer.readPos();
            if (size > 1024) size = 1024;
            send(m_fd, writeBuffer.Data(writeBuffer.readPos()), size, 0);
        }
    }
    writeBuffer.init(0);
    readBuffer.init(0);
    Socket::close();
}

bool ClientSocket::have_data()
{
    if (m_connecting == Connected)
        return writeBuffer.readPos() < writeBuffer.writePos();
    return connectBuffer.readPos() < connectBuffer.writePos();
}

void ClientSocket::s5connect()
{
    unsigned long offs = connectBuffer.writePos();
    unsigned long addr = inet_addr(m_szResolveHost);
    if (addr != INADDR_NONE){
        connectBuffer
        << (char)0x05
        << (char)0x01
        << (char)0x00
        << (char)0x01
        << addr
        << m_nPort;
    }else{
        connectBuffer
        << (char)0x05
        << (char)0x01
        << (char)0x00
        << (char)0x03
        << (char)strlen(m_szHost);
        connectBuffer.pack(m_szHost, strlen(m_szHost));
        connectBuffer << m_nPort;
    }
    m_connecting = Socks5_WaitConnect;
    dumpPacket(connectBuffer, offs, "SOCKS5 connect send");
}

void ClientSocket::write_ready()
{
    Buffer *b = (m_connecting == Connected) ? &writeBuffer : &connectBuffer;
    if (b->readPos() < b->writePos()){
        int size = b->writePos() - b->readPos();
        if (size > 2048) size = 2048;
        int res = send(m_fd, b->Data(b->readPos()), size, 0);
        if (res != size){
            if ((res != -1) || (errno != EAGAIN))
                error(ErrorWrite);
            return;
        }
        b->incReadPos(res);
        if (b->readPos() == b->size()) b->init(0);
    }
}

#define HTTP_GOODSTRING  "HTTP/1.0 200 Connection established"
#define HTTP_GOODSTRING2 "HTTP/1.1 200 Connection established"

void ClientSocket::read_ready()
{
    if (m_resolving == Done){
        unsigned short port = m_nPort;
        if (m_connecting == ProxyResolve){
            m_connecting = Connecting;
            switch (Sockets::proxyType){
            case PROXY_NONE:
                m_proxyAddr = m_remoteAddr;
                break;
            case PROXY_SOCKS4:
                port = Sockets::proxyPort;
                m_connecting = HostResolve;
                m_proxyAddr = m_remoteAddr;
                m_szResolveHost = strdup(m_szHost);
                struct in_addr addr;
                if ((addr.s_addr = inet_addr(m_szResolveHost)) != INADDR_NONE){
                    m_remoteAddr = addr.s_addr;
                    m_resolving = Done;
                    free(m_szResolveHost);
                    m_szResolveHost = NULL;
                }else{
                    m_resolving = Resolving;
                    m_remoteAddr = 0;
                    resolve();
                }
                if (m_resolving != Done)
                    return;
                break;
            case PROXY_SOCKS5:
            case PROXY_HTTP:
                m_proxyAddr = m_remoteAddr;
                port = Sockets::proxyPort;
                break;
            default:
                log(L_ERROR, "Unknown proxy type");
                error(ErrorProtocol);
            }
        }

        if (m_connecting == HostResolve)
            m_connecting = Connecting;

        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons(port);
        remote_addr.sin_addr.s_addr = m_proxyAddr;

        if (m_remoteAddr == 0) return;
        if (m_remoteAddr == (unsigned long)(-1)){
            error(ErrorResolve);
            close();
            return;
        }
        log(L_DEBUG, "Connect %s:%u", inet_ntoa(remote_addr.sin_addr), port);
        if (::connect(m_fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr)))
        {
#if WIN32
            int err = h_errno;
            if (h_errno != WSAEWOULDBLOCK){
#else
            if (errno != EINPROGRESS){
#endif
                error(ErrorConnect);
                close();
                return;
            }
        }
        m_resolving = None;
        return;
    }

    switch (m_connecting){
    case Connected:
        break;
    case Socks4_Wait:{
            char buff[12];
            int r = recv(m_fd, buff, 9, 0);
            if (r == 0){
                error(ErrorDisconnect);
                return;
            }
            if (r < 0){
                if (errno != EAGAIN)
                    error(ErrorRead);
                return;
            }
            unsigned long offs = readBuffer.writePos();
            readBuffer.pack(buff, r);
            dumpPacket(readBuffer, offs, "HTTP answer");
            readBuffer.init(0);
            if ((r < 4) && (buff[1] != 90)) {
                log(L_ERROR, "Bad proxy answer");
                error(ErrorProtocol);
                return;
            }
            m_connecting = Connected;
            connect_ready();
            return;
        }
    case Socks5_Wait:{
            unsigned char buff[2];
            int r = recv(m_fd, (char*)buff, sizeof(buff), 0);
            if (r == 0){
                error(ErrorDisconnect);
                return;
            }
            if (r < 0){
                if (errno != EAGAIN)
                    error(ErrorRead);
                return;
            }
            unsigned long offs = readBuffer.writePos();
            readBuffer.pack((char*)buff, r);
            dumpPacket(readBuffer, offs, "SOCKS5 answer");
            readBuffer.init(0);
            if ((buff[0] != 0x05) || (buff[1] == 0xFF)){
                log(L_ERROR, "Bad proxy answer");
                error(ErrorProtocol);
                return;
            }
            if (buff[1] == 0x02){
                unsigned long offs = connectBuffer.writePos();
                connectBuffer << (char)0x01;
                connectBuffer << (char)strlen(Sockets::proxyUser.c_str());
                connectBuffer.pack(Sockets::proxyUser.c_str(), strlen(Sockets::proxyUser.c_str()));
                connectBuffer << (char)strlen(Sockets::proxyPasswd.c_str());
                connectBuffer.pack(Sockets::proxyPasswd.c_str(), strlen(Sockets::proxyPasswd.c_str()));
                m_connecting = Socks5_WaitAuth;
                dumpPacket(connectBuffer, offs, "SOCKS5 auth send");
                return;
            }
            s5connect();
            return;
        }
    case Socks5_WaitAuth:{
            char buff[2];
            int r = recv(m_fd, buff, sizeof(buff), 0);
            if (r == 0){
                error(ErrorDisconnect);
                return;
            }
            if (r < 0){
                if (errno != EAGAIN)
                    error(ErrorRead);
                return;
            }
            unsigned long offs = readBuffer.writePos();
            readBuffer.pack(buff, r);
            dumpPacket(readBuffer, offs, "SOCKS5 answer");
            readBuffer.init(0);
            if ((buff[0] != 0x01) || (buff[1] == 0)){
                log(L_ERROR, "Bad proxy answer");
                error(ErrorProtocol);
                return;
            }
            s5connect();
            return;
        }
    case Socks5_WaitConnect:{
            char buff[10];
            int r = recv(m_fd, buff, sizeof(buff), 0);
            if (r == 0){
                error(ErrorDisconnect);
                return;
            }
            if (r < 0){
                if (errno != EAGAIN)
                    error(ErrorRead);
                return;
            }
            unsigned long offs = readBuffer.writePos();
            readBuffer.pack(buff, r);
            dumpPacket(readBuffer, offs, "SOCKS5 answer");
            readBuffer.init(0);
            if ((buff[0] != 0x05) || (buff[1] != 0x00)) {
                log(L_ERROR, "Bad proxy answer");
                error(ErrorProtocol);
                return;
            }
            m_connecting = Connected;
            connect_ready();
            return;
        }
    case Http_Wait:{
            char buff[8192];
            int r = recv(m_fd, buff, sizeof(buff), 0);
            if (r == 0){
                error(ErrorDisconnect);
                return;
            }
            if (r < 0){
                if (errno != EAGAIN)
                    error(ErrorRead);
                return;
            }
            buff[r] = 0;
            unsigned long offs = readBuffer.writePos();
            readBuffer.pack(buff, r);
            dumpPacket(readBuffer, offs, "HTTP answer");
            readBuffer.init(0);
            if (memcmp(buff, HTTP_GOODSTRING, strlen(HTTP_GOODSTRING)) ||
                    memcmp(buff, HTTP_GOODSTRING2, strlen(HTTP_GOODSTRING2))){
                log(L_ERROR, "Bad proxy answer %s", buff);
                error(ErrorProtocol);
            }
            m_connecting = Connected;
            connect_ready();
            return;
        }
    default:
        log(L_ERROR, "Bad connecting state");
        break;
    }

    if (readBuffer.writePos() == readBuffer.size()){
        char c;
        int read = recv(m_fd, &c, 1, 0);
        if (read == 0){
            error(ErrorDisconnect);
            return;
        }
        if (read < 0){
            if (errno != EAGAIN)
                error(ErrorRead);
            return;
        }
        readBuffer.pack(&c, 1);
        if (c == '\n'){
            readBuffer.allocate(readBuffer.size() + 1, 0);
            *(readBuffer.Data(readBuffer.size())) = 0;
            packet_ready();
        }
        return;
    }
    int read = recv(m_fd, readBuffer.Data(readBuffer.writePos()),
                    readBuffer.size() - readBuffer.writePos(), 0);
    if (read == 0){
        error(ErrorDisconnect);
        return;
    }
    if (read < 0){
        if (errno != EAGAIN)
            error(ErrorRead);
        return;
    }
    readBuffer.setWritePos(readBuffer.writePos() + read);
    if (readBuffer.writePos() == readBuffer.size())
        packet_ready();
}

void ClientSocket::create_socket()
{
    close();
    m_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_fd == -1)
    {
        error(ErrorCreate);
        return;
    }
    setOpt();
    struct sockaddr_in local_addr;
    memset(&local_addr.sin_zero, 0, 8);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = 0;
    local_addr.sin_addr.s_addr = 0;
    if (bind(m_fd, (struct sockaddr *)&local_addr, sizeof(sockaddr_in)) == -1)
    {
        error(ErrorBind);
        close();
        return;
    }
    if (pSockets) pSockets->createSocket(this);
}

void ClientSocket::resolve()
{
    // try and resolve hostname
    struct hostent *he = gethostbyname(m_szHost);
    if (he == NULL){
        m_remoteAddr = (unsigned long)(-1);
    }
    m_remoteAddr = *((unsigned long *)(he->h_addr));
    m_resolving = Done;
}

void ClientSocket::connect(const char *host, unsigned short port)
{
    create_socket();
    if (m_fd == -1)
        return;

    connectBuffer.init(0);
    m_connecting = ProxyResolve;
    switch (Sockets::proxyType){
    case PROXY_NONE:
        m_szResolveHost = strdup(host);
        break;
    case PROXY_SOCKS4:
    case PROXY_SOCKS5:
    case PROXY_HTTP:
        m_szResolveHost = strdup(Sockets::proxyHost.c_str());
        break;
    default:
        log(L_ERROR, "Unknown proxy type");
        error(ErrorProtocol);
    }

    m_szHost = strdup(host);
    m_nPort = port;

    struct in_addr addr;

    if ((addr.s_addr = inet_addr(m_szResolveHost)) != INADDR_NONE){
        m_remoteAddr = addr.s_addr;
        m_resolving = Done;
        free(m_szResolveHost);
        m_szResolveHost = NULL;
    }else{
        m_resolving = Resolving;
        m_remoteAddr = 0;
        resolve();
    }
    if (m_resolving == Done) read_ready();
}

void Socket::dumpPacket(Buffer &b, unsigned start, const char *operation)
{
    if ((log_level & L_PACKET) == 0) return;
    string res;
    log(L_PACKET, "%s %u bytes", operation, b.size() - start);
    char line[81];
    char *p1 = line;
    char *p2 = line;
    unsigned n = 20;
    unsigned offs = 0;
    for (unsigned i = start; i < b.size(); i++, n++){
        char buf[32];
        if (n == 16)
            log(L_PACKET | L_SILENT, "%s", line);
        if (n >= 16){
            memset(line, ' ', 80);
            line[80] = 0;
            snprintf(buf, sizeof(buf), "     %04X: ", offs);
            memcpy(line, buf, strlen(buf));
            p1 = line + strlen(buf);
            p2 = p1 + 52;
            n = 0;
            offs += 0x10;
        }
        if (n == 8) p1++;
        unsigned char c = (unsigned char)*(b.Data(i));
        *(p2++) = ((c >= ' ') && (c != 0x7F)) ? c : '.';
        snprintf(buf, sizeof(buf), "%02X ", c);
        memcpy(p1, buf, 3);
        p1 += 3;
    }
    if (n <= 16) log(L_PACKET | L_SILENT, "%s", line);
}


