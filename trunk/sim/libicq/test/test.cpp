
#include "icqclient.h"
#include "buffer.h"
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

#ifdef WIN32
#define sockclose closesocket
#define socklen_t int
#else
#define sockclose ::close
#endif

// ---------------------------------------------------------------------------------

class SocketBase
{
public:
    SocketBase();
    virtual ~SocketBase();
    virtual void read_ready() = 0;
    virtual void write_ready() = 0;
    virtual void error_state() = 0;
    virtual bool have_data() = 0;
    virtual void idle() = 0;
    bool getLocalAddr(unsigned long &host, unsigned short &port);
    int fd;
    static void process(unsigned timeout);
protected:
    static list<SocketBase*> sockets;
    void close();
};

list<SocketBase*> SocketBase::sockets;

void SocketBase::process(unsigned timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int max_fd = 0;
    fd_set rf;
    fd_set wf;
    fd_set ef;
    FD_ZERO(&rf);
    FD_ZERO(&wf);
    FD_ZERO(&ef);
    list<SocketBase*>::iterator it;
    for (;;){
        bool ok = true;
        for (it = sockets.begin(); it != sockets.end(); it++){
            if ((*it) == NULL){
                sockets.remove(*it);
                ok = false;
                break;
            }
        }
        if (ok) break;
    }
    for (it = sockets.begin(); it != sockets.end(); it++){
        if ((*it) == NULL) continue;
        int fd = (*it)->fd;
        if (fd == -1) continue;
        if (fd > max_fd) max_fd = fd;
        FD_SET(fd, &rf);
        FD_SET(fd, &ef);
        if ((*it)->have_data()) FD_SET(fd, &wf);
    }
    int res = select(max_fd + 1, &rf, &wf, &ef, &tv);
    if (res <= 0){
        for (it = sockets.begin(); it != sockets.end(); it++){
            if ((*it) == NULL) continue;
            (*it)->idle();
        }
        return;
    }
    for (it = sockets.begin(); it != sockets.end(); it++){
        if ((*it) == NULL) continue;
        int fd = (*it)->fd;
        if (fd == -1){
            (*it)->idle();
            continue;
        }
        if (FD_ISSET(fd, &ef)){
            (*it)->error_state();
            continue;
        }
        if (FD_ISSET(fd, &rf)){
            (*it)->read_ready();
            continue;
        }
        if (FD_ISSET(fd, &wf)){
            (*it)->write_ready();
        }
    }
}

SocketBase::SocketBase()
{
    fd = -1;
    sockets.push_back(this);
}

SocketBase::~SocketBase()
{
    close();
    for (list<SocketBase*>::iterator it = sockets.begin(); it != sockets.end(); ++it){
        if ((*it) == this){
            *it = NULL;
            break;
        }
    }
}

void SocketBase::close()
{
    if (fd != -1){
        sockclose(fd);
        fd = -1;
    }
}

bool SocketBase::getLocalAddr(unsigned long &host, unsigned short &port)
{
    struct sockaddr_in addr;
    socklen_t sizeofAddr = sizeof(addr);
    if (getsockname(fd, (struct sockaddr *)&addr, &sizeofAddr) < 0)
        return false;
    host = htonl(addr.sin_addr.s_addr);
    port = htons(addr.sin_port);
    return true;
}

// ---------------------------------------------------------------------------------

class MySocket : public SocketBase, public Socket
{
public:
    MySocket();
    ~MySocket();
    virtual int read(char *buf, unsigned int size);
    virtual void write(const char *buf, unsigned int size);
    virtual void connect(const char *host, int port);
    virtual void close();
    virtual unsigned long localHost();
    virtual void pause(unsigned);
protected:
    virtual void read_ready();
    virtual void write_ready();
    virtual void error_state();
    virtual bool have_data();
    virtual void idle();
    bool bConnecting;
    Buffer outBuf;
    time_t pauseTime;
};

MySocket::MySocket()
{
    pauseTime = 0;
    bConnecting = 0;
}

MySocket::~MySocket()
{
    close();
}

void MySocket::close()
{
    bConnecting = false;
    outBuf.init(0);
    SocketBase::close();
}

void MySocket::connect(const char *host, int port)
{
    close();
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        if (notify) notify->error_state(ErrorConnect);
        return;
    }
#ifndef WIN32
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl != -1)
        fcntl(fd, F_SETFL, fl | O_NONBLOCK);
#endif
    struct sockaddr_in local_addr;
    memset(&local_addr.sin_zero, 0, 8);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = 0;
    local_addr.sin_addr.s_addr = 0;
    if (bind(fd, (struct sockaddr *)&local_addr, sizeof(sockaddr_in)) == -1)
    {
        if (notify) notify->error_state(ErrorConnect);
        close();
        return;
    }
    struct sockaddr_in remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    if ((remote_addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE){
        struct hostent *he = gethostbyname(host);
        if (he == NULL){
            log(L_WARN, "Can't resolve %s", host);
            close();
            if (notify) notify->error_state(ErrorConnect);
            return;
        }
        remote_addr.sin_addr.s_addr = *((unsigned long *)(he->h_addr));
    }
    if (::connect(fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr)))
    {
#if WIN32
        int err = h_errno;
        if (h_errno != WSAEWOULDBLOCK){
#else
        if (errno != EINPROGRESS){
#endif
            close();
            if (notify) notify->error_state(ErrorConnect);
            return;
        }
    }
    bConnecting = true;
}

int MySocket::read(char *buf, unsigned int size)
{
    int readn = recv(fd, buf, size, 0);
    if (readn == 0){
        if (notify) notify->error_state(ErrorConnectionClosed);
        return -1;
    }
    if (readn < 0){
        if (errno != EAGAIN){
            if (notify) notify->error_state(ErrorRead);
            return -1;
        }
        return 0;
    }
    return readn;
}

void MySocket::write(const char *buf, unsigned int size)
{
    outBuf.pack(buf, size);
}

void MySocket::pause(unsigned int t)
{
    if (pauseTime) return;
    time(&pauseTime);
    pauseTime += t;
}

unsigned long MySocket::localHost()
{
    unsigned long host = 0;
    unsigned short port = 0;
    getLocalAddr(host, port);
    return host;
}

void MySocket::idle()
{
    if (have_data()) return;
    if (pauseTime == 0) return;
    time_t now;
    time(&now);
    if (now > pauseTime) return;
    pauseTime = 0;
    if (notify) notify->write_ready();
}

void MySocket::read_ready()
{
    if (notify) notify->read_ready();
}

void MySocket::error_state()
{
    if (notify) notify->error_state(ErrorConnectionClosed);
}

void MySocket::write_ready()
{
    if (bConnecting){
        bConnecting = false;
        if (notify) notify->connect_ready();
        return;
    }
    if (outBuf.readPos() < outBuf.writePos()){
        int size = outBuf.writePos() - outBuf.readPos();
        if (size > 2048) size = 2048;
        int res = send(fd, outBuf.Data(outBuf.readPos()), size, 0);
        if (res != size){
            if ((res != -1) || (errno != EAGAIN)){
                if (notify)
                    notify->error_state(ErrorWrite);
            }
            return;
        }
        outBuf.incReadPos(res);
        if (outBuf.readPos() == outBuf.size()) outBuf.init(0);
    }
}

bool MySocket::have_data()
{
    return bConnecting || (outBuf.readPos() < outBuf.writePos());
}

// ---------------------------------------------------------------------------------

class MyServerSocket : public SocketBase, public ServerSocket
{
public:
    MyServerSocket();
    ~MyServerSocket();
    virtual unsigned short port();
protected:
    unsigned short p;
    virtual void read_ready();
    virtual void write_ready();
    virtual void error_state();
    virtual bool have_data();
    virtual void idle();
};

MyServerSocket::MyServerSocket()
{
    p = 0;
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        log(L_WARN, "Can't create socket %s", strerror(errno));
        return;
    }

#ifndef WIN32
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl != -1)
        fcntl(fd, F_SETFL, fl | O_NONBLOCK);
#endif

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

    struct sockaddr_in addr;
    memset(&addr.sin_zero, 0, 8);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(sockaddr_in)) < 0){
        log(L_WARN, "Can't bind %s", strerror(errno));
        close();
        return;
    }
    unsigned long host;
    getLocalAddr(host, p);
    if (::listen(fd, 256) == -1)
    {
        log(L_WARN, "Can't listen %s", strerror(errno));
        close();
        return;
    }
    return;
}

MyServerSocket::~MyServerSocket()
{
    close();
}

void MyServerSocket::read_ready()
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int nfd = ::accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (nfd < -1){
        log(L_WARN, "Accept error %s", strerror(errno));
        return;
    }
    log(L_DEBUG, "Accept connection");
    MySocket *n = new MySocket;
    n->fd = nfd;
    if (notify == NULL){
        delete n;
        return;
    }
    notify->accept(n);
}

void MyServerSocket::write_ready()
{
}

void MyServerSocket::error_state()
{
}

bool MyServerSocket::have_data()
{
    return false;
}

void MyServerSocket::idle()
{
}

unsigned short MyServerSocket::port()
{
    return p;
}

// ---------------------------------------------------------------------------------

class Client : public ICQClient
{
public:
    Client();
    virtual Socket *createSocket();
    virtual ServerSocket *createServerSocket();
};

Client::Client()
{
}

Socket *Client::createSocket()
{
    return new MySocket;
}

ServerSocket *Client::createServerSocket()
{
    return new MyServerSocket;
}

// -----------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Usage %s uin passwd\n", argv[0]);
        return 1;
    }
    unsigned long uin = atol(argv[1]);
    if (uin == 0){
        fprintf(stderr, "Bad uin\n");
        return 1;
    }
    log_level = 15;
    Client c;
    c.owner->Uin = uin;
    c.storePassword(argv[2]);
    c.setStatus(ICQ_STATUS_ONLINE);
    for (;;){
        SocketBase::process(1);
        c.idle();
    }
    return 0;
}
