
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
        SIMSockets(QObject *parent, IResolver* resolver);
        virtual ~SIMSockets();
        virtual Socket *createSocket();
        virtual ServerSocket *createServerSocket();

        virtual void resolve(const QString &host);
        virtual void setResolver(IResolver* resolver);

    signals:
        void resolveReady(unsigned long res, const QString &);

    public slots:
        void resultsReady();
        void idle();
        void checkState();

    protected:
        std::list<IResolver*> resolvers;
    private:
        IResolver* m_resolver;
    };
}

#endif

// vim: set expandtab:

