
#include "contacts/client.h"
#include "clientlist.h"
#include "contacts.h"

namespace SIM
{
    ClientList::ClientList() : QObject(),
        std::vector<Client*>()
    {
    }

    ClientList::~ClientList()
    {
        emit ignoreEvents(true);
        for (ClientList::iterator it = begin(); it != end(); ++it)
            delete *it;
        emit ignoreEvents(false);
    }

    void ClientList::addToContacts()
    {
        for (ClientList::iterator it = begin(); it != end(); ++it)
            getContacts()->addClient(*it);
        clear();
    }
}

// vim: set expandtab:

