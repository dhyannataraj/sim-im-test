
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
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

    Contact::Contact(unsigned long id, Buffer *cfg)
        : m_id(id)
    {
        //load_data(contactData, &data, cfg);
        m_userdata = PropertyHub::create("");
        m_userdata->setValue("id", (uint)id);
        m_userData = UserData::create();
    }

    Contact::~Contact()
    {
        if (!getContacts()->p->bNoRemove){
            EventContact e(this, EventContact::eDeleted);
            e.process();
        }
        //free_data(contactData, &data);
        getContacts()->p->contacts.erase(m_id);
    }

    const DataDef *Contact::dataDef()
    {
        return contactData;
    }

    void *Contact::getUserData_old(unsigned id, bool bCreate)
    {
        void *res = userData.getUserData(id, bCreate);
        if (res)
            return res;
        if (bCreate)
            return userData.getUserData(id, true);
        Group *group = getContacts()->group(getGroup());
        if (group)
            return group->getUserData_old(id, false);
        return getContacts()->getUserData_old(id);
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

    int Contact::getGroup()
    {
        return userdata()->value("Group").toInt();
    }

    void Contact::setGroup(int g)
    {
        userdata()->setValue("Group", g);
    }

    QString Contact::getName()
    {
        return userdata()->value("Name").toString();
    }

    void Contact::setName(const QString& s)
    {
        userdata()->setValue(s, "Name");
    }

    bool Contact::getIgnore()
    {
        return userdata()->value("Ignore").toBool();
    }

    void Contact::setIgnore(bool i)
    {
        userdata()->setValue("Ignore", i);
    }

    int Contact::getLastActive()
    {
        return userdata()->value("LastActive").toInt();
    }

    void Contact::setLastActive(int la)
    {
        userdata()->setValue("LastActive", la);
    }

    QString Contact::getEMails()
    {
        return userdata()->value("EMails").toString();
    }

    void Contact::setEMails(const QString& e)
    {
        userdata()->setValue("EMails", e);
    }

    QString Contact::getPhones()
    {
        return userdata()->value("Phones").toString();
    }

    void Contact::setPhones(const QString& p)
    {
        userdata()->setValue("Phones", p);
    }

    int Contact::getPhoneStatus()
    {
        return userdata()->value("PhoneStatus").toInt();
    }

    void Contact::setPhoneStatus(int ps)
    {
        userdata()->setValue("PhoneStatus", ps);
    }

    QString Contact::getFirstName()
    {
        return userdata()->value("FirstName").toString();
    }

    void Contact::setFirstName(const QString& n)
    {
        userdata()->setValue("FirstName", n);
    }

    QString Contact::getLastName()
    {
        return userdata()->value("LastName").toString();
    }

    void Contact::setLastName(const QString& n)
    {
        userdata()->setValue("LastName", n);
    }

    QString Contact::getNotes()
    {
        return userdata()->value("Notes").toString();
    }
    
    void Contact::setNotes(const QString& n)
    {
        userdata()->setValue("Notes", n);
    }

    int Contact::getFlags()
    {
        return userdata()->value("Flags").toInt();
    }

    void Contact::setFlags(int flags)
    {
        userdata()->setValue("Flags", flags);
    }

    QString Contact::getEncoding()
    {
        return userdata()->value("Encoding").toString();
    }

    void Contact::setEncoding(const QString& enc)
    {
        userdata()->setValue("Encoding", enc);
    }
}

// vim: set expandtab:

