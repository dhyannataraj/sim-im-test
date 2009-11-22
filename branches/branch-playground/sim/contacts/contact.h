
#ifndef SIM_CONTACT_H
#define SIM_CONTACT_H

#include "userdata.h"
#include "clientuserdata.h"

namespace SIM
{
    struct ContactData
    {
        Data            Group;      // Group ID
        Data            Name;       // Contact Display Name (UTF-8)
        Data            Ignore;     // In ignore list
        Data            LastActive;
        Data            EMails;
        Data            Phones;
        Data            PhoneStatus;
        Data            FirstName;
        Data            LastName;
        Data            Notes;
        Data            Flags;
        Data            Encoding;
    };

    const unsigned CONTACT_TEMP             = 0x0001;
    const unsigned CONTACT_DRAG             = 0x0002;
    const unsigned CONTACT_NOREMOVE_HISTORY = 0x1000;

    const unsigned CONTACT_TEMPORARY    = CONTACT_TEMP | CONTACT_DRAG;

    class EXPORT Contact
    {
    public:
        Contact(unsigned long id = 0, Buffer *cfg = NULL);
        virtual ~Contact();
        unsigned long id() const { return m_id; }
        PROP_ULONG(Group)
        PROP_UTF8(Name)
        PROP_BOOL(Ignore)
        PROP_ULONG(LastActive)
        PROP_UTF8(EMails)
        PROP_UTF8(Phones)
        PROP_ULONG(PhoneStatus)
        PROP_UTF8(FirstName)
        PROP_UTF8(LastName)
        PROP_UTF8(Notes)
        PROP_ULONG(Flags)
        PROP_STR(Encoding)
        void *getUserData(unsigned id, bool bCreate = false);
        ClientUserData clientData;
        bool setFirstName(const QString &name, const QString &client);
        bool setLastName(const QString &name, const QString &client);
        bool setEMails(const QString &mails, const QString &client);
        bool setPhones(const QString &phones, const QString &client);
        unsigned long contactInfo(unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
        QString tipText();
        ContactData data;
        const DataDef *dataDef();
        void setup();
        QVariantMap* userdata() const { return m_userdata; }

        UserData& getUserData() { return userData; }
    protected:
        unsigned long m_id;
        friend class ContactList;
        friend class ContactListPrivate;

    private:
        UserData userData;
        QVariantMap* m_userdata;
    };

}

#endif

// vim: set expandtab:

