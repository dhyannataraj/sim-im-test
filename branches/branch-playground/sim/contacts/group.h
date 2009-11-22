
#ifndef SIM_GROUP_H
#define SIM_GROUP_H

#include "simapi.h"
#include "userdata.h"
#include "clientuserdata.h"
#include "propertyhub.h"

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
        ClientUserData clientData;
        PropertyHub* userdata() const { return m_userdata; }
        UserData& getUserData() { return userData; }

    protected:
        unsigned long m_id;
        GroupData data; friend class ContactList;
        friend class ContactListPrivate;

    private:
        UserData userData;
        PropertyHub* m_userdata;
    };
}

#endif

// vim: set expandtab:

