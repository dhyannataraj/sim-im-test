/***************************************************************************
                          contacts.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"

#include <list>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

#include <qfile.h>

namespace SIM
{

typedef map<unsigned, PacketType*>	PACKET_MAP;

class ContactListPrivate
{
public:
    ContactListPrivate();
    ~ContactListPrivate();
    void clear(bool bClearAll);
    unsigned registerUserData(const char *name, const DataDef *def);
    void unregisterUserData(unsigned id);
    void flush(Contact *c, Group *g, const char *section, const char *cfg);
    void flush(Contact *c, Group *g);
    UserData userData;
    list<UserDataDef> userDataDef;
    Contact			*owner;
    list<Contact*>  contacts;
    vector<Group*>  groups;
    vector<Client*> clients;
    list<Protocol*> protocols;
    PACKET_MAP		packets;
    bool			bNoRemove;
};

/*
typedef struct ContactData
{
    unsigned long	Group;		// Group ID
    char			*Name;		// Contact Display Name (UTF-8)
	unsigned long	Ignore;		// In ignore list
	unsigned long	LastActive;
	char			*EMails;
	char			*Phones;
	char			*FirstName;
	char			*LastName;
	char			*Notes;
	unsigned long	Temp;
} ContactData;
*/

static DataDef contactData[] =
    {
        { "Group", DATA_LONG, 1, 0 },
        { "Name", DATA_UTF, 1, 0 },
        { "Ignore", DATA_BOOL, 1, 0 },
        { "LastActive", DATA_ULONG, 1, 0 },
        { "EMails", DATA_UTF, 1, 0 },
        { "Phones", DATA_UTF, 1, 0 },
        { "PhoneStatus", DATA_ULONG, 1, 0 },
        { "FirstName", DATA_UTF, 1, 0 },
        { "LastName", DATA_UTF, 1, 0 },
        { "Notes", DATA_UTF, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Contact::Contact(unsigned long id, const char *cfg)
{
    m_id = id;
    load_data(contactData, &data, cfg);
}

Contact::~Contact()
{
    if (!getContacts()->p->bNoRemove){
        Event e(EventContactDeleted, this);
        e.process();
    }
    free_data(contactData, &data);
    list<Contact*> &contacts = getContacts()->p->contacts;
    for (list<Contact*>::iterator it = contacts.begin(); it != contacts.end(); ++it){
        if ((*it) == this){
            contacts.erase(it);
            break;
        }
    }
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
        setFirstName(NULL);
    str = getLastName();
    getToken(str, '/');
    if (str != "-")
        setLastName(NULL);
    QString res;
    str = getEMails();
    while (!str.isEmpty()){
        QString item = getToken(str, ';', false);
        QString value = getToken(item, '/', false);
        if (item != "-")
            continue;
        if (!res.isEmpty())
            res += ";";
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
            res += ";";
        res += value;
        res += "/-";
    }
    setPhones(res);
    ClientDataIterator it(clientData);
    void *data;
    while ((data = ++it) != NULL)
        it.client()->setupContact(this, data);
}

typedef list<string> PROTO_LIST;

typedef struct STR_ITEM
{
    QString		value;
    PROTO_LIST	proto;
} STR_ITEM;

typedef list<STR_ITEM> STR_LIST;

static void add_str(STR_LIST &m, const QString &value, const char *client)
{
    STR_LIST::iterator it;
    for (it = m.begin(); it != m.end(); ++it){
        QString v = (*it).value;
        if (v == value)
            break;
    }
    if (it != m.end()){
        PROTO_LIST &proto = (*it).proto;
        PROTO_LIST::iterator itp;
        if (!proto.empty() && !strcmp(client, "-"))
            return;
        for (itp = proto.begin(); itp != proto.end(); ++itp){
            if (!strcmp((*itp).c_str(), client))
                return;
        }
        for (itp = proto.begin(); itp != proto.end(); ++itp){
            if (!strcmp((*itp).c_str(), "-"))
                break;;
        }
        if (itp != proto.end())
            proto.erase(itp);
        proto.push_back(string(client));
        return;
    }
    STR_ITEM item;
    item.value = value;
    item.proto.push_back(client);
    m.push_back(item);
}

static QString addStrings(const QString &old_value, const QString &values, const char *client)
{
    STR_LIST str_list;
    QString m = old_value;
    while (m.length()){
        QString str_item = getToken(m, ';', false);
        QString str = getToken(str_item, '/');
        while (str_item.length()){
            QString proto = getToken(str_item, ',');
            if (client && (proto == client))
                continue;
            add_str(str_list, str, proto.latin1());
        }
    }
    if (client){
        for (STR_LIST::iterator it = str_list.begin(); it != str_list.end(); ++it){
            PROTO_LIST &proto = (*it).proto;
            PROTO_LIST::iterator itp;
            for (itp = proto.begin(); itp != proto.end(); ++itp)
                if (!strcmp((*itp).c_str(), client))
                    break;
            if (itp != proto.end())
                proto.erase(itp);
        }
    }
    m = values;
    while (m.length()){
        QString v = getToken(m, ';', false);
        if (v.length() == 0)
            continue;
        add_str(str_list, v, client ? client : "-");
    }
    QString res;
    for (STR_LIST::iterator it = str_list.begin(); it != str_list.end(); ++it){
        PROTO_LIST &proto = (*it).proto;
        if (proto.size() == 0)
            continue;
        if (res.length())
            res += ";";
        res += quoteChars((*it).value, ";/");
        res += "/";
        QString proto_str;
        for (PROTO_LIST::iterator itp = proto.begin(); itp != proto.end(); ++itp){
            if (proto_str.length())
                proto_str += ",";
            proto_str += (*itp).c_str();
        }
        res += proto_str;
    }
    return res;
}

bool Contact::setEMails(const QString &mail, const char *client)
{
    return setEMails(addStrings(getEMails(), mail, client));
}

bool Contact::setPhones(const QString &phone, const char *client)
{
    return setPhones(addStrings(getPhones(), phone, client));
}

static QString packString(const QString &value, const char *client)
{
    QString res = quoteChars(value, "/");
    res += "/";
    if (client){
        res += client;
    }else{
        res += "-";
    }
    return res;
}

static QString addString(const QString &oldValue, const QString &newValue, const char *client)
{
    QString res;
    if (oldValue.length() == 0){
        if (newValue.length() == 0)
            return res;
        return packString(newValue, client);
    }
    QString value = oldValue;
    getToken(value, '/');
    if ((value == client) || (client == NULL))
        return packString(newValue, client);
    return oldValue;
}

bool Contact::setFirstName(const QString &name, const char *client)
{
    return setFirstName(addString(getFirstName(), name, client));
}

bool Contact::setLastName(const QString &name, const char *client)
{
    return setLastName(addString(getLastName(), name, client));
}

static char tipDiv[] = "<br>___________<br>";

QString Contact::tipText()
{
    QString tip;
    tip += "<b>";
    tip += quoteString(getName());
    tip += "</b>";
    QString firstName = getFirstName();
    firstName = getToken(firstName, '/');
    firstName = quoteString(firstName);
    QString lastName = getLastName();
    lastName = getToken(lastName, '/');
    lastName = quoteString(lastName);
    if (firstName.length() || lastName.length()){
        tip += "<br>";
        if (firstName.length()){
            tip += firstName;
            tip += " ";
        }
        tip += lastName;
    }
    bool bFirst = true;
    QString mails = getEMails();
    while (mails.length()){
        QString mail = getToken(mails, ';', false);
        mail = getToken(mail, '/');
        if (bFirst){
            tip += "<br>";
            bFirst = false;
        }else{
            tip += ", ";
        }
        tip += quoteString(mail);
    }
    void *data;
    ClientDataIterator it(clientData);
    while ((data = ++it) != NULL){
        Client *client = clientData.activeClient(data, it.client());
        if (client == NULL)
            continue;
        QString str = client->contactTip(data);
        if (str.length()){
            tip += tipDiv;
            tip += str;
        }
    }
    bFirst = true;
    QString phones = getPhones();
    while (phones.length()){
        if (bFirst){
            tip += tipDiv;
            bFirst = false;
        }else{
            tip += "<br>";
        }
        QString phone_item = getToken(phones, ';', false);
        phone_item = getToken(phone_item, '/', false);
        QString phone = getToken(phone_item, ',');
        getToken(phone_item, ',');
        unsigned phone_type = atol(phone_item.latin1());
        QString icon;
        switch (phone_type){
        case PHONE:
            icon = "phone";
            break;
        case FAX:
            icon = "fax";
            break;
        case CELLULAR:
            icon = "cell";
            break;
        case PAGER:
            icon = "pager";
            break;
        }
        if (icon.length()){
            tip += "<img src=\"icon:";
            tip += icon;
            tip += "\">";
        }
        tip += " ";
        tip += quoteString(phone);
    }
    return tip;
}

unsigned long Contact::contactInfo(unsigned &style, const char *&statusIcon, string *icons)
{
    style = 0;
    statusIcon = NULL;
    if (icons)
        *icons = "";
    unsigned long status = STATUS_UNKNOWN;
    void *data;
    ClientDataIterator it(clientData);
    while ((data = ++it) != NULL){
        Client *client = clientData.activeClient(data, it.client());
        if (client == NULL)
            continue;
        client->contactInfo(data, status, style, statusIcon, icons);
    }
    QString phones = getPhones();
    bool bCell  = false;
    bool bPager = false;
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        phoneItem = getToken(phoneItem, '/', false);
        getToken(phoneItem, ',');
        getToken(phoneItem, ',');
        unsigned n = atol(phoneItem.latin1());
        if (n == CELLULAR) bCell = true;
        if (n == PAGER) bPager = true;
    }
    if (bCell){
        if (statusIcon){
            if (icons){
                if (icons->length())
                    *icons += ',';
                *icons += "cell";
            }
        }else{
            statusIcon = "cell";
        }
    }
    if (bPager){
        if (statusIcon){
            if (icons){
                if (icons->length())
                    *icons += ',';
                *icons += "pager";
            }
        }else{
            statusIcon = "pager";
        }
    }
    if (status == STATUS_UNKNOWN){
        if (statusIcon == NULL){
            QString mails = getEMails();
            if (!mails.isEmpty())
                statusIcon = "mail_generic";
        }
        if (statusIcon == NULL)
            statusIcon = "nonim";
        return STATUS_NA;
    }
    if (statusIcon == NULL)
        statusIcon = "empty";
    return status;
}

QString Client::ownerName()
{
    return "";
}

QString Client::contactName(void*)
{
    return i18n(protocol()->description()->text);
}

CommandDef *Client::infoWindows(Contact*, void*)
{
    return NULL;
}

QWidget *Client::infoWindow(QWidget*, Contact*, void*, unsigned)
{
    return NULL;
}

CommandDef *Client::configWindows()
{
    return NULL;
}

QWidget *Client::configWindow(QWidget*, unsigned)
{
    return NULL;
}

QWidget *Client::searchWindow()
{
    return NULL;
}

void Client::contactInfo(void*, unsigned long&, unsigned&, const char*&, string*)
{
}

QString Client::contactTip(void*)
{
    return "";
}

void Client::updateInfo(Contact *contact, void *data)
{
    if (data){
        Event e(EventFetchInfoFail, contact);
        e.process();
    }else{
        Event e(EventClientChanged, this);
        e.process();
    }
}

/*
typedef struct GroupData
{
    char			*Name;		// Display name (UTF-8)
} GroupData;
*/
static DataDef groupData[] =
    {
        { "Name", DATA_UTF, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Group::Group(unsigned long id, const char *cfg)
{
    m_id = id;
    load_data(groupData, &data, cfg);
}

Group::~Group()
{
    if (!getContacts()->p->bNoRemove){
        Contact *contact;
        ContactList::ContactIterator itc;
        while ((contact = ++itc) != NULL){
            if (contact->getGroup() != id())
                continue;
            contact->setGroup(0);
            Event e(EventContactChanged, contact);
            e.process();
        }
        Event e(EventGroupDeleted, this);
        e.process();
    }
    free_data(groupData, &data);
    vector<Group*> &groups = getContacts()->p->groups;
    for (vector<Group*>::iterator it = groups.begin(); it != groups.end(); ++it){
        if ((*it) == this){
            groups.erase(it);
            break;
        }
    }
}

void *Group::getUserData(unsigned id, bool bCreate)
{
    void *res = userData.getUserData(id, bCreate);
    if (res)
        return res;
    if (bCreate)
        return userData.getUserData(id, true);
    return getContacts()->getUserData(id);
}

ContactListPrivate::ContactListPrivate()
{
    Group *notInList = new Group(0);
    owner = new Contact(0);
    groups.push_back(notInList);
    bNoRemove = false;
}

ContactListPrivate::~ContactListPrivate()
{
    clear(true);
    delete owner;
}

void ContactListPrivate::clear(bool bClearAll)
{
    bNoRemove = true;
    for (list<Contact*>::iterator it_c = contacts.begin(); it_c != contacts.end();){
        Contact *contact = *it_c;
        delete contact;
        it_c = contacts.begin();
    }
    for (vector<Group*>::iterator it_g = groups.begin(); it_g != groups.end();){
        Group *group = *it_g;
        if (!bClearAll && (group->id() == 0)){
            ++it_g;
            continue;
        }
        delete group;
        it_g = groups.begin();
    }
    bNoRemove = false;
}

unsigned ContactListPrivate::registerUserData(const char *name, const DataDef *def)
{
    unsigned id = 0;
    for (list<UserDataDef>::iterator it = userDataDef.begin(); it != userDataDef.end(); ++it){
        if (id <= (*it).id)
            id = (*it).id + 1;
    }
    UserDataDef d;
    d.id = id;
    d.name = name;
    d.def = def;
    userDataDef.push_back(d);
    return id;
}

void ContactListPrivate::unregisterUserData(unsigned id)
{
    for (list<Contact*>::iterator it_c = contacts.begin(); it_c != contacts.end(); ++it_c){
        (*it_c)->userData.freeUserData(id);
    }
    for (vector<Group*>::iterator it_g = groups.begin(); it_g != groups.end(); ++it_g)
        (*it_g)->userData.freeUserData(id);
    userData.freeUserData(id);
    for (list<UserDataDef>::iterator it = userDataDef.begin(); it != userDataDef.end(); ++it){
        if (id != (*it).id) continue;
        userDataDef.erase(it);
        break;
    }
}

ContactList::ContactList()
{
    p = new ContactListPrivate;
}

ContactList::~ContactList()
{
    delete p;
}

unsigned ContactList::registerUserData(const char *name, const DataDef *def)
{
    return p->registerUserData(name, def);
}

void ContactList::unregisterUserData(unsigned id)
{
    p->unregisterUserData(id);
}

Contact *ContactList::owner()
{
    return p->owner;
}

Contact *ContactList::contact(unsigned long id, bool isNew)
{
    list<Contact*>::iterator it;
    for (it = p->contacts.begin(); it != p->contacts.end(); ++it){
        if ((*it)->id() == id)
            return (*it);
    }
    if (!isNew)
        return NULL;
    if (id == 0){
        id = 1;
        for (it = p->contacts.begin(); it != p->contacts.end(); ++it){
            if (id <= (*it)->id())
                id = (*it)->id() + 1;
        }
    }
    Contact *res = new Contact(id);
    p->contacts.push_back(res);
    Event e(EventContactCreated, res);
    e.process();
    return res;
}

void ContactList::addContact(Contact *contact)
{
    if (contact->id())
        return;
    unsigned long id = 1;
    list<Contact*>::iterator it;
    for (it = p->contacts.begin(); it != p->contacts.end(); ++it){
        if (id <= (*it)->id())
            id = (*it)->id() + 1;
    }
    contact->m_id = id;
    p->contacts.push_back(contact);
    Event e(EventContactCreated, contact);
    e.process();
}

Group *ContactList::group(unsigned long id, bool isNew)
{
    vector<Group*>::iterator it;
    if (id || !isNew){
        for (it = p->groups.begin(); it != p->groups.end(); ++it){
            if ((*it)->id() == id)
                return (*it);
        }
    }
    if (!isNew)
        return NULL;
    if (id == 0){
        for (it = p->groups.begin(); it != p->groups.end(); ++it){
            if (id <= (*it)->id())
                id = (*it)->id() + 1;
        }
    }
    Group *res = new Group(id);
    p->groups.push_back(res);
    Event e(EventGroupCreated, res);
    e.process();
    return res;
}

int ContactList::groupIndex(unsigned long id)
{
    for (unsigned i = 0; i < p->groups.size(); i++){
        if (p->groups[i]->id() == id)
            return i;
    }
    return -1;
}

int ContactList::groupCount()
{
    return p->groups.size();
}

bool ContactList::moveGroup(unsigned long id, bool bUp)
{
    if (id == 0)
        return false;
    for (unsigned i = 1; i < p->groups.size(); i++){
        if (p->groups[i]->id() == id){
            if (bUp) i--;
            if (i == 0)
                return false;
            Group *g = p->groups[i];
            p->groups[i] = p->groups[i+1];
            p->groups[i+1] = g;
            Event e1(EventGroupChanged, p->groups[i]);
            Event e2(EventGroupChanged, p->groups[i+1]);
            e1.process();
            e2.process();
            return true;
        }
    }
    return false;
}

class GroupIteratorPrivate
{
public:
    GroupIteratorPrivate();
    Group *operator++();
    vector<Group*>::iterator it;
};

GroupIteratorPrivate::GroupIteratorPrivate()
{
    it = getContacts()->p->groups.begin();
}

Group *GroupIteratorPrivate::operator++()
{
    if (it != getContacts()->p->groups.end()){
        Group *res = (*it);
        ++it;
        return res;
    }
    return NULL;
}

ContactList::GroupIterator::GroupIterator()
{
    p = new GroupIteratorPrivate;
}

void ContactList::GroupIterator::reset()
{
    p->it = getContacts()->p->groups.begin();
}

ContactList::GroupIterator::~GroupIterator()
{
    delete p;
}

Group *ContactList::GroupIterator::operator ++()
{
    return ++(*p);
}

class ContactIteratorPrivate
{
public:
    ContactIteratorPrivate();
    Contact *operator++();
    list<Contact*>::iterator it;
};

ContactIteratorPrivate::ContactIteratorPrivate()
{
    it = getContacts()->p->contacts.begin();
}

Contact *ContactIteratorPrivate::operator++()
{
    if (it != getContacts()->p->contacts.end()){
        Contact *res = (*it);
        ++it;
        return res;
    }
    return NULL;
}

ContactList::ContactIterator::ContactIterator()
{
    p = new ContactIteratorPrivate;
}

void ContactList::ContactIterator::reset()
{
    p->it = getContacts()->p->contacts.begin();
}

ContactList::ContactIterator::~ContactIterator()
{
    delete p;
}

Contact *ContactList::ContactIterator::operator ++()
{
    return ++(*p);
}

class ProtocolIteratorPrivate
{
public:
    ProtocolIteratorPrivate();
    Protocol *operator++();
    list<Protocol*>::iterator it;
};

ProtocolIteratorPrivate::ProtocolIteratorPrivate()
{
    it = getContacts()->p->protocols.begin();
}

Protocol *ProtocolIteratorPrivate::operator++()
{
    if (it != getContacts()->p->protocols.end()){
        Protocol *res = (*it);
        ++it;
        return res;
    }
    return NULL;
}

ContactList::ProtocolIterator::ProtocolIterator()
{
    p = new ProtocolIteratorPrivate;
}

void ContactList::ProtocolIterator::reset()
{
    p->it = getContacts()->p->protocols.begin();
}

ContactList::ProtocolIterator::~ProtocolIterator()
{
    delete p;
}

Protocol *ContactList::ProtocolIterator::operator ++()
{
    return ++(*p);
}

Protocol::Protocol(Plugin *plugin)
{
    m_plugin = plugin;
    ContactListPrivate *p = getContacts()->p;
    p->protocols.push_back(this);
}

Protocol::~Protocol()
{
    ContactListPrivate *p = getContacts()->p;
    for (list<Protocol*>::iterator it = p->protocols.begin(); it != p->protocols.end(); ++it){
        if ((*it) != this)
            continue;
        p->protocols.erase(it);
        break;
    }
}

class PacketIteratorPrivate
{
public:
    PacketIteratorPrivate();
    PacketType *operator++();
    PACKET_MAP::iterator it;
};

PacketIteratorPrivate::PacketIteratorPrivate()
{
    it = getContacts()->p->packets.begin();
}

PacketType *PacketIteratorPrivate::operator++()
{
    if (it != getContacts()->p->packets.end()){
        PacketType *res = (*it).second;
        ++it;
        return res;
    }
    return NULL;
}

ContactList::PacketIterator::PacketIterator()
{
    p = new PacketIteratorPrivate;
}

void ContactList::PacketIterator::reset()
{
    p->it = getContacts()->p->packets.begin();
}

ContactList::PacketIterator::~PacketIterator()
{
    delete p;
}

PacketType *ContactList::PacketIterator::operator ++()
{
    return ++(*p);
}

PacketType::PacketType(unsigned id, const char *name, unsigned flags)
{
    m_id = id;
    m_name = strdup(name);
    m_flags = flags;
}

PacketType::~PacketType()
{
    free(m_name);
}

void ContactList::addPacketType(unsigned id, const char *name, unsigned flags)
{
    PACKET_MAP::iterator it = p->packets.find(id);
    if (it != p->packets.end())
        return;
    p->packets.insert(PACKET_MAP::value_type(id, new PacketType(id, name, flags)));
}

void ContactList::removePacketType(unsigned id)
{
    PACKET_MAP::iterator it = p->packets.find(id);
    if (it != p->packets.end()){
        delete (*it).second;
        p->packets.erase(it);
    }
}

PacketType *ContactList::getPacketType(unsigned id)
{
    PACKET_MAP::iterator it = p->packets.find(id);
    if (it == p->packets.end())
        return NULL;
    return (*it).second;
}

/*
typedef struct ClientData
{
    unsigned	ManualStatus;

    unsigned	CommonStatus;

    char		*Password;

    unsigned	SavePassword;

	char		*PreviousPassword;

    unsigned	Invisible;

} ClientData;
*/

static DataDef clientData[] =
    {
        { "ManualStatus", DATA_LONG, 1, STATUS_OFFLINE },
        { "CommonStatus", DATA_BOOL, 1, 1 },
        { "Password", DATA_UTF, 1, 0 },
        { "", DATA_BOOL, 1, 1 },		// SavePassword

        { "", DATA_UTF, 1, 0 },			// PreviousPassword

        { "Invisible", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Client::Client(Protocol *protocol, const char *cfg)
{
    load_data(clientData, &data, cfg);

    // now uncrypt password somehow
    QString pswd = getPassword();
    if (pswd.length() && (pswd[0] == '$')) {
        pswd = pswd.mid(1);
        QString new_pswd;
        unsigned short temp = 0x4345;
        QString tmp;
        do {
            QString sub_str = getToken(pswd, '$');
            temp ^= sub_str.toUShort(0,16);
            new_pswd += tmp.setUnicodeCodes(&temp,1);
            temp = sub_str.toUShort(0,16);
        } while (pswd.length());
        setPassword(new_pswd);
    }

    m_protocol = protocol;
    m_status = STATUS_OFFLINE;
    m_state  = Offline;
}

void Client::setStatus(unsigned status, bool bCommon)
{
    setManualStatus(status);
    setCommonStatus(bCommon);
    Event e(EventClientChanged, this);
    e.process();
}

Client::~Client()
{
    Group *grp;
    ContactList::GroupIterator itg;
    while ((grp = ++itg) != NULL){
        if (grp->clientData.size() == 0)
            continue;
        grp->clientData.freeClientData(this);
        Event e(EventGroupChanged, grp);
        e.process();
    }

    Contact *contact;
    ContactList::ContactIterator itc;
    list<Contact*> forRemove;
    while ((contact = ++itc) != NULL){
        if (contact->clientData.size() == 0)
            continue;
        contact->clientData.freeClientData(this);
        if (contact->clientData.size()){
            contact->setup();
            Event e(EventContactChanged, contact);
            e.process();
            continue;
        }
        forRemove.push_back(contact);
    }
    list<Contact*>::iterator itr;
    for (itr = forRemove.begin(); itr != forRemove.end(); ++itr){
        delete *itr;
    }
    ContactListPrivate *p = getContacts()->p;
    for (vector<Client*>::iterator it = p->clients.begin(); it != p->clients.end(); ++it){
        if ((*it) != this)
            continue;
        p->clients.erase(it);
        Event e(EventClientsChanged);
        e.process();
        break;
    }
    free_data(clientData, &data);
}

string Client::getConfig()
{
    QString real_pswd = getPassword();
    QString pswd = getPassword();

    // crypt password somehow
    if (pswd.length()) {
        QString new_passwd;
        unsigned short temp = 0x4345;
        for (int i = 0; i < (int)(pswd.length()); i++) {
            temp ^= (pswd[i].unicode());
            new_passwd += "$";
            new_passwd += QString::number(temp,16);
        }
        setPassword(new_passwd);
    }
    QString prev = getPreviousPassword();
    if (!prev.isEmpty())
        setPassword(prev);
    if (!getSavePassword())
        setPassword(NULL);
    string res = save_data(clientData, &data);
    setPassword(real_pswd);
    return res;
}

void Client::setClientInfo(void*)
{
}

unsigned Client::getStatus()
{
    return m_status;
}

bool Client::compareData(void*, void*)
{
    return false;
}

void Client::setState(State state, const char *text, unsigned code)
{
    m_state = state;
    Event e(EventClientChanged, this);
    e.process();
    if (state == Error){
        clientErrorData d;
        d.client  = this;
        d.err_str = text;
        d.code	  = code;
        d.args    = NULL;
        Event e(EventClientError, &d);
        e.process();
    }
}

bool ContactList::moveClient(Client *client, bool bUp)
{
    unsigned i;
    for (i = 0; i < p->clients.size(); i++)
        if (p->clients[i] == client)
            break;
    if (!bUp)
        i++;
    if ((i == 0) && (i >= p->clients.size()))
        return false;
    Client *c = p->clients[i];
    p->clients[i] = p->clients[i-1];
    p->clients[i-1] = c;
    Event e(EventClientsChanged);
    e.process();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        contact->clientData.sort();
        Event e(EventContactChanged, contact);
        e.process();
    }
    return true;
}

typedef struct _ClientUserData
{
    Client	*client;
    void	*data;
} _ClientUserData;

class ClientUserDataPrivate : public vector<_ClientUserData>
{
public:
    ClientUserDataPrivate();
    ~ClientUserDataPrivate();
};

ClientUserDataPrivate::ClientUserDataPrivate()
{
}

ClientUserDataPrivate::~ClientUserDataPrivate()
{
    for (ClientUserDataPrivate::iterator it = begin(); it != end(); ++it){
        _ClientUserData &d = *it;
        free_data(d.client->protocol()->userDataDef(), d.data);
        free(d.data);
    }
}

ClientUserData::ClientUserData()
{
    p = new ClientUserDataPrivate;
}

ClientUserData::~ClientUserData()
{
    delete p;
}

unsigned ClientUserData::size()
{
    return p->size();
}

bool ClientUserData::have(void *data)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        if ((*it).data == data)
            return true;
    }
    return false;
}

Client *ClientUserData::activeClient(void *&data, Client *client)
{
    ClientUserDataPrivate::iterator it;
    for (it = p->begin(); it != p->end(); ++it){
        if (((*it).client == client) && ((*it).data == data))
            break;
        if ((*it).client->protocol() != client->protocol())
            continue;
        if (client->compareData(data, (*it).data))
            return NULL;
    }
    if (it == p->end())
        return NULL;
    if (client->getState() == Client::Connected)
        return client;
    for (++it; it != p->end(); ++it){
        if ((*it).client->protocol() != client->protocol())
            continue;
        if ((*it).client->getState() != Client::Connected)
            continue;
        if (client->compareData(data, (*it).data)){
            data = (*it).data;
            return (*it).client;
        }
    }
    return client;
}

string ClientUserData::save()
{
    string res;
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        _ClientUserData &d = *it;
        string cfg = save_data(d.client->protocol()->userDataDef(), d.data);
        if (cfg.length()){
            if (res.length())
                res += "\n";
            res += "[";
            res += d.client->name();
            res += "]\n";
            res += cfg;
        }
    }
    return res;
}

void ClientUserData::load(Client *client, const char *cfg)
{
    _ClientUserData data;
    data.client = client;
    const DataDef *def = client->protocol()->userDataDef();
    size_t size = 0;
    for (const DataDef *d = def; d->name; ++d)
        size += sizeof(unsigned) * d->n_values;
    data.data = malloc(size);
    load_data(def, data.data, cfg);
    p->push_back(data);
}

void *ClientUserData::createData(Client *client)
{
    _ClientUserData data;
    data.client = client;
    const DataDef *def = client->protocol()->userDataDef();
    size_t size = 0;
    for (const DataDef *d = def; d->name; ++d)
        size += sizeof(unsigned) * d->n_values;
    data.data = malloc(size);
    load_data(def, data.data, NULL);
    p->push_back(data);
    return data.data;
}

void *ClientUserData::getData(Client *client)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        if ((*it).client == client)
            return (*it).data;
    }
    return NULL;
}

void ClientUserData::freeData(void *data)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        if ((*it).data == data){
            free_data((*it).client->protocol()->userDataDef(), data);
            free(data);
            p->erase(it);
            return;
        }
    }
}

void ClientUserData::freeClientData(Client *client)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end();){
        if ((*it).client != client){
            ++it;
            continue;
        }
        free_data((*it).client->protocol()->userDataDef(), (*it).data);
        free((*it).data);
        p->erase(it);
        it = p->begin();
    }
}

void ClientUserData::join(ClientUserData &data)
{
    for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it)
        p->push_back(*it);
    data.p->clear();
    sort();
}

void ClientUserData::join(unsigned n, ClientUserData &data)
{
    for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it, n--){
        if (n == 0){
            p->push_back(*it);
            data.p->erase(it);
            break;
        }
    }
    sort();
}

class ClientDataIteratorPrivate
{
public:
    ClientDataIteratorPrivate(ClientUserDataPrivate *p, Client *client);
    void *operator ++();
    Client *m_lastClient;
protected:
    ClientUserDataPrivate *m_p;
    ClientUserDataPrivate::iterator m_it;
    Client *m_client;
};

ClientDataIteratorPrivate::ClientDataIteratorPrivate(ClientUserDataPrivate *p, Client *client)
{
    m_p = p;
    m_client = client;
    m_lastClient = NULL;
    m_it = p->begin();
}

void *ClientDataIteratorPrivate::operator ++()
{
    for (; m_it != m_p->end(); ++m_it){
        if ((m_client == NULL) || ((*m_it).client == m_client)){
            void *res = (*m_it).data;
            m_lastClient = (*m_it).client;
            ++m_it;
            return res;
        }
    }
    return NULL;
}

ClientDataIterator::ClientDataIterator(ClientUserData &data, Client *client)
{
    p = new ClientDataIteratorPrivate(data.p, client);
}

ClientDataIterator::~ClientDataIterator()
{
    delete p;
}

void *ClientDataIterator::operator ++()
{
    return ++(*p);
}

Client *ClientDataIterator::client()
{
    return p->m_lastClient;
}

static bool cmp_client_data(_ClientUserData p1, _ClientUserData p2)
{
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *c = getContacts()->getClient(i);
        if (c == p1.client){
            if (c != p2.client)
                return true;
            return p1.data < p2.data;
        }
        if (c == p2.client)
            return false;
    }
    return p1.data < p2.data;
}

void ClientUserData::sort()
{
    std::sort(p->begin(), p->end(), cmp_client_data);
}

// _____________________________________________________________________________________

class UserDataIteratorPrivate : public list<UserDataDef>::iterator
{
public:
    UserDataIteratorPrivate(const list<UserDataDef>::iterator &it) :
    list<UserDataDef>::iterator(it) {}
};

UserDataDef *ContactList::UserDataIterator::operator++()
{
    if ((*p) == getContacts()->p->userDataDef.end())
        return NULL;
    UserDataDef *res = &(**p);
    ++(*p);
    return res;
}

ContactList::UserDataIterator::UserDataIterator()
{
    p = new UserDataIteratorPrivate(getContacts()->p->userDataDef.begin());
}

ContactList::UserDataIterator::~UserDataIterator()
{
    delete p;
}

// ______________________________________________________________________________________

UserData::UserData()
{
    n_data = 0;
    userData = NULL;
}

UserData::~UserData()
{
    if (userData){
        for (unsigned i = 0; i < n_data; i++)
            freeUserData(i);
        free(userData);
    }
}

void *UserData::getUserData(unsigned id, bool bCreate)
{
    if ((id < n_data) && userData[id])
        return userData[id];
    if (!bCreate)
        return NULL;
    list<UserDataDef> &d = getContacts()->p->userDataDef;
    list<UserDataDef>::iterator it;
    for (it= d.begin(); it != d.end(); ++it)
        if ((*it).id == id)
            break;
    if (it == d.end())
        return NULL;
    if (id >= n_data){
        size_t size = sizeof(void**) * (id + 1);
        if (userData){
            userData = (void**)realloc(userData, size);
        }else{
            userData = (void**)malloc(size);
        }
        memset(userData + n_data, 0, size - sizeof(void**) * n_data);
        n_data = id + 1;
    }
    size_t size = 0;
    for (const DataDef *def = (*it).def; def->name; ++def)
        size += sizeof(unsigned) * def->n_values;
    userData[id] = malloc(size);
    load_data((*it).def, userData[id], NULL);
    return userData[id];
}

void UserData::freeUserData(unsigned id)
{
    if ((id < n_data) && userData && userData[id]){
        list<UserDataDef> &d = getContacts()->p->userDataDef;
        for (list<UserDataDef>::iterator it = d.begin(); it != d.end(); ++it){
            if ((*it).id != id) continue;
            free_data((*it).def, userData[id]);
            break;
        }
        free(userData[id]);
        userData[id] = NULL;
    }
}

string UserData::save()
{
    string res;
    if (userData == NULL)
        return res;
    for (unsigned id = 0; id < n_data; id++){
        if (userData[id] == NULL)
            continue;
        list<UserDataDef> &d = getContacts()->p->userDataDef;
        for (list<UserDataDef>::iterator it = d.begin(); it != d.end(); ++it){
            if ((*it).id != id) continue;
            string cfg = save_data((*it).def, userData[id]);
            if (cfg.length()){
                if (res.length())
                    res += "\n";
                res += "[";
                res += (*it).name;
                res += "]\n";
                res += cfg;
            }
            break;
        }
    }
    return res;
}

void UserData::load(unsigned long id, const DataDef *def, const char *cfg)
{
    void *d = getUserData(id, true);
    if (d == NULL)
        return;
    free_data(def, d);
    load_data(def, d, cfg);
}

void ContactList::addClient(Client *client)
{
    p->clients.push_back(client);
    Event e(EventClientsChanged);
    e.process();
}

static char CONTACTS_CONF[] = "contacts.conf";
static char _CONTACT[] = "[Contact=";
static char _GROUP[] = "[Group=";
static char _OWNER[] = "[Owner]";

void ContactList::save()
{
    string cfgName = user_file(CONTACTS_CONF);
    QFile f(QFile::decodeName(cfgName.c_str()));
    if (!f.open(IO_WriteOnly | IO_Truncate)){
        log(L_ERROR, "Can't create %s", cfgName.c_str());
        return;
    }
    string line = p->userData.save();
    if (line.length()){
        f.writeBlock(line.c_str(), line.length());
        f.writeBlock("\n", 1);
    }
    line = save_data(contactData, &owner()->data);
    if (line.length()){
        string cfg  = _OWNER;
        cfg += "\n";
        f.writeBlock(cfg.c_str(), cfg.length());
        f.writeBlock(line.c_str(), line.length());
        f.writeBlock("\n", 1);
    }
    for (vector<Group*>::iterator it_g = p->groups.begin(); it_g != p->groups.end(); ++it_g){
        Group *grp = *it_g;
        line = _GROUP;
        line += number(grp->id());
        line += "]\n";
        f.writeBlock(line.c_str(), line.length());
        line = save_data(groupData, &grp->data);
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        }
        line = grp->userData.save();
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        }
        line = grp->clientData.save();
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        }
    }
    for (list<Contact*>::iterator it_c = p->contacts.begin(); it_c != p->contacts.end(); ++it_c){
        Contact *contact = *it_c;
        if (contact->getTemporary())
            continue;
        line = _CONTACT;
        line += number(contact->id());
        line += "]\n";
        f.writeBlock(line.c_str(), line.length());
        line = save_data(contactData, &contact->data);
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        }
        line = contact->userData.save();
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        }
        line = contact->clientData.save();
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        }
    }
}

void ContactList::clear()
{
    p->clear(false);
}

void ContactList::load()
{
    clear();
    string cfgName = user_file(CONTACTS_CONF);
    QFile f(QFile::decodeName(cfgName.c_str()));
    if (!f.open(IO_ReadOnly)){
        log(L_ERROR, "Can't open %s", cfgName.c_str());
        return;
    }
    string cfg;
    string s;
    string section;
    Contact *c = NULL;
    Group *g = NULL;
    while (getLine(f, s)){
        if (s[0] == '['){
            if (s == _OWNER){
                p->flush(c, g, section.c_str(), cfg.c_str());
                p->flush(c, g);
                cfg = "";
                c = owner();
                g = NULL;
                section = "";
                continue;
            }
            if ((s.length() > strlen(_GROUP)) && !memcmp(s.c_str(), _GROUP, strlen(_GROUP))){
                p->flush(c, g, section.c_str(), cfg.c_str());
                p->flush(c, g);
                cfg = "";
                c = NULL;
                unsigned long id = atol(s.c_str() + strlen(_GROUP));
                g = group(id, id != 0);
                section = "";
                continue;
            }
            if ((s.length() > strlen(_CONTACT)) && !memcmp(s.c_str(), _CONTACT, strlen(_CONTACT))){
                p->flush(c, g, section.c_str(), cfg.c_str());
                p->flush(c, g);
                cfg = "";
                g = NULL;
                unsigned long id = atol(s.c_str() + strlen(_GROUP));
                c = contact(id, true);
                section = "";
                continue;
            }
            p->flush(c, g, section.c_str(), cfg.c_str());
            cfg = "";
            s = s.substr(1);
            section = getToken(s, ']');
            continue;
        }
        cfg += s;
        cfg += "\n";
    }
    p->flush(c, g, section.c_str(), cfg.c_str());
    p->flush(c, g);
}

void ContactListPrivate::flush(Contact *c, Group *g)
{
    ClientUserData *data = NULL;
    if (c)
        data = &c->clientData;
    if (g)
        data = &g->clientData;
    if (data)
        data->sort();
}

void ContactListPrivate::flush(Contact *c, Group *g, const char *section, const char *cfg)
{
    if ((cfg == NULL) || (*cfg == 0))
        return;
    if (*section == 0){
        if (c){
            free_data(contactData, &c->data);
            load_data(contactData, &c->data, cfg);
            return;
        }
        if (g){
            free_data(groupData, &g->data);
            load_data(groupData, &g->data, cfg);
            return;
        }
        return;
    }
    list<UserDataDef>::iterator it;
    for (it = userDataDef.begin(); it != userDataDef.end(); ++it){
        if (strcmp(section, (*it).name.c_str()))
            continue;
        UserData *data = &userData;
        if (c)
            data = &c->userData;
        if (g)
            data = &g->userData;
        data->load((*it).id, (*it).def, cfg);
        return;
    }
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (client->name() != section)
            continue;
        ClientUserData *data = NULL;
        if (c)
            data = &c->clientData;
        if (g)
            data = &g->clientData;
        if (data)
            data->load(client, cfg);
        return;
    }
}

unsigned ContactList::nClients()
{
    return p->clients.size();
}

Client *ContactList::getClient(unsigned n)
{
    if (n >= p->clients.size())
        return NULL;
    return p->clients[n];
}

void ContactList::clearClients()
{
    p->bNoRemove = true;
    while (!p->clients.empty())
        delete p->clients[0];
    p->bNoRemove = false;
    Event eClients(EventClientsChanged);
    eClients.process();
}

void *ContactList::getUserData(unsigned id)
{
    return p->userData.getUserData(id, true);
}

static string stripPhone(const char *phone)
{
    string res;
    if (phone == NULL)
        return res;
    for (; *phone; phone++){
        char c = *phone;
        if ((c < '0') || (c >= '9'))
            continue;
        res += c;
    }
    return res;
}

bool ContactList::cmpPhone(const char *phone1, const char *phone2)
{
    return stripPhone(phone1) == stripPhone(phone2);
}

Contact *ContactList::contactByPhone(const char *_phone)
{
    string phone = stripPhone(_phone);
    if (phone.empty())
        return NULL;
    Contact *c;
    ContactIterator it;
    while ((c = ++it) != NULL){
        QString phones = c->getPhones();
        while (!phones.isEmpty()){
            QString phoneItem = getToken(phones, ';', false);
            if (cmpPhone(phoneItem.utf8(), _phone))
                return c;
        }
    }
    c = contact(0, true);
    c->setTemporary(CONTACT_TEMP);
    c->setName(QString::fromUtf8(_phone));
    Event e(EventContactChanged, c);
    e.process();
    return c;
}

Contact *ContactList::contactByMail(const QString &_mail, const QString &_name)
{
    QString name = _name;
    Contact *c;
    ContactIterator it;
    if (_mail.isEmpty()){
        while ((c = ++it) != NULL){
            if (c->getName().lower() == name.lower())
                return c;
        }
        c = contact(0, true);
        c->setTemporary(CONTACT_TEMP);
        c->setName(name);
        Event e(EventContactChanged, c);
        e.process();
        return c;
    }
    if (name.isEmpty())
        name = _mail;
    while ((c = ++it) != NULL){
        QString mails = c->getEMails();
        while (mails.length()){
            QString mail = getToken(mails, ';', false);
            mail = getToken(mail, '/');
            if (mail.lower() == _mail.lower())
                return c;
        }
    }
    c = contact(0, true);
    c->setTemporary(CONTACT_TEMP);
    c->setName(name);
    c->setEMails(_mail + "/-");
    Event e(EventContactChanged, c);
    e.process();
    return c;
}

EXPORT ContactList *getContacts()
{
    return PluginManager::contacts;
}

};

