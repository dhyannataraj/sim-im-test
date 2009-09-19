
#ifndef SIM_SIMCLIENTSOCKET_H
#define SIM_SIMCLIENTSOCKET_H

#include <QObject>
#include <QTimer>

#include "socket.h"

class Q3Socket;

namespace SIM
{
    class SIMClientSocket : public QObject, public Socket
    {
        Q_OBJECT
    public:
        SIMClientSocket(Q3Socket *s=NULL);
        virtual ~SIMClientSocket();
        virtual int read(char *buf, unsigned int size);
        virtual void write(const char *buf, unsigned int size);
        virtual void connect(const QString &host, unsigned short port);
        virtual unsigned long localHost();
        virtual void pause(unsigned);
        virtual void close();
        virtual int getFd();

    signals:
        void interfaceDown(int sockfd); // Probably, sockfd is not needed
        void interfaceUp(int sockfd);

    protected slots:
        void slotConnected();
        void slotConnectionClosed();
        void slotReadReady();
        void slotBytesWritten(int);
        void slotBytesWritten();
        void slotError(int);
        void slotLookupFinished(int);
        void resolveReady(unsigned long addr, const QString &host);
        void timeout();
        void error(int errcode);
        void checkInterface();
        void timerEvent(QTimerEvent* ev);

    protected:
        int m_carrierCheckTimer;
        bool m_state;
        void timerStop();
        unsigned short port;
        QString host;
        Q3Socket *sock;
        QTimer  *timer;
        bool bInWrite;
        QString m_interface;
    };
}

#endif

// vim: set expandtab:
