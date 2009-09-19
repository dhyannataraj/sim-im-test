
#ifndef SIM_SIMSERVERSOCKET_H
#define SIM_SIMSERVERSOCKET_H

#include <QObject>
#include <QString>

#include "serversocket.h"
#include "tcpclient.h"

class Q3SocketDevice;
class QSocketNotifier;

namespace SIM
{
    class SIMServerSocket : public QObject, public ServerSocket
    {
        Q_OBJECT
    public:
        SIMServerSocket();
        ~SIMServerSocket();
        virtual unsigned short port() { return m_nPort; }
        bool created() { return (sock != NULL); }
        void bind(unsigned short minPort, unsigned short maxPort, TCPClient *client);
#ifndef WIN32
        void bind(const char *path);
#endif
        void close();
    protected slots:
        void activated(int);
        void activated();
    protected:
        void listen(TCPClient*);
        void error(const char *err);
        Q3SocketDevice   *sock;
        QSocketNotifier *sn;
        QString			m_name;
        unsigned short  m_nPort;
    };

}

#endif

// vim: set expandtab:

