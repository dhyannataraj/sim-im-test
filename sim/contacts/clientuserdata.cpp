
#include <algorithm>
#include <vector>
#include <stdio.h>
#include "clientuserdata.h"
#include "contacts.h"
#include "client.h"
#include "imcontact.h"

namespace SIM
{
    struct _ClientUserData
    {
        Client  *client;
        IMContact* data;
    };

    class ClientUserDataPrivate : public std::vector<_ClientUserData>
    {
    public:
        ClientUserDataPrivate();
        ~ClientUserDataPrivate();
    };

    ClientUserDataPrivate::ClientUserDataPrivate()
    {
    }

    ClientUserDataPrivate::~ClientUserDataPrivate()
    {
        // why do I have to delete something here which is created somehwere else??
        for (ClientUserDataPrivate::iterator it = begin(); it != end(); ++it){
            _ClientUserData &d = *it;
            free_data(d.client->protocol()->userDataDef(), d.data);
            delete d.data;
        }
    }

    ClientUserData::ClientUserData()
    {
        p = new ClientUserDataPrivate;
    }

    ClientUserData::~ClientUserData()
    {
        delete p;
    }

    unsigned ClientUserData::size()
    {
        return p->size();
    }

    QString ClientUserData::property(const char *name)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            _ClientUserData &d = *it;
            Data *user_data = (Data*)d.data;
            for (const DataDef *def = d.client->protocol()->userDataDef(); def->name; def++){
                if (!strcmp(def->name, name)){
                    switch (def->type){
                        case DATA_STRING:
                        case DATA_UTF:
                            if (!user_data->str().isEmpty())
                                return user_data->str();
                        case DATA_ULONG:
                            if (user_data->toULong() != (unsigned long)(def->def_value))
                                return QString::number(user_data->toULong());
                        case DATA_LONG:
                            if (user_data->toLong() != (long)(def->def_value))
                                return QString::number(user_data->toLong());
                        default:
                            break;
                    }
                }
                user_data += def->n_values;
            }
        }
        return QString::null;
    }

    bool ClientUserData::have(void *data)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            if ((void*)it->data == data)
                return true;
        }
        return false;
    }

    Client *ClientUserData::activeClient(void *&data, Client *client)
    {
        ClientUserDataPrivate::iterator it;
        for (it = p->begin(); it != p->end(); ++it){
            if ((it->client == client) && (it->data == data))
                break;
            if (((IMContact*)(it->data))->Sign.toULong() != ((IMContact*)data)->Sign.toULong())
                continue;
            if (client->compareData(data, it->data))
                return NULL;
        }
        if (it == p->end())
            return NULL;
        if (client->getState() == Client::Connected)
            return client;
        for (++it; it != p->end(); ++it){
            if (it->client->getState() != Client::Connected)
                continue;
            if (((IMContact*)(it->data))->Sign.toULong() != ((IMContact*)data)->Sign.toULong())
                continue;
            if (client->compareData(data, it->data)){
                data = it->data;
                return it->client;
            }
        }
        return client;
    }


    static bool cmp_client_data(_ClientUserData p1, _ClientUserData p2)
    {
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *c = getContacts()->getClient(i);
            if (c == p1.client){
                if (c != p2.client)
                    return true;
                return p1.data < p2.data;
            }
            if (c == p2.client)
                return false;
        }
        return p1.data < p2.data;
    }

    void ClientUserData::sort()
    {
        std::sort(p->begin(), p->end(), cmp_client_data);
    }


    QByteArray ClientUserData::save() const
    {
        QByteArray res;
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            _ClientUserData &d = *it;
            if (d.client->protocol()->description()->flags & PROTOCOL_TEMP_DATA)
                continue;
            QByteArray cfg = d.data->serialize(); //save_data(d.client->protocol()->userDataDef(), d.data);
            if (cfg.length()){
                if (res.length())
                    res += '\n';
                res += '[';
                res += d.client->name().toUtf8();
                res += "]\n";
                res += cfg;
            }
        }
        return res;
    }

    void ClientUserData::load(Client *client, Buffer *cfg)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            Client *c = it->client;
            if(c == client)
                return;
        }
        _ClientUserData data;
        data.client = client;
        const DataDef *def = client->protocol()->userDataDef();
//        size_t size = 0;
//        for (const DataDef *d = def; d->name; ++d)
//            size += d->n_values;
//        data.data = new Data[size];
        data.data = client->protocol()->createIMContact();
        //load_data(def, data.data, cfg);
        data.data->deserialize(cfg);
        p->push_back(data);
    }
    

    void *ClientUserData::createData(Client *client)
    {
        _ClientUserData data;
        data.client = client;
        data.data = client->protocol()->createIMContact();
        p->push_back(data);
        return data.data;
    }

    void *ClientUserData::getData(Client *client)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            if (it->client == client)
                return it->data;
        }
        return NULL;
    }

    void ClientUserData::freeData(void *_data)
    {
        SIM::Data *data = (SIM::Data*)_data;
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            if ((void*)it->data == data){
                free_data(it->client->protocol()->userDataDef(), data);
                delete data;
                p->erase(it);
                return;
            }
        }
    }

    void ClientUserData::freeClientData(Client *client)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end();){
            if (it->client != client){
                ++it;
                continue;
            }
            //free_data(it->client->protocol()->userDataDef(), it->data);
            delete it->data;
            p->erase(it);
            it = p->begin();
        }
    }

    void ClientUserData::join(ClientUserData &data)
    {
        for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it)
            p->push_back(*it);
        data.p->clear();
        sort();
    }

    void ClientUserData::join(IMContact *cData, ClientUserData &data)
    {
        for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it){
            if (&it->data->Sign == &(cData->Sign)){
                p->push_back(*it);
                data.p->erase(it);
                break;
            }
        }
        sort();
    }

    class ClientDataIteratorPrivate
    {
    public:
        ClientDataIteratorPrivate(ClientUserDataPrivate *p, Client *client);
        void *operator ++();
        void reset();
        Client *m_lastClient;
    protected:
        ClientUserDataPrivate *m_p;
        ClientUserDataPrivate::iterator m_it;
        Client *m_client;
    };

    ClientDataIteratorPrivate::ClientDataIteratorPrivate(ClientUserDataPrivate *p, Client *client)
    {
        m_p = p;
        m_client = client;
        reset();
    }

    void *ClientDataIteratorPrivate::operator ++()
    {
        for (; m_it != m_p->end(); ++m_it){
            if ((m_client == NULL) || ((*m_it).client == m_client)){
                void *res = (*m_it).data;
                m_lastClient = (*m_it).client;
                ++m_it;
                return res;
            }
        }
        return NULL;
    }

    void ClientDataIteratorPrivate::reset()
    {
        m_lastClient = NULL;
        m_it = m_p->begin();
    }

    ClientDataIterator::ClientDataIterator() : p(0)
    {
        p = ClientDataIteratorPrivatePtr();
    }

    ClientDataIterator::ClientDataIterator(ClientUserData &data, Client *client)
    {
        p = ClientDataIteratorPrivatePtr(new ClientDataIteratorPrivate(data.p, client));
    }

    ClientDataIterator::~ClientDataIterator()
    {
    }

    IMContact *ClientDataIterator::operator ++()
    {
        return (IMContact*)(++(*p));
    }

    void ClientDataIterator::reset()
    {
        p->reset();
    }

    Client *ClientDataIterator::client()
    {
        return p->m_lastClient;
    }
}

// vim: set expandtab:

