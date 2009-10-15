
#include <list>

#include "simsockets.h"

#include "stdresolver.h"
#include "simresolver.h"
#include "simclientsocket.h"
#include "simserversocket.h"
#include "fetch.h"

namespace SIM
{
    SIMSockets::SIMSockets(QObject *parent)
        : SocketFactory(parent)
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

    Socket *SIMSockets::createSocket()
    {
        return new SIMClientSocket();
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

