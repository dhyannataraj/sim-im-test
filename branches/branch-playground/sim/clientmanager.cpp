
#include "clientmanager.h"

namespace SIM
{
    ClientManager::ClientManager()
    {
    }

    ClientManager::~ClientManager()
    {
    }

    void ClientManager::addClient(ClientPtr client)
    {
        m_clients.insert(client->name(), client);
    }
    
    ClientPtr ClientManager::client(const QString& name)
    {
        ClientMap::iterator it = m_clients.find(name);
        if(it != m_clients.end())
            return it.value();
        return ClientPtr();
    }

    static ClientManager* g_clientManager = 0;

    ClientManager* getClientManager()
    {
        return g_clientManager;
    }

    void createClientManager()
    {
        if(!g_clientManager)
            g_clientManager = new ClientManager();
    }

    void destroyClientManager()
    {
        if(g_clientManager)
        {
            delete g_clientManager;
            g_clientManager = 0;
        }
    }
}

// vim: set expandtab:

