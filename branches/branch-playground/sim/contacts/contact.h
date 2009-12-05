
#ifndef SIM_CONTACT_H
#define SIM_CONTACT_H

#include "userdata.h"
#include "propertyhub.h"
#include "clientuserdata.h"
#include "misc.h"

namespace SIM
{
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

        int getGroup();
        void setGroup(int g);

        QString getName();
        void setName(const QString& s);

        bool getIgnore();
        void setIgnore(bool i);

        int getLastActive();
        void setLastActive(int la);

        QString getEMails();
        void setEMails(const QString& e);

        QString getPhones();
        void setPhones(const QString& p);

        int getPhoneStatus();
        void setPhoneStatus(int ps);

        QString getFirstName();
        void setFirstName(const QString& n);

        QString getLastName();
        void setLastName(const QString& n);

        QString getNotes();
        void setNotes(const QString& n);

        int getFlags();
        void setFlags(int flags);

        QString getEncoding();
        void setEncoding(const QString& enc);

        void *getUserData_old(unsigned id, bool bCreate = false) SIM_DEPRECATED;
        ClientUserData clientData;
        bool setFirstName(const QString &name, const QString &client);
        bool setLastName(const QString &name, const QString &client);
        bool setEMails(const QString &mails, const QString &client);
        bool setPhones(const QString &phones, const QString &client);
        unsigned long contactInfo(unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
        QString tipText();
        const DataDef *dataDef();
        void setup();
        PropertyHubPtr userdata() const { return m_userData->root(); }

        UserData_old& getUserData_old() { return userData; }
        UserDataPtr getUserData() { return m_userData; }
    protected:
        unsigned long m_id;
        friend class ContactList;
        friend class ContactListPrivate;

    private:
        UserData_old userData;
        UserDataPtr m_userData; // FIXME this mess
    };

}

#endif

// vim: set expandtab:

