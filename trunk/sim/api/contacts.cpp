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
#include "stl.h"
#include "buffer.h"

#include <qfile.h>
#include <qdir.h>
#include <qtextcodec.h>
#include <qregexp.h>

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
    void flush(Contact *c, Group *g, const char *section, Buffer *cfg);
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
        { "Flags", DATA_ULONG, 1, 0 },
        { "Encoding", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Contact::Contact(unsigned long id, Buffer *cfg)
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

static char tipDiv[] = "<br>__________<br>";

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

typedef struct sortClientData
{
    void		*data;
    Client		*client;
    unsigned	nClient;
} sortClientData;

static bool cmp_sd(sortClientData p1, sortClientData p2)
{
    if (((clientData*)(p1.data))->LastSend.value > ((clientData*)(p2.data))->LastSend.value)
        return true;
    if (((clientData*)(p1.data))->LastSend.value < ((clientData*)(p2.data))->LastSend.value)
        return false;
    return p1.nClient < p2.nClient;
}

unsigned long Contact::contactInfo(unsigned &style, const char *&statusIcon, string *icons)
{
    style = 0;
    statusIcon = NULL;
    if (icons)
        *icons = "";
    unsigned long status = STATUS_UNKNOWN;
    void *data;
    ClientDataIterator it(clientData, NULL);
    vector<sortClientData> d;
    while ((data = ++it) != NULL){
        sortClientData sd;
        sd.data    = data;
        sd.client  = it.client();
        sd.nClient = 0;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            if (getContacts()->getClient(i) == sd.client){
                sd.nClient = i;
                break;
            }
        }
        d.push_back(sd);
    }
    sort(d.begin(), d.end(), cmp_sd);
    for (unsigned i = 0; i < d.size(); i++){
        void *data = d[i].data;
        Client *client = clientData.activeClient(data, d[i].client);
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
        return STATUS_UNKNOWN;
    }
    if (statusIcon == NULL)
        statusIcon = "empty";
    return status;
}

string Client::resources(void*)
{
    return "";
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

Group::Group(unsigned long id, Buffer *cfg)
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

PacketType::PacketType(unsigned id, const char *name, bool bText)
{
    m_id    = id;
    m_name  = strdup(name);
    m_bText = bText;
}

PacketType::~PacketType()
{
    free(m_name);
}

void ContactList::addPacketType(unsigned id, const char *name, bool bText)
{
    PACKET_MAP::iterator it = p->packets.find(id);
    if (it != p->packets.end())
        return;
    p->packets.insert(PACKET_MAP::value_type(id, new PacketType(id, name, bText)));
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

static DataDef _clientData[] =
    {
        { "ManualStatus", DATA_LONG, 1, DATA(1) },
        { "CommonStatus", DATA_BOOL, 1, DATA(1) },
        { "Password", DATA_UTF, 1, 0 },
        { "", DATA_BOOL, 1, DATA(1) },		// SavePassword
        { "", DATA_UTF, 1, 0 },				// PreviousPassword
        { "Invisible", DATA_BOOL, 1, 0 },
        { "LastSend", DATA_STRLIST, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Client::Client(Protocol *protocol, Buffer *cfg)
{
    load_data(_clientData, &data, cfg);

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
    freeData();
}

void Client::freeData()
{
    ContactListPrivate *p = getContacts()->p;
    for (vector<Client*>::iterator it = p->clients.begin(); it != p->clients.end(); ++it){
        if ((*it) != this)
            continue;
        p->clients.erase(it);
        if (!getContacts()->p->bNoRemove){
            Event e(EventClientsChanged);
            e.process();
        }
        break;
    }
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
            if (!getContacts()->p->bNoRemove){
                contact->setup();
                Event e(EventContactChanged, contact);
                e.process();
            }
            continue;
        }
        forRemove.push_back(contact);
    }
    list<Contact*>::iterator itr;
    for (itr = forRemove.begin(); itr != forRemove.end(); ++itr){
        Contact *contact = *itr;
        delete contact;
    }
    free_data(_clientData, &data);
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
    string res = save_data(_clientData, &data);
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
        d.flags	  = ERR_ERROR;
        d.options = NULL;
        d.id	  = 0;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            if (getContacts()->getClient(i) == this){
                d.id = i + 1;
                break;
            }
        }
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

string ClientUserData::property(const char *name)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        _ClientUserData &d = *it;
        Data *user_data = (Data*)d.data;
        for (const DataDef *def = d.client->protocol()->userDataDef(); def->name; def++){
            if (!strcmp(def->name, name)){
                switch (def->type){
                case DATA_STRING:
                case DATA_UTF:
                    if (user_data->ptr)
                        return user_data->ptr;
                case DATA_ULONG:
                    if (user_data->value != (unsigned long)(def->def_value))
                        return number(user_data->value);
                }
            }
            user_data += def->n_values;
        }
    }
    return "";
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
        if (((clientData*)((*it).data))->Sign.value != ((clientData*)data)->Sign.value)
            continue;
        if (client->compareData(data, (*it).data))
            return NULL;
    }
    if (it == p->end())
        return NULL;
    if (client->getState() == Client::Connected)
        return client;
    for (++it; it != p->end(); ++it){
        if ((*it).client->getState() != Client::Connected)
            continue;
        if (((clientData*)((*it).data))->Sign.value != ((clientData*)data)->Sign.value)
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
        if (d.client->protocol()->description()->flags & PROTOCOL_TEMP_DATA)
            continue;
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

void ClientUserData::load(Client *client, Buffer *cfg)
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

void ClientUserData::join(clientData *cData, ClientUserData &data)
{
    for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it){
        if ((*it).data == cData){
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
    void reset();
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
    reset();
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

void ClientDataIteratorPrivate::reset()
{
    m_lastClient = NULL;
    m_it = m_p->begin();
}

ClientDataIterator::ClientDataIterator(ClientUserData &data, Client *client)
{
    p = new ClientDataIteratorPrivate(data.p, client);
}

ClientDataIterator::~ClientDataIterator()
{
    delete p;
}

clientData *ClientDataIterator::operator ++()
{
    return (clientData*)(++(*p));
}

void ClientDataIterator::reset()
{
    p->reset();
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

void UserData::load(unsigned long id, const DataDef *def, Buffer *cfg)
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
static char CONTACT[] = "Contact=";
static char GROUP[] = "Group=";
static char OWNER[] = "Owner";
static char BACKUP_SUFFIX[] = "~";

void ContactList::save()
{
    string cfgName = user_file(CONTACTS_CONF);
    QFile f(QFile::decodeName((cfgName + BACKUP_SUFFIX).c_str())); // use backup file for this ...
    if (!f.open(IO_WriteOnly | IO_Truncate)){
        log(L_ERROR, "Can't create %s", (const char*)f.name().local8Bit());
        return;
    }
    string line = p->userData.save();
    if (line.length()){
        f.writeBlock(line.c_str(), line.length());
        f.writeBlock("\n", 1);
    }
    line = save_data(contactData, &owner()->data);
    if (line.length()){
        string cfg  = "[";
        cfg += OWNER;
        cfg += "]\n";
        f.writeBlock(cfg.c_str(), cfg.length());
        f.writeBlock(line.c_str(), line.length());
        f.writeBlock("\n", 1);
    }
    for (vector<Group*>::iterator it_g = p->groups.begin(); it_g != p->groups.end(); ++it_g){
        Group *grp = *it_g;
        line = "[";
        line += GROUP;
        line += number(grp->id());
        line += "]\n";
        f.writeBlock(line.c_str(), line.length());
        line = save_data(groupData, &grp->data);
        if (line.length()){
            f.writeBlock(line.c_str(), line.length());
            f.writeBlock("\n", 1);
        } else {
            /* Group has no name --> Not In List
               since the load_data seems to have problems with totally empty
               entries, this must be ...*/
            f.writeBlock("Name=\"NIL\"\n", 11);
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
        if (contact->getFlags() & CONTACT_TEMPORARY)
            continue;
        line = "[";
        line += CONTACT;
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

    const int status = f.status();
#if COMPAT_QT_VERSION >= 0x030200
    const QString errorMessage = f.errorString();
#else
    const QString errorMessage = "Write file fail";
#endif
    f.close();
    if (status != IO_Ok) {
        log(L_ERROR, "IO error during writting to file %s : %s", (const char*)f.name().local8Bit(), (const char*)errorMessage.local8Bit());
        return;
    }

    // rename to normal file
    QFileInfo fileInfo(f.name());
    QString desiredFileName = fileInfo.fileName();
    desiredFileName = desiredFileName.left(desiredFileName.length() - strlen(BACKUP_SUFFIX));
#ifdef WIN32
    fileInfo.dir().remove(desiredFileName);
#endif
    if (!fileInfo.dir().rename(fileInfo.fileName(), desiredFileName)) {
        log(L_ERROR, "Can't rename file %s to %s", (const char*)fileInfo.fileName().local8Bit(), (const char*)desiredFileName.local8Bit());
        return;
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
    Buffer cfg;
    cfg.init(f.size());
    int n = f.readBlock(cfg.data(), f.size());
    if (n < 0){
        log(L_ERROR, "Can't read %s", cfgName.c_str());
        return;
    }
    Contact *c = NULL;
    Group   *g = NULL;
    for (;;){
        string s = cfg.getSection();
        if (s.empty())
            break;
        if (s == OWNER){
            p->flush(c, g);
            c = owner();
            g = NULL;
            s = "";
        }else if ((s.length() > strlen(GROUP)) && !memcmp(s.c_str(), GROUP, strlen(GROUP))){
            p->flush(c, g);
            c = NULL;
            unsigned long id = atol(s.c_str() + strlen(GROUP));
            g = group(id, id != 0);
            s = "";
        }else if ((s.length() > strlen(CONTACT)) && !memcmp(s.c_str(), CONTACT, strlen(CONTACT))){
            p->flush(c, g);
            g = NULL;
            unsigned long id = atol(s.c_str() + strlen(GROUP));
            c = contact(id, true);
            s = "";
        }
        p->flush(c, g, s.c_str(), &cfg);
    }
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

void ContactListPrivate::flush(Contact *c, Group *g, const char *section, Buffer *cfg)
{
    if (cfg == NULL)
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
        if ((c < '0') || (c > '9'))
            continue;
        res += c;
    }
    if (res.length() > 7)
        res = res.substr(res.length() - 7);
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
            if (cmpPhone(getToken(phoneItem, ',').utf8(), _phone))
                return c;
        }
    }
    c = contact(0, true);
    c->setFlags(CONTACT_TEMP);
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
        c->setFlags(CONTACT_TEMP);
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
    c->setFlags(CONTACT_TEMP);
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

static ENCODING encodings[] =
    {
        { I18N_NOOP("Unicode"), "UTF-8", 106, 0, 65001, true },

        { I18N_NOOP("Arabic"), "ISO 8859-6", 82, 180, 28596, false },
        { I18N_NOOP("Arabic"), "CP 1256", 2256, 180, 1256, true },

        { I18N_NOOP("Baltic"), "ISO 8859-13", 109, 186, 28594, false },
        { I18N_NOOP("Baltic"), "CP 1257", 2257, 186, 1257, true },

        { I18N_NOOP("Central European"), "ISO 8859-2", 5, 238, 28592, false },
        { I18N_NOOP("Esperanto"), "ISO 8859-3", 6, 238, 28593, false },
        { I18N_NOOP("Central European"), "CP 1250", 2250, 238, 1250, true },

        { I18N_NOOP("Chinese "), "GBK", 2025, 134, 0, false },
        { I18N_NOOP("Chinese Simplified"), "gbk2312",2312, 134, 0, true },
        { I18N_NOOP("Chinese Traditional"), "Big5",2026, 136, 0, true },

        { I18N_NOOP("Cyrillic"), "ISO 8859-5", 8, 204, 28595, false },
        { I18N_NOOP("Cyrillic"), "KOI8-R", 2084, 204, 1251, false },
        { I18N_NOOP("Ukrainian"), "KOI8-U", 2088, 204, 1251, false },
        { I18N_NOOP("Cyrillic"), "CP 1251", 2251, 204, 1251, true },

        { I18N_NOOP("Greek"), "ISO 8859-7", 10, 161, 28597, false },
        { I18N_NOOP("Greek"), "CP 1253", 2253, 161, 1253, true },

        { I18N_NOOP("Hebrew"), "ISO 8859-8-I", 85, 177, 28598,  false },
        { I18N_NOOP("Hebrew"), "CP 1255", 2255, 177, 1255, true },

        { I18N_NOOP("Japanese"), "JIS7", 16, 128, 0, false },
        { I18N_NOOP("Japanese"), "eucJP", 18, 128, 0, false },
        { I18N_NOOP("Japanese"), "Shift-JIS", 17, 128, 0, true },

        { I18N_NOOP("Korean"), "eucKR", 38, 0, 0, true },

        { I18N_NOOP("Western European"), "ISO 8859-1", 4, 0, 28591, false },
        { I18N_NOOP("Western European"), "ISO 8859-15", 111, 0, 28605, false },
        { I18N_NOOP("Western European"), "CP 1252", 2252, 0, 1252, true },

        { I18N_NOOP("Tamil"), "TSCII", 2028, 0, 0, true },

        { I18N_NOOP("Thai"), "TIS-620", 2259, 222, 0, true },

        { I18N_NOOP("Turkish"), "ISO 8859-9", 12, 162, 28599, false },
        { I18N_NOOP("Turkish"), "CP 1254", 2254, 162, 1254, true },

        { NULL, NULL, 0, 0, 0, false }
    };

const ENCODING *ContactList::getEncodings()
{
    return encodings;
}

QTextCodec *ContactList::getCodecByName(const char *encoding)
{
    QTextCodec *codec = NULL;
    if (encoding && *encoding){
        codec = QTextCodec::codecForName(encoding);
        if (codec)
            return codec;
    }
    codec = QTextCodec::codecForLocale();
    const ENCODING *e;
    for (e = encodings; e->language; e++){
        if (!strcmp(codec->name(), e->codec))
            break;
    }
    if (e->language && !e->bMain){
        for (e++; e->language; e++){
            if (e->bMain){
                codec = QTextCodec::codecForName(e->codec);
                break;
            }
        }
    }
    if (codec == NULL)
        codec= QTextCodec::codecForLocale();
    return codec;
}

QTextCodec *ContactList::getCodec(Contact *contact)
{
    QTextCodec *codec = NULL;
    if (contact && *contact->getEncoding()){
        codec = getCodecByName(contact->getEncoding());
        if (codec)
            return codec;
    }
    return getCodecByName(owner()->getEncoding());
}

QString ContactList::toUnicode(Contact *contact, const char *str)
{
    if (str && *str){
        QString res = getCodec(contact)->toUnicode(str, strlen(str));
        return res.replace(QRegExp("\\r"), "");
    }
    return "";
}

string ContactList::fromUnicode(Contact *contact, const QString &str)
{
    if (str.isEmpty())
        return "";
    QString s = str;
    s = s.replace(QRegExp("\\r?\\n"), "\\r\\n");
    QCString res = getCodec(contact)->fromUnicode(str);
    return (const char*)res;
}
}

EXPORT QString g_i18n(const char *text, Contact *contact)
{
    QString male = i18n("male", text);
    if (contact == NULL)
        return male;
    QString female = i18n("female", text);
    if (male == female)
        return male;
    string gender = contact->clientData.property("Gender");
    if (atol(gender.c_str()) == 1)
        return female;
    return male;
}

