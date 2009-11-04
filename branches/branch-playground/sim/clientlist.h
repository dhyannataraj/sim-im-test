
#ifndef SIM_CLIENTLIST_H
#define SIM_CLIENTLIST_H

#include <QObject>
#include <vector>

namespace SIM
{
    class Client;
    class EXPORT ClientList : public QObject, public std::vector<Client*>
    {
        Q_OBJECT
    public:
        ClientList();
        virtual ~ClientList();
        void addToContacts();

    signals:
        void ignoreEvents(bool ignore);

    };
}

#endif

// vim: set expandtab:

