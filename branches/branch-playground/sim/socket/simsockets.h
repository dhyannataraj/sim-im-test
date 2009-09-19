
#ifndef SIM_SIMSOCKETS_H
#define SIM_SIMSOCKETS_H

#include <QObject>
#include <list>
#include "socketfactory.h"

#include "iresolver.h"

#ifndef INADDR_NONE
	#define INADDR_NONE     0xFFFFFFFF
#endif

namespace SIM
{
    class SIMSockets : public SocketFactory
    {
        Q_OBJECT
    public:
        SIMSockets(QObject *parent);
        virtual ~SIMSockets();
        virtual Socket *createSocket();
        virtual ServerSocket *createServerSocket();
        void resolve(const QString &host);

    signals:
        void resolveReady(unsigned long res, const QString &);

    public slots:
        void resultsReady();
        void idle();
        void checkState();

    protected:
        std::list<IResolver*> resolvers;
    };
}

#endif

// vim: set expandtab:

