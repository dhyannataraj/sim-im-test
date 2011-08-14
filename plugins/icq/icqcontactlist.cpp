#include "icqcontactlist.h"
#include "log.h"

using SIM::log;
using SIM::L_DEBUG;

ICQContactList::ICQContactList(ICQClient* client) : m_client(client)
{
}

void ICQContactList::addContact(const ICQContactPtr& contact)
{
    //log(L_DEBUG, "ICQContactList::addContact(%s)", qPrintable(contact->getScreen()));
    m_contacts.insert(contact->getScreen(), contact);
}

ICQContactPtr ICQContactList::contact(int icqContactId)
{
    for(QMap<QString, ICQContactPtr>::iterator it = m_contacts.begin(); it != m_contacts.end(); ++it)
    {
        if(it.value()->getIcqID() == (unsigned)icqContactId)
            return it.value();
    }
    return ICQContactPtr();
}

ICQContactPtr ICQContactList::contactByScreen(const QString& screen)
{
    return m_contacts.value(screen);
}

int ICQContactList::contactCount() const
{
    return m_contacts.count();
}

QList<ICQContactPtr> ICQContactList::allContacts() const
{
    return m_contacts.values();
}

void ICQContactList::addGroup(const ICQGroupPtr& group)
{
    m_groups.insert(group->icqId(), group);
}

ICQGroupPtr ICQContactList::group(int icqGroupId)
{
    return m_groups.value(icqGroupId);
}

int ICQContactList::groupCount() const
{
    return m_groups.count();
}
