
#ifndef SIM_GROUP_H
#define SIM_GROUP_H

#include "simapi.h"
#include "userdata.h"
#include "clientuserdata.h"

namespace SIM
{

    struct GroupData
    {
        Data        Name;       // Display name (UTF-8)
    };
    class EXPORT Group
    {
    public:
        Group(unsigned long id = 0, Buffer *cfg = NULL);
        virtual ~Group();
        unsigned long id() { return m_id; }
        PROP_UTF8(Name)
        void *getUserData(unsigned id, bool bCreate = false);
        UserData userData;
        ClientUserData clientData;
        QVariantMap* userdata() const { return m_userdata; }

    protected:
        unsigned long m_id;
        GroupData data; friend class ContactList;
        friend class ContactListPrivate;

    private:
        QVariantMap* m_userdata;
    };
}

#endif

// vim: set expandtab:

