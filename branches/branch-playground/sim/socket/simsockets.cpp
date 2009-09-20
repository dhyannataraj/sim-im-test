
#include <list>

#include "simsockets.h"

#include "stdresolver.h"
#include "simresolver.h"
#include "simclientsocket.h"
#include "simserversocket.h"

namespace SIM
{
    SIMSockets::SIMSockets(QObject *parent)
        : SocketFactory(parent)
    {
        m_resolver = new StdResolver(this, QString::null);
    }

    SIMSockets::SIMSockets(QObject *parent, IResolver* resolver) : SocketFactory(parent),
        m_resolver(resolver)
    {
    }

    SIMSockets::~SIMSockets()
    {
    }

    void SIMSockets::checkState()
    {
#ifdef WIN32
        bool state;
        if (get_connection_state(state))
            setActive(state);
#endif
    }

    void SIMSockets::idle()
    {
        SocketFactory::idle();
    }

    void SIMSockets::resolve(const QString &host)
    {
        IResolver* resolver = m_resolver->clone(host);
        resolvers.push_back(resolver);
    }

    void SIMSockets::resultsReady()
    {
        std::list<IResolver*>::iterator it;
        for (it = resolvers.begin(); it != resolvers.end();){
            IResolver *r = *it;
            if (!r->isDone()){
                ++it;
                continue;
            }
            bool isActive;
            if (r->isTimeout()){
                isActive = false;
            }else{
                isActive = true;
            }
            if (r->addr() == INADDR_NONE)
                isActive = false;
#ifdef WIN32
            bool bState;
            if (get_connection_state(bState))
                isActive = bState;
#endif
            setActive(isActive);
            emit resolveReady(r->addr(), r->host());
            resolvers.remove(r);
            delete r;
            it = resolvers.begin();
        }
    }

    void SIMSockets::setResolver(IResolver* resolver)
    {
        if(m_resolver)
            delete m_resolver;
        m_resolver = resolver;
    }

    Socket *SIMSockets::createSocket()
    {
        return new SIMClientSocket;
    }

    ServerSocket *SIMSockets::createServerSocket()
    {
        return new SIMServerSocket();
    }
    SocketFactory *getSocketFactory()
    {
        return PluginManager::factory;
    }

}

// vim: set expandtab:

