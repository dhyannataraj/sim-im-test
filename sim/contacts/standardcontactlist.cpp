#include <QDir>
#include <QStringList>
#include "standardcontactlist.h"
#include "profilemanager.h"
#include "contacts/client.h"
#include "clientmanager.h"
#include "log.h"
#include "events/eventhub.h"
#include "events/standardevent.h"
#include "events/contactevent.h"

namespace SIM {

StandardContactList::StandardContactList()
{
    getEventHub()->registerEvent(SIM::StandardEvent::create("contacts_loaded"));
    getEventHub()->registerEvent(SIM::StandardEvent::create("contact_list_updated"));
    getEventHub()->registerEvent(SIM::ContactEvent::create("contact_change_status"));
    m_userData = UserData::create();
}


void StandardContactList::clear()
{
}

bool StandardContactList::load()
{
    log(L_DEBUG, "ContactList::load()");
    if(!load_new())
        return load_old();
    return true;
}

bool StandardContactList::sync()
{
    if(getProfileManager()->currentProfile().isNull())
        return false;

    getProfileManager()->sync();

    config()->rootHub()->addPropertyHub(m_userData->saveState());

    save_owner();
    save_groups();
    save_contacts();

    return config()->writeToFile();
}

bool StandardContactList::addContact(const ContactPtr& newContact)
{
    ContactPtr c = contact(newContact->id());
    if(c)
        return false;
    m_contacts.insert(newContact->id(), newContact);
    return true;
}

ContactPtr StandardContactList::contact(int id) const
{
    QMap<int, ContactPtr>::const_iterator it = m_contacts.find(id);
    if(it == m_contacts.end())
        return ContactPtr();
    return it.value();
}

bool StandardContactList::contactExists(int id) const
{
    ContactPtr c = contact(id);
    if(c)
        return true;
    return false;
}

void StandardContactList::removeContact(int id)
{
    QMap<int, ContactPtr>::iterator it = m_contacts.find(id);
    if(it != m_contacts.end())
        m_contacts.erase(it);
}

ContactPtr StandardContactList::createContact()
{
    int id = 0;
    QList<int> ids = contactIds();
    if(ids.size() > 0)
    {
        qSort(ids.begin(), ids.end(), qGreater<int>());
        id = ids.at(0) + 1;
    }
    return ContactPtr(new Contact(id));
}

ContactPtr StandardContactList::createContact(int id)
{
    return ContactPtr(new Contact(id));
}

QList<int> StandardContactList::contactIds() const
{
    QList<int> result;
    foreach(const ContactPtr& contact, m_contacts)
    {
        if(contact->flag(Contact::flIgnore))
            continue;
        result.append(contact->id());
    }
    return result;
}

bool StandardContactList::addGroup(const GroupPtr& newGroup)
{
    GroupPtr g = group(newGroup->id());
    if(g)
        return false;
    m_groups.insert(newGroup->id(), newGroup);
    return true;
}

GroupPtr StandardContactList::group(int id) const
{
    QMap<int, GroupPtr>::const_iterator it = m_groups.find(id);
    if(it == m_groups.end())
        return GroupPtr();
    return it.value();
}

void StandardContactList::removeGroup(int id)
{
    QMap<int, GroupPtr>::iterator it = m_groups.find(id);
    if(it != m_groups.end())
        m_groups.erase(it);
}

GroupPtr StandardContactList::createGroup(int id)
{
    return GroupPtr(new Group(id));
}

ContactPtr StandardContactList::ownerContact()
{
    return m_owner;
}

QList<GroupPtr> StandardContactList::allGroups() const
{
    return m_groups.values();
}

QList<ContactPtr> StandardContactList::contactsForGroup(int groupId)
{
    QList<ContactPtr> result;
    foreach(const ContactPtr& contact, m_contacts)
    {
        if(contact->groupId() == groupId)
            result.append(contact);
    }

    return result;
}

void StandardContactList::incomingMessage(const MessagePtr& message)
{
    Q_UNUSED(message);
}

UserDataPtr StandardContactList::userdata() const
{
    return m_userData;
}

bool StandardContactList::save_owner()
{
    PropertyHubPtr ownerHub = PropertyHub::create("owner");
    config()->rootHub()->addPropertyHub(ownerHub);

    // code

    return true;
}

bool StandardContactList::save_groups()
{
    PropertyHubPtr groupsHub = PropertyHub::create("groups");
    config()->rootHub()->addPropertyHub(groupsHub);

    for (auto it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        groupsHub->addPropertyHub(it.value()->saveState());
    }

    return true;
}

bool StandardContactList::save_contacts()
{
    PropertyHubPtr contactsHub = PropertyHub::create("contacts");
    config()->rootHub()->addPropertyHub(contactsHub);

    for(QMap<int, ContactPtr>::iterator it = m_contacts.begin(); it != m_contacts.end(); ++it)
    {
        PropertyHubPtr curContactsHub = it.value()->saveState();
        contactsHub->addPropertyHub(curContactsHub);
    }
    return true;
}

bool StandardContactList::load_new()
{
    PropertyHubPtr userDataHub = config()->rootHub()->propertyHub("userdata");
    if (userDataHub.isNull())
        return false;
    m_userData->loadState(userDataHub);

    if (!load_owner())
        return false;

    if(!load_groups())
        return false;

    if(!load_contacts())
        return false;

    return true;
}

bool StandardContactList::load_owner()
{
    PropertyHubPtr ownerHub = config()->rootHub()->propertyHub("owner");
    if (ownerHub.isNull())
        return false;

    // code

    return true;
}

bool StandardContactList::load_groups()
{
    PropertyHubPtr groupsHub = config()->rootHub()->propertyHub("groups");
    if (groupsHub.isNull())
        return false;

    QStringList groupList = groupsHub->propertyHubNames();
    foreach(QString groupID , groupList)
    {
        GroupPtr gr = createGroup(groupID.toInt());
        if (!gr->loadState(groupsHub->propertyHub(groupID)))
            return false;
        addGroup(gr);
    }

    return true;
}

bool StandardContactList::load_contacts()
{
    PropertyHubPtr contactsHub = config()->rootHub()->propertyHub("contacts");
    if (contactsHub.isNull())
        return false;

    QStringList contactsList = contactsHub->propertyHubNames();
    foreach(QString contactID , contactsList)
    {
        ContactPtr c = createContact(contactID.toInt());
        if (!c->loadState(contactsHub->propertyHub(contactID)))
            return false;
        addContact(c);
    }
    return true;
}

bool StandardContactList::load_old()
{
    QString cfgName = getProfileManager()->profilePath() + QDir::separator() + "contacts.conf";
    QFile f(cfgName);
    if (!f.open(QIODevice::ReadOnly)){
        log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
        return false;
    }

    ParserState state;
    while(!f.atEnd())
    {
        QString line = QString::fromLocal8Bit(f.readLine());
        line = line.trimmed();
        //log(L_DEBUG, "Line: %s", qPrintable(line));
        if(line.startsWith("[Group="))
        {
            state.nextSection = ParserState::Group;
            load_old_dispatch(state);
            resetState(state);
            state.groupId = line.mid(7, line.length() - 8).toInt();
            if(state.groupId > 0)
                addGroup(createGroup(state.groupId));
        }
        else if(line.startsWith("[Contact="))
        {
            state.nextSection = ParserState::Contact;
            load_old_dispatch(state);
            resetState(state);
            state.contactId = line.mid(9, line.length() - 10).toInt();
            if(state.contactId > 0)
                addContact(createContact(state.contactId));
        }
        else if(line.startsWith("["))
        {
            state.nextSection = ParserState::Client;
            load_old_dispatch(state);
            state.dataname = line.mid(1, line.length() - 2);
        }
        else
        {
            state.data.append(line + "\n");
        }
    }
    return true;
}

bool StandardContactList::load_old_dispatch(ParserState& state)
{
    if(state.contactId != -1)
    {
        ContactPtr c = contact(state.contactId);
        if(!c)
            return false;
        if(state.dataname.isEmpty())
        {
            c->deserialize(state.data);
        }
        else
        {
            if(state.dataname.indexOf('.') >= 0)
            {
                ClientPtr client = getClientManager()->client(state.dataname);
                if(!client)
                    return false;
                IMContactPtr imcontact = client->createIMContact();
                imcontact->setParentContactId(state.contactId);
                imcontact->deserialize(state.data);
                c->addClientContact(imcontact);
                if(c->name().isEmpty())
                    c->setName(imcontact->name());
                client->addIMContact(imcontact);
            }
            else if(!state.dataname.isEmpty())
            {
                deserializeLines(c->userdata(), state.dataname, state.data);
            }
            else
            {
                c->deserialize(state.data);
            }
        }
    }
    else if(state.groupId != -1)
    {
        GroupPtr gr = group(state.groupId);
        if(!gr)
            return false;
        if(state.dataname.isEmpty())
        {
            gr->deserialize(state.data);
        }
        else
        {
            if(state.dataname.indexOf('.') >= 0)
            {
                ClientPtr client = getClientManager()->client(state.dataname);
                if(!client)
                    return false;
                IMGroupPtr imgroup = client->createIMGroup();
                imgroup->deserialize(state.data);
                gr->addClientGroup(imgroup);
            }
            else
            {
                deserializeLines(gr->userdata(), state.dataname, state.data);
            }
        }
    }

    return true;
}

void StandardContactList::resetState(ParserState& state)
{
    state.contactId = -1;
    state.groupId = -1;
    state.dataname = QString();
    state.data.clear();
}

bool StandardContactList::deserializeLines(const UserDataPtr& ud, const QString& dataname, const QString& data)
{
    PropertyHubPtr hub = ud->getUserData(dataname);
    if(!hub)
        hub = ud->createUserData(dataname);
    QStringList list = data.split('\n');
    foreach(const QString& s, list)
    {
        QStringList keyval = s.split('=');
        if(keyval.size() != 2)
            continue;
        if(keyval.at(1).startsWith('\"') && keyval.at(1).endsWith('\"') )
            hub->setValue(keyval.at(0), keyval.at(1).mid(1, keyval.at(1).size() - 2));
        else
            hub->setValue(keyval.at(0), keyval.at(1));
    }

    return true;
}

ConfigPtr StandardContactList::config()
{
    if (!m_config.isNull() && m_loadedProfile == getProfileManager()->currentProfileName())
        return m_config;

    m_loadedProfile = getProfileManager()->currentProfileName();
    QString cfgName = getProfileManager()->profilePath() + QDir::separator() + "contacts.xml";
    m_config = ConfigPtr(new Config(cfgName));

    m_config->readFromFile();

    return m_config;
}

} // namespace SIM
