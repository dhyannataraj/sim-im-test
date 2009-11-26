
#include <map>
#include "contacts.h"
#include "userdata.h"
#include "contactlistprivate.h"

namespace SIM
{
    typedef QMap<unsigned, SIM::Data*> UserDataMap;

    class UserDataPrivate
    {
    public:
        UserDataMap m_userData;
    };

    UserData_old::UserData_old()
    {
        d = new UserDataPrivate;
    }

    UserData_old::~UserData_old()
    {
        UserDataMap::Iterator userDataIt;
        for(userDataIt = d->m_userData.begin(); userDataIt != d->m_userData.end(); ++userDataIt) {
            std::map<unsigned long, UserDataDef>::iterator it = getContacts()->p->userDataDef.find(userDataIt.key());
            if (it != getContacts()->p->userDataDef.end())
                free_data(it->second.def, *userDataIt);
            delete[] userDataIt.value();
        }
        delete d;
    }

    void *UserData_old::getUserData(unsigned id, bool bCreate)
    {
        UserDataMap::Iterator userDataIt = d->m_userData.find(id);
        if (userDataIt != d->m_userData.end())
            return *userDataIt;
        if (!bCreate)
            return NULL;

        std::map<unsigned long, UserDataDef>::iterator it = getContacts()->p->userDataDef.find(id);
        if (it == getContacts()->p->userDataDef.end())
            return NULL;

        size_t size = 0;
        for (const DataDef *def = it->second.def; def->name; ++def)
            size += def->n_values;

        Data* data = new Data[size];
        d->m_userData.insert(id, data);
        load_data(it->second.def, data, NULL);
        return data;
    }

    void UserData_old::freeUserData(unsigned id)
    {
        UserDataMap::Iterator userDataIt = d->m_userData.find(id);
        if (userDataIt != d->m_userData.end()) {
            std::map<unsigned long, UserDataDef>::iterator it = getContacts()->p->userDataDef.find(id);
            if (it != getContacts()->p->userDataDef.end())
                free_data(it->second.def, d->m_userData[id]);
            delete[] userDataIt.value();
            d->m_userData.erase(userDataIt);
        }
    }

    QByteArray UserData_old::save() const
    {
        QByteArray res;
        UserDataMap::Iterator userDataIt;
        for(userDataIt = d->m_userData.begin(); userDataIt != d->m_userData.end(); ++userDataIt) {
            std::map<unsigned long, UserDataDef>::iterator it = getContacts()->p->userDataDef.find(userDataIt.key());
            if (it != getContacts()->p->userDataDef.end()){
                QByteArray cfg = save_data(it->second.def, userDataIt.value());
                if (cfg.length()){
                    if (res.length())
                        res += '\n';
                    res += '[';
                    res += it->second.name.toUtf8();
                    res += "]\n";
                    res += cfg;
                }
            }
        }
        return res;
    }

    void UserData_old::load(unsigned long id, const DataDef *def, Buffer *cfg)
    {
        void *d = getUserData(id, true);
        if (d == NULL)
            return;
        free_data(def, d);
        load_data(def, d, cfg);
    }

    UserData::UserData()
    {
    }

    UserData::~UserData()
    {
    }

    PropertyHubPtr UserData::getUserData(const QString& id)
    {
        DataMap::const_iterator it = m_data.find(id);
        if(it != m_data.end())
            return it.value();
        return PropertyHubPtr();
    }

    PropertyHubPtr UserData::createUserData(const QString& id)
    {
        PropertyHubPtr hub = PropertyHubPtr(new PropertyHub());
        m_data.insert(id, hub);
        return hub;
    }

    void UserData::destroyUserData(const QString& id)
    {
        DataMap::iterator it = m_data.find(id);
        if(it != m_data.end())
            m_data.erase(it);
    }

    QByteArray UserData::serialize()
    {
        return QByteArray();
    }

    bool UserData::deserialize(const QByteArray& arr)
    {
        return false;
    }

    void UserData::setNamespace(const QString& ns)
    {
        m_namespace = ns;
    }

    QString UserData::getNamespace()
    {
        return m_namespace;
    }
}

// vim: set expandtab:

