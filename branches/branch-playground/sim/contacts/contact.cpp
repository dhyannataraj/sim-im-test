
#include "contact.h"
#include "contacts.h"
#include "clientdataiterator.h"
#include "contactlistprivate.h"
#include "group.h"
#include "client.h"

namespace SIM
{

    DataDef contactData[] =
    {
        { "Group", DATA_ULONG, 1, 0 },
        { "Name", DATA_UTF, 1, 0 },
        { "Ignore", DATA_BOOL, 1, 0 },
        { "LastActive", DATA_ULONG, 1, 0 },
        { "EMails", DATA_UTF, 1, 0 },
        { "Phones", DATA_UTF, 1, 0 },
        { "PhoneStatus", DATA_ULONG, 1, 0 },
        { "FirstName", DATA_UTF, 1, 0 },
        { "LastName", DATA_UTF, 1, 0 },
        { "Notes", DATA_UTF, 1, 0 },
        { "Flags", DATA_ULONG, 1, 0 },
#ifdef __OS2__        
        { "Encoding", DATA_STRING, 1, getDefEncoding() },
#else
        { "Encoding", DATA_STRING, 1, 0 },
#endif        
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

    Contact::Contact(unsigned long id, Buffer *cfg)
        : m_id(id)
    {
        load_data(contactData, &data, cfg);
        m_userdata = new QVariantMap();
        m_userdata->insert("id", (uint)id);
    }

    Contact::~Contact()
    {
        if (!getContacts()->p->bNoRemove){
            EventContact e(this, EventContact::eDeleted);
            e.process();
        }
        free_data(contactData, &data);
        getContacts()->p->contacts.erase(m_id);
        delete m_userdata;
    }

    const DataDef *Contact::dataDef()
    {
        return contactData;
    }

    void *Contact::getUserData(unsigned id, bool bCreate)
    {
        void *res = userData.getUserData(id, bCreate);
        if (res)
            return res;
        if (bCreate)
            return userData.getUserData(id, true);
        Group *group = getContacts()->group(getGroup());
        if (group)
            return group->getUserData(id, false);
        return getContacts()->getUserData(id);
    }

    void Contact::setup()
    {
        QString str = getFirstName();
        getToken(str, '/');
        if (str != "-")
            setFirstName(QString());
        str = getLastName();
        getToken(str, '/');
        if(!str.contains('-'))
            setLastName(QString());
        QString res;
        str = getEMails();
        while (!str.isEmpty()){
            QString item = getToken(str, ';', false);
            QString value = getToken(item, '/', false);
            if (item != "-")
                continue;
            if (!res.isEmpty())
                res += ';';
            res += value;
            res += "/-";
        }
        setEMails(res);
        str = getPhones();
        while (!str.isEmpty()){
            QString item = getToken(str, ';', false);
            QString value = getToken(item, '/', false);
            if (item != "-")
                continue;
            if (!res.isEmpty())
                res += ';';
            res += value;
            res += "/-";
        }
        setPhones(res);
        ClientDataIterator it(clientData);
        void *data;
        while ((data = ++it) != NULL)
            it.client()->setupContact(this, data);
    }
}

// vim: set expandtab:

