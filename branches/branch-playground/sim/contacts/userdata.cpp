
#include <map>
#include <QTextStream>
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
        m_root = PropertyHub::create();
    }

    UserData::~UserData()
    {
    }

    PropertyHubPtr UserData::getUserData(const QString& id)
    {
        if(id.isEmpty())
            return m_root;
        DataMap::const_iterator it = m_data.find(id);
        if(it != m_data.end())
            return it.value();
        return PropertyHubPtr();
    }

    PropertyHubPtr UserData::createUserData(const QString& id)
    {
        if(id.isEmpty())
            return m_root;
        PropertyHubPtr hub = PropertyHub::create(id);
        m_data.insert(id, hub);
        return hub;
    }

    void UserData::destroyUserData(const QString& id)
    {
        DataMap::iterator it = m_data.find(id);
        if(it != m_data.end())
            m_data.erase(it);
    }

    PropertyHubPtr UserData::root()
    {
        return m_root;
    }

    bool UserData::serialize(QDomElement el)
    {
        QDomElement root = el.ownerDocument().createElement("userdata");
        el.appendChild(root);
        QDomElement roothub = el.ownerDocument().createElement("propertyhub");
        m_root->serialize(roothub);
        root.appendChild(roothub);
        foreach(PropertyHubPtr hub, m_data)
        {
            QDomElement hubelement = el.ownerDocument().createElement("propertyhub");
            hubelement.setAttribute("name", hub->getNamespace());
            hub->serialize(hubelement);
            root.appendChild(hubelement);
        }
        return true;
    }

    bool UserData::deserialize(QDomElement el)
    {
        QDomElement root = el.elementsByTagName("userdata").at(0).toElement();
        if(root.isNull())
            return false;
        QDomNodeList list = root.elementsByTagName("propertyhub");
        for(int i = 0; i < list.size(); i++)
        {
            QByteArray array;
            QTextStream stream(&array);
            QDomNode child = list.at(i);
            QDomElement el = child.firstChildElement("root");
            el.save(stream, 1);
            QString hubname = child.toElement().attribute("name");
            PropertyHubPtr hub;
            hub = createUserData(hubname);
            if(!hub->deserialize(el))
                return false;
        }
        return true;
    }

    UserDataPtr UserData::create()
    {
        return UserDataPtr(new UserData());
    }

}

// vim: set expandtab:

