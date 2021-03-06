
#ifndef SIM_CONTACTLISTPRIVATE_H
#define SIM_CONTACTLISTPRIVATE_H

#include <QString>
#include <map>
#include <vector>
#include <list>
#include "group.h"
#include "contact.h"

#include "contacts/packettype.h"
#include "contacts/userdata.h"
#include "contacts.h"

namespace SIM
{
    typedef std::map<unsigned, PacketType*>	PACKET_MAP;
    // Yes, it's a private implementation of ContactList.
    // But UserData USES it. So, I had to make this header.
    class ContactListPrivate
    {
    public:
        ContactListPrivate();
        ~ContactListPrivate();
        void clear(bool bClearAll);
        unsigned registerUserData(const QString &name, const DataDef *def);
        void unregisterUserData(unsigned id);
        void flush(Contact *c, Group *g, const QByteArray &section, Buffer *cfg);
        void flush(Contact *c, Group *g);
        std::map<unsigned long, UserDataDef> userDataDef;
        Contact			*m_owner;
        std::map<unsigned long, Contact*>  contacts;
        std::vector<Group*>  groups;
        std::vector<Client*> clients;
        QList<Protocol*>     protocols;
        PACKET_MAP           packets;
        bool                 m_bNoRemove;
		Group				*m_notInList;
    };
}

#endif

// vim: set expandtab:

