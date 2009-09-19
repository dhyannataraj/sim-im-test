#include <QFile>
#include <Q3Socket>
#include <Q3SocketDevice>
#include <QSocketNotifier>
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

#include "simserversocket.h"

#include "log.h"
#include "misc.h"
#include "serversocketnotify.h"
#include "socketfactory.h"
#include "simclientsocket.h"
#include "tcpclient.h"

namespace SIM
{
    SIMServerSocket::SIMServerSocket()
    {
        sn = NULL;
        sock = new Q3SocketDevice;
    }

    SIMServerSocket::~SIMServerSocket()
    {
        close();
    }

    void SIMServerSocket::close()
    {
        if (sn){
            delete sn;
            sn = NULL;
        }
        if (sock){
            delete sock;
            sock = NULL;
        }
        if (!m_name.isEmpty())
            QFile::remove(m_name);
    }

    void SIMServerSocket::bind(unsigned short minPort, unsigned short maxPort, TCPClient *client)
    {
        if (client && notify){
            EventSocketListen e(notify, client);
            if (e.process())
                return;
        }
        unsigned short startPort = (unsigned short)(minPort + get_random() % (maxPort - minPort + 1));
        bool bOK = false;
        for (m_nPort = startPort;;){
            if (sock->bind(QHostAddress(), m_nPort)){
                bOK = true;
                break;
            }
            if (++m_nPort > maxPort)
                m_nPort = minPort;
            if (m_nPort == startPort)
                break;
        }
        if (!bOK || !sock->listen(50)){
            error(I18N_NOOP("Can't allocate port"));
            return;
        }
        listen(client);
    }

#ifndef WIN32

    void SIMServerSocket::bind(const char *path)
    {
        m_name = QFile::decodeName(path);
        QString user_id;
        uid_t uid = getuid();
        struct passwd *pwd = getpwuid(uid);
        if (pwd){
            user_id = pwd->pw_name;
        }else{
            user_id = QString::number(uid);
        }
        m_name = m_name.replace(QRegExp("\\%user\\%"), user_id);
        QFile::remove(m_name);

        int s = socket(PF_UNIX, SOCK_STREAM, 0);
        if (s == -1){
            error("Can't create listener");
            return;
        }
        sock->setSocket(s, Q3SocketDevice::Stream);

        struct sockaddr_un nsun;
        nsun.sun_family = AF_UNIX;
        strcpy(nsun.sun_path, QFile::encodeName(m_name));
        if (::bind(s, (struct sockaddr*)&nsun, sizeof(nsun)) < 0){
            log(L_WARN, "Can't bind %s: %s", nsun.sun_path, strerror(errno));
            error("Can't bind");
            return;
        }
        if (::listen(s, 156) < 0){
            log(L_WARN, "Can't listen %s: %s", nsun.sun_path, strerror(errno));
            error("Can't listen");
            return;
        }
        listen(NULL);
    }

#endif

    void SIMServerSocket::error(const char *err)
    {
        close();
        if (notify && notify->error(err)){
            notify->m_listener = NULL;
            getSocketFactory()->remove(this);
        }
    }

    void SIMServerSocket::listen(TCPClient*)
    {
        sn = new QSocketNotifier(sock->socket(), QSocketNotifier::Read, this);
        connect(sn, SIGNAL(activated(int)), this, SLOT(activated(int)));
        if (notify)
            notify->bind_ready(m_nPort);
    }

    void SIMServerSocket::activated(int)
    {
        if (sock == NULL) return;
        int fd = sock->accept();
        if (fd >= 0){
            log(L_DEBUG, "accept ready");
            if (notify){
                Q3Socket *s = new Q3Socket;
                s->setSocket(fd);
                if (notify->accept(new SIMClientSocket(s), htonl(s->address().toIPv4Address())))
                {
                    if (notify)
                        notify->m_listener = NULL;
                    getSocketFactory()->remove(this);
                }
            }else{
#ifdef WIN32
                ::closesocket(fd);
#else
                ::close(fd);
#endif
            }
        }
    }

    void SIMServerSocket::activated()
    {
    }

}

// vim: set expandtab:

