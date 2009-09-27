
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

    UserData::UserData()
    {
        d = new UserDataPrivate;
    }

    UserData::~UserData()
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

    void *UserData::getUserData(unsigned id, bool bCreate)
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

    void UserData::freeUserData(unsigned id)
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

    QByteArray UserData::save() const
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

    void UserData::load(unsigned long id, const DataDef *def, Buffer *cfg)
    {
        void *d = getUserData(id, true);
        if (d == NULL)
            return;
        free_data(def, d);
        load_data(def, d, cfg);
    }
}

// vim: set expandtab:

