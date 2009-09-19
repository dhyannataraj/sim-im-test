
#include <Q3Socket>
#include <Q3SocketDevice>
#include <QTimerEvent>

#ifdef WIN32
	#include <winsock.h>
	#include <wininet.h>
#else
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <pwd.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
#endif
#include <errno.h>

#include "simclientsocket.h"

#include "socketfactory.h"
#include "simsockets.h"
#include "log.h"
#include "misc.h"

namespace SIM
{
    const unsigned CONNECT_TIMEOUT = 60;

    SIMClientSocket::SIMClientSocket(Q3Socket *s)
    {
        sock = s;
        if (sock == NULL)
            sock = new Q3Socket(this);
        QObject::connect(sock, SIGNAL(connected()), this, SLOT(slotConnected()));
        QObject::connect(sock, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
        QObject::connect(sock, SIGNAL(error(int)), this, SLOT(slotError(int)));
        QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(slotReadReady()));
        QObject::connect(sock, SIGNAL(bytesWritten(int)), this, SLOT(slotBytesWritten(int)));
        bInWrite = false;
        timer = NULL;
        m_carrierCheckTimer = 0;
    }

    SIMClientSocket::~SIMClientSocket()
    {
        if (!sock)
            return;
        timerStop();
        sock->close();

        if (sock->state() == Q3Socket::Closing)
            sock->connect(sock, SIGNAL(delayedCloseFinished()), SLOT(deleteLater()));
        else
            delete sock;
    }

    void SIMClientSocket::close()
    {
        timerStop();
        sock->close();
    }

    void SIMClientSocket::timerStop()
    {
        if (timer){
            delete timer;
            timer = NULL;
        }
    }

    void SIMClientSocket::slotLookupFinished(int state)
    {
        log(L_DEBUG, "Lookup finished %u", state);
        if (state == 0){
            log(L_WARN, "Can't lookup");
            notify->error_state(I18N_NOOP("Connect error"));
            getSocketFactory()->setActive(false);
        }
    }

    int SIMClientSocket::read(char *buf, unsigned int size)
    {
        unsigned available = sock->bytesAvailable();
        if (size > available)
            size = available;
        if (size == 0)
            return size;
        int res = sock->read(buf, size);
        if (res < 0){
            log(L_DEBUG, "QClientSocket::read error %u", errno);
            if (notify)
                notify->error_state(I18N_NOOP("Read socket error"));
            return -1;
        }
        return res;
    }

    void SIMClientSocket::write(const char *buf, unsigned int size)
    {
        bInWrite = true;
        int res = sock->write(buf, size);
        bInWrite = false;
        if (res != (int)size){
            if (notify)
                notify->error_state(I18N_NOOP("Write socket error"));
            return;
        }
        if (sock->bytesToWrite() == 0)
            QTimer::singleShot(0, this, SLOT(slotBytesWritten()));
    }

    void SIMClientSocket::connect(const QString &_host, unsigned short _port)
    {
        port = _port;
        host = _host;
        if (host.isNull())
            host=""; // Avoid crashing when _host is NULL
#ifdef WIN32
        bool bState;
        if (get_connection_state(bState) && !bState){
            QTimer::singleShot(0, this, SLOT(slotConnectionClosed()));
            return;
        }
#endif
        log(L_DEBUG, QString("Connect to %1:%2").arg(host).arg(port));
        if (inet_addr(host.toUtf8()) == INADDR_NONE){
            log(L_DEBUG, QString("Start resolve %1").arg(host));
            SIMSockets *s = static_cast<SIMSockets*>(getSocketFactory());
            QObject::connect(s, SIGNAL(resolveReady(unsigned long, const QString&)),
                    this, SLOT(resolveReady(unsigned long, const QString&)));
            s->resolve(host);
            return;
        }
        resolveReady(inet_addr(host.toUtf8()), host);
    }

    void SIMClientSocket::resolveReady(unsigned long addr, const QString &_host)
    {
        if (_host != host)
            return;
        if (addr == INADDR_NONE){
            if (notify)
                notify->error_state(I18N_NOOP("Can't resolve host"));
            return;
        }
        if (notify)
            notify->resolve_ready(addr);
        in_addr a;
        a.s_addr = addr;
        host = inet_ntoa(a);
        log(L_DEBUG, QString("Resolve ready %1").arg(host));
        timerStop();
        timer = new QTimer(this);
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
        timer->start(CONNECT_TIMEOUT * 1000);
        sock->connectToHost(host, port);
    }

    void SIMClientSocket::slotConnected()
    {
        log(L_DEBUG, "Connected");
        timerStop();
        if (notify) notify->connect_ready();
        getSocketFactory()->setActive(true);
        m_state = true;
#if !defined(WIN32) && !defined(Q_OS_MAC)
        m_carrierCheckTimer = startTimer(10000); // FIXME hardcoded
#endif
    }

    void SIMClientSocket::slotConnectionClosed()
    {
        log(L_WARN, "Connection closed");
        timerStop();
        if (notify)
            notify->error_state(I18N_NOOP("Connection closed"));
#ifdef WIN32
        bool bState;
        if (get_connection_state(bState) && !bState)
            getSocketFactory()->setActive(false);
#endif
    }

    void SIMClientSocket::timeout()
    {
        QTimer::singleShot(0, this, SLOT(slotConnectionClosed()));
    }

    void SIMClientSocket::timerEvent(QTimerEvent* ev)
    {
        if(m_carrierCheckTimer != 0 && ev->timerId() == m_carrierCheckTimer)
        {
            checkInterface();
        }
    }

    void SIMClientSocket::checkInterface()
    {
#if !defined(WIN32) && !defined(Q_OS_MAC)
        int fd = sock->socket();
        if(fd == -1)
        {
            return;
        }
        struct ifreq ifr;
        struct ifreq* ifrp;
        struct ifreq ibuf[16];
        struct ifconf	ifc;

        ifc.ifc_len = sizeof(ifr)*16;
        ifc.ifc_buf = (caddr_t)&ibuf;
        memset(ibuf, 0, sizeof(struct ifreq)*16);

        int hret = ioctl(fd, SIOCGIFCONF, &ifc);
        if(hret == -1)
        {
            return;
        }
        bool iffound = false;
        for(int i = 0; i < ifc.ifc_len/sizeof(struct ifreq); i++)
        {
            ifrp = ibuf + i;
            strncpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));

            if  (
                    strcmp(ifr.ifr_name, "lo") == 0 ||
                    (htonl(((sockaddr_in*)&ifrp->ifr_addr)->sin_addr.s_addr) != sock->address().toIPv4Address())
                )	continue;

            m_interface = ifr.ifr_name;
            iffound = true;

            hret = ioctl(fd, SIOCGIFFLAGS, &ifr);
            if(hret != -1)
            {
                int state = ifr.ifr_flags & IFF_RUNNING;
                if(state < 0)
                {
                    log(L_DEBUG, "Incorrect state: %d (%s)", state, ifr.ifr_name);
                    return;
                }
                if((state == 0) && (m_state))
                {
                    m_state = false;
                    emit interfaceDown(fd);
                    EventInterfaceDown e(fd);
                    e.process();
                    return;
                }
                if((state != 0) && (!m_state))
                {
                    m_state = true;
                    emit interfaceUp(fd);
                    EventInterfaceUp e(fd);
                    e.process();
                    return;
                }
                return;
            }
        }
        if(!iffound)
        {
            m_state = false;
            emit interfaceDown(fd);
            EventInterfaceDown e(fd);
            e.process();
        }
#else
        return;
#endif
    }

    void SIMClientSocket::error(int errcode)
    {
        log(L_DEBUG, "SIMClientSocket::error(%d), SocketDevice error: %d", errcode, sock->socketDevice()->error());
    }

    void SIMClientSocket::slotReadReady()
    {
        if (notify)
            notify->read_ready();
    }

    void SIMClientSocket::slotBytesWritten(int)
    {
        slotBytesWritten();
    }

    void SIMClientSocket::slotBytesWritten()
    {
        if (bInWrite || (sock == NULL)) return;
        if ((sock->bytesToWrite() == 0) && notify)
            notify->write_ready();
    }

#ifdef WIN32
#define socklen_t int
#endif

    unsigned long SIMClientSocket::localHost()
    {
        unsigned long res = 0;
        int s = sock->socket();
        struct sockaddr_in addr;
        memset(&addr, sizeof(addr), 0);
        socklen_t size = sizeof(addr);
        if (getsockname(s, (struct sockaddr*)&addr, &size) >= 0)
            res = addr.sin_addr.s_addr;
        if (res == 0x7F000001){
            char hostName[255];
            if (gethostname(hostName,sizeof(hostName)) >= 0) {
                struct hostent *he = NULL;
                he = gethostbyname(hostName);
                if (he != NULL)
                    res = *((unsigned long*)(he->h_addr));
            }
        }
        return res;
    }

    void SIMClientSocket::slotError(int err)
    {
        if (err)
            log(L_DEBUG, "Slot error %u", err);
        timerStop();
        if (notify)
            notify->error_state(I18N_NOOP("Socket error"));
    }

    void SIMClientSocket::pause(unsigned t)
    {
        QTimer::singleShot(t * 1000, this, SLOT(slotBytesWritten()));
    }

    int SIMClientSocket::getFd()
    {
        return sock->socket();
    }

}

// vim: set expandtab:

