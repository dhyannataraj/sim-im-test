
#ifndef SIM_TESTCLIENTSOCKET_H
#define SIM_TESTCLIENTSOCKET_H

#include <QtTest/QtTest>
#include <QObject>
#include "event.h"
#include "socket/socket.h"
#include "socket/socketfactory.h"
#include "socket/iresolver.h"

namespace testClientSocket
{
    class TestSocket : public SIM::Socket
    {
    public:
        TestSocket();
        virtual ~TestSocket();
        virtual int read(char *buf, unsigned int size);
        virtual void write(const char *buf, unsigned int size);
        virtual void close();
        virtual unsigned long localHost();
        virtual void pause(unsigned);

        static int socketCounter;
        int length;
    };

    class TestFactory : public SIM::SocketFactory
    {
    public:
        TestFactory(QObject* parent);
        virtual ~TestFactory();
        virtual SIM::Socket* createSocket();
        virtual SIM::ServerSocket* createServerSocket();
        virtual bool erase(SIM::ClientSocket* sock);
        virtual void resolve(const QString &host);
        virtual void setResolver(SIM::IResolver* resolver);

        // Testing variables:
        int socketCounter;
    };

    class TestClientSocketNotify : public SIM::ClientSocketNotify
    {
    public:
        TestClientSocketNotify();
        virtual bool error_state(const QString &err, unsigned code = 0);
        virtual void connect_ready();
        virtual void packet_ready();
        virtual void write_ready() {}
        virtual void resolve_ready(unsigned long) {}
        
    };

    /*
    class TestThread : public QThread
    {
    public:
        TestThread(QObject* parent);

    };
    */

    class TestClientSocket : public QObject, public SIM::EventReceiver
    {
        Q_OBJECT
    public:
        virtual bool processEvent(class Event*);
    private slots:
        void testCtorDtor();
        void testReading();
        void initTestCase();
        void cleanupTestCase();

    private:
        SIM::ClientSocket* m_socket;
        TestClientSocketNotify* m_notify;
        TestFactory* m_factory;
    };
}

#endif

// vim: set expandtab:

