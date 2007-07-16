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

#include <algorithm>
#include <vector>
#include <algorithm>

#include <qfile.h>
#include <qdir.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include "buffer.h"
#include "event.h"
#include "log.h"
#include "misc.h"
#include "unquot.h"

#include "contacts.h"

namespace SIM
{

using namespace std;

typedef map<unsigned, PacketType*>	PACKET_MAP;

class ContactListPrivate
{
public:
    ContactListPrivate();
    ~ContactListPrivate();
    void clear(bool bClearAll);
    unsigned registerUserData(const QString &name, const DataDef *def);
    void unregisterUserData(unsigned id);
    void flush(Contact *c, Group *g, const QCString &section, Buffer *cfg);
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

#ifdef __OS2__    
char *getDefEncoding()
{
	// default charset is win1251 if cp866 used in OS/2
    COUNTRYCODE ccode = {0};  // Country code info (0 = current country)
    COUNTRYINFO cinfo = {0};  // Buffer for country-specific information
    ULONG       ilen  = 0;

    if ( DosQueryCtryInfo(sizeof(cinfo), &ccode, &cinfo, &ilen) == 0 ) {
    	if ( cinfo.codepage == 866 ) {
    		return "CP 1251";
    	}
    }
    return NULL;
}
#endif    

static DataDef contactData[] =
    {
        { "Group", DATA_ULONG, 1, 0 },
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
#ifdef __OS2__        
        { "Encoding", DATA_STRING, 1, getDefEncoding() },
#else
        { "Encoding", DATA_STRING, 1, 0 },
#endif        
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

Contact::Contact(unsigned long id, Buffer *cfg)
{
    m_id = id;
    load_data(contactData, &data, cfg);
}

Contact::~Contact()
{
    if (!getContacts()->p->bNoRemove){
        EventContact e(this, EventContact::eDeleted);
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
        setFirstName(QString::null);
    str = getLastName();
    getToken(str, '/');
    if (str != '-')
        setLastName(QString::null);
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

struct STR_ITEM
{
    QString     value;
    QStringList proto;
};

typedef list<STR_ITEM> STR_LIST;

/* adds a value / client pair to str_list, *avoids* doubled entries */
static void add_str (STR_LIST & m, const QString & value, const char *client)
{
    STR_LIST::iterator it;

    if (!client)
        client = "-";

    /* check if value is already in str_list */
    for (it = m.begin (); it != m.end (); ++it) {
        QString     v = (*it).value;
        if (v == value)
            break;
    }
    /* already there */
    if (it != m.end ()) {
        QStringList &proto = (*it).proto;
        QStringList::iterator itp;
        /* client == '-' --> ignore */
        if (!proto.empty () && !strcmp (client, "-"))
            return;
        /* search if  already in list */
        itp = proto.find(client);
        if(itp == proto.end())
            return;
        /* search if "-" somewhere in list --> delete it */
        itp = proto.find("-");
        if(itp != proto.end())
            proto.erase (itp);
        /* add new client */
        proto.push_back (client);
    }
    else {
        /* new entry */
        STR_ITEM    item;
        item.value = value;
        item.proto.push_back (client);
        m.push_back (item);
    }
}

static QString addStrings (const QString &old_value, const QString &values,
                           const QString &client)
{
    STR_LIST    str_list;

    /* split old_value */
    QString     m = old_value;
    while (m.length ()) {
        /* str_item  == one telephone entry */
        QString     str_item = getToken (m, ';', false);
        /* str == telephone entry without clients
         * str_item == clients
         */
        QString     str = getToken (str_item, '/');
        /* now split clients */
        while (str_item.length ()) {
            /* split by '/' - this was inserted as this function
             * doesn't worked correct
             */
            QString     proto = getToken (str_item, '/');
            /* now split by ',' since this seems to be the correct separator */
            while (proto.length ()) {
                QString     proto2 = getToken (proto, ',');
                /* and add */
                add_str (str_list, str, proto2);
            }
        }
    }

    /* split values */
    m = values;
    while (m.length ()) {
        QString     str_item = getToken (m, ';', false);
        QString     str = getToken (str_item, '/');
        while (str_item.length ()) {
            QString     proto = getToken (str_item, '/');
            while (proto.length ()) {
                QString     proto2 = getToken (proto, ',');
                add_str (str_list, str, proto2);
            }
        }
    }
    /* add new client if new client was given */
    if (!client.isEmpty()) {
        for (STR_LIST::iterator it = str_list.begin (); it != str_list.end (); ++it) {
            add_str (str_list, (*it).value, client);
        }
    }

    /* now build new string */
    QString     res;
    for (STR_LIST::iterator it = str_list.begin (); it != str_list.end (); ++it) {
        QStringList &proto = (*it).proto;
        if (proto.size() == 0)
            continue;
        if (res.length())
            res += ';';
        res += quoteChars ((*it).value, ";/");
        res += '/';
        QString     proto_str;
        for (QStringList::iterator itp = proto.begin (); itp != proto.end (); ++itp) {
            if (proto_str.length ())
                proto_str += ',';
            proto_str += quoteChars ((*itp), ",;/");
        }
        res += proto_str;
    }
    return res;
}

bool Contact::setEMails(const QString &mail, const QString &client)
{
    return setEMails(addStrings(getEMails(), mail, client));
}

bool Contact::setPhones(const QString &phone, const QString &client)
{
    return setPhones(addStrings(getPhones(), phone, client));
}

static QString packString(const QString &value, const QString &client)
{
    QString res = quoteChars(value, "/");
    res += '/';
    if (!client.isEmpty()){
        res += client;
    }else{
        res += '-';
    }
    return res;
}

static QString addString(const QString &oldValue, const QString &newValue, const QString &client)
{
    QString res;
    if (oldValue.length() == 0){
        if (newValue.length() == 0)
            return res;
        return packString(newValue, client);
    }
    QString value = oldValue;
    getToken(value, '/');
    if (value == client || client.isEmpty())
        return packString(newValue, client);
    return oldValue;
}

bool Contact::setFirstName(const QString &name, const QString &client)
{
    return setFirstName(addString(getFirstName(), name, client));
}

bool Contact::setLastName(const QString &name, const QString &client)
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
            tip += ' ';
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
        unsigned phone_type = phone_item.toULong();
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
        tip += ' ';
        tip += quoteString(phone);
    }
    return tip;
}

struct sortClientData
{
    void		*data;
    Client		*client;
    unsigned	nClient;
};

static bool cmp_sd(sortClientData p1, sortClientData p2)
{
    if (((clientData*)(p1.data))->LastSend.asULong() > ((clientData*)(p2.data))->LastSend.asULong())
        return true;
    if (((clientData*)(p1.data))->LastSend.asULong() < ((clientData*)(p2.data))->LastSend.asULong())
        return false;
    return p1.nClient < p2.nClient;
}

unsigned long Contact::contactInfo(unsigned &style, QString &statusIcon, QString *icons)
{
    style = 0;
    statusIcon = QString::null;
    if (icons)
        *icons = QString::null;
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
        unsigned n = phoneItem.toULong();
        if (n == CELLULAR)
            bCell = true;
        if (n == PAGER)
            bPager = true;
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

QString Client::resources(void*)
{
    return QString::null;
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

QString Client::contactTip(void*)
{
    return QString::null;
}

void Client::updateInfo(Contact *contact, void *data)
{
    if (data){
        EventContact(contact, EventContact::eFetchInfoFailed).process();
    }else{
        EventClientChanged(this).process();
    }
}

static DataDef groupData[] =
    {
        { "Name", DATA_UTF, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
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
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
        EventGroup e(this, EventGroup::eDeleted);
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

unsigned ContactListPrivate::registerUserData(const QString &name, const DataDef *def)
{
    unsigned id = 0x1000;   // must be unique...
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
        if (id != (*it).id)
            continue;
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

unsigned ContactList::registerUserData(const QString &name, const DataDef *def)
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
    EventContact e(res, EventContact::eCreated);
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
    EventContact e(contact, EventContact::eCreated);
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
    EventGroup e(res, EventGroup::eAdded);
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
            if (!bUp && (i == p->groups.size() - 1))
                return false;
            if (bUp) i--;
            if (i == 0)
                return false;
            Group *g = p->groups[i];
            p->groups[i] = p->groups[i+1];
            p->groups[i+1] = g;
            EventGroup e1(p->groups[i], EventGroup::eChanged);
            EventGroup e2(p->groups[i+1], EventGroup::eChanged);
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

PacketType::PacketType(unsigned id, const QString &name, bool bText)
{
    m_id    = id;
    m_name  = name;
    m_bText = bText;
}

PacketType::~PacketType()
{
}

void ContactList::addPacketType(unsigned id, const QString &name, bool bText)
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

static DataDef _clientData[] =
    {
        { "ManualStatus", DATA_ULONG, 1, DATA(1) },
        { "CommonStatus", DATA_BOOL, 1, DATA(1) },
        { "Password", DATA_UTF, 1, 0 },
        { "", DATA_BOOL, 1, DATA(1) },      // SavePassword
        { "", DATA_UTF, 1, 0 },             // PreviousPassword
        { "Invisible", DATA_BOOL, 1, 0 },
        { "LastSend", DATA_STRLIST, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
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
    EventClientChanged(this).process();
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
            EventClientsChanged e;
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
        if (!getContacts()->p->bNoRemove){
            EventGroup e(grp, EventGroup::eChanged);
            e.process();
        }
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
                EventContact e(contact, EventContact::eChanged);
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

QCString Client::getConfig()
{
    QString real_pswd = getPassword();
    QString pswd = getPassword();

    // crypt password somehow
    if (pswd.length()) {
        QString new_passwd;
        unsigned short temp = 0x4345;
        for (int i = 0; i < (int)(pswd.length()); i++) {
            temp ^= (pswd[i].unicode());
            new_passwd += '$';
            new_passwd += QString::number(temp,16);
        }
        setPassword(new_passwd);
    }
    QString prev = getPreviousPassword();
    if (!prev.isEmpty())
        setPassword(prev);
    if (!getSavePassword())
        setPassword(NULL);
    QCString res = save_data(_clientData, &data);
    setPassword(real_pswd);
    return res;
}

void Client::setClientInfo(void*)
{
}

bool Client::compareData(void*, void*)
{
    return false;
}

void Client::setState(State state, const QString &text, unsigned code)
{
    m_state = state;
    EventClientChanged(this).process();
    if (state == Error){
        EventError::ClientErrorData d;
        d.client  = this;
        d.err_str = text;
        d.code    = code;
        d.args    = QString::null;
        d.flags   = EventError::ClientErrorData::E_ERROR;
        d.options = NULL;
        d.id      = 0;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            if (getContacts()->getClient(i) == this){
                d.id = i + 1;
                break;
            }
        }
        EventClientError e(d);
        e.process();
    }
}

void Client::contactsLoaded()
{
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
    EventClientsChanged e;
    e.process();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        contact->clientData.sort();
        EventContact e(contact, EventContact::eChanged);
        e.process();
    }
    return true;
}

struct _ClientUserData
{
    Client  *client;
    Data    *data;
};

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
    // why do I have to delete something here which is created somehwere else??
    for (ClientUserDataPrivate::iterator it = begin(); it != end(); ++it){
        _ClientUserData &d = *it;
        free_data(d.client->protocol()->userDataDef(), d.data);
        delete[] d.data;
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

QString ClientUserData::property(const char *name)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        _ClientUserData &d = *it;
        Data *user_data = (Data*)d.data;
        for (const DataDef *def = d.client->protocol()->userDataDef(); def->name; def++){
            if (!strcmp(def->name, name)){
                switch (def->type){
                case DATA_STRING:
                case DATA_UTF:
                    if (!user_data->str().isEmpty())
                        return user_data->str();
                case DATA_ULONG:
                    if (user_data->toULong() != (unsigned long)(def->def_value))
                        return QString::number(user_data->toULong());
                case DATA_LONG:
                    if (user_data->toLong() != (long)(def->def_value))
                        return QString::number(user_data->toLong());
                default:
                     break;
                }
            }
            user_data += def->n_values;
        }
    }
    return QString::null;
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
        if (((clientData*)((*it).data))->Sign.toULong() != ((clientData*)data)->Sign.toULong())
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
        if (((clientData*)((*it).data))->Sign.toULong() != ((clientData*)data)->Sign.toULong())
            continue;
        if (client->compareData(data, (*it).data)){
            data = (*it).data;
            return (*it).client;
        }
    }
    return client;
}

QCString ClientUserData::save()
{
    QCString res;
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        _ClientUserData &d = *it;
        if (d.client->protocol()->description()->flags & PROTOCOL_TEMP_DATA)
            continue;
        QCString cfg = save_data(d.client->protocol()->userDataDef(), d.data);
        if (cfg.length()){
            if (res.length())
                res += '\n';
            res += '[';
            res += d.client->name().utf8();
            res += "]\n";
            res += cfg;
        }
    }
    return res;
}

void ClientUserData::load(Client *client, Buffer *cfg)
{
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        Client *c = (*it).client;
        if(c == client)
            return;
    }
    _ClientUserData data;
    data.client = client;
    const DataDef *def = client->protocol()->userDataDef();
    size_t size = 0;
    for (const DataDef *d = def; d->name; ++d)
        size += d->n_values;
    data.data = new Data[size];
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
        size += d->n_values;
    data.data = new Data[size];
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

void ClientUserData::freeData(void *_data)
{
    SIM::Data *data = (SIM::Data*)_data;
    for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
        if ((*it).data == data){
            free_data((*it).client->protocol()->userDataDef(), data);
            delete[] data;
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
        delete[] (*it).data;
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
        if ((*it).data == &(cData->Sign)){
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

typedef QMap<unsigned, SIM::Data*> UserDataMap;

class UserDataPrivate
{
public:
    UserDataMap m_userData;
};

UserData::UserData()
{
    d = new UserDataPrivate;
}

UserData::~UserData()
{
    UserDataMap::Iterator userDataIt;
    for(userDataIt = d->m_userData.begin(); userDataIt != d->m_userData.end(); ++userDataIt) {
        list<UserDataDef> &dataDef = getContacts()->p->userDataDef;
        for (list<UserDataDef>::iterator it = dataDef.begin(); it != dataDef.end(); ++it){
            if ((*it).id != userDataIt.key())
                continue;
            free_data((*it).def, *userDataIt);
            break;
        }
        delete[] userDataIt.data();
    }
    delete d;
}

void *UserData::getUserData(unsigned id, bool bCreate)
{
    UserDataMap::Iterator userDataIt = d->m_userData.find(id);
    if (userDataIt != d->m_userData.end())
        return *userDataIt;
    if (!bCreate)
        return NULL;

    list<UserDataDef> &dataDef = getContacts()->p->userDataDef;
    list<UserDataDef>::iterator it;
    for (it= dataDef.begin(); it != dataDef.end(); ++it)
        if ((*it).id == id)
            break;
    if (it == dataDef.end())
        return NULL;

    size_t size = 0;
    for (const DataDef *def = (*it).def; def->name; ++def)
        size += def->n_values;

    Data* data = new Data[size];
    d->m_userData.insert(id, data);
    load_data((*it).def, data, NULL);
    return data;
}

void UserData::freeUserData(unsigned id)
{
    UserDataMap::Iterator userDataIt = d->m_userData.find(id);
    if (userDataIt != d->m_userData.end()) {
        list<UserDataDef> &dataDef = getContacts()->p->userDataDef;
        for (list<UserDataDef>::iterator it = dataDef.begin(); it != dataDef.end(); ++it){
            if ((*it).id != id)
                continue;
            free_data((*it).def, d->m_userData[id]);
            break;
        }
        delete[] userDataIt.data();
        d->m_userData.erase(userDataIt);
    }
}

QCString UserData::save()
{
    QCString res;
    UserDataMap::Iterator userDataIt;
    for(userDataIt = d->m_userData.begin(); userDataIt != d->m_userData.end(); ++userDataIt) {
        list<UserDataDef> &d = getContacts()->p->userDataDef;
        for (list<UserDataDef>::iterator it = d.begin(); it != d.end(); ++it){
            if ((*it).id != userDataIt.key())
                continue;
            QCString cfg = save_data((*it).def, userDataIt.data());
            if (cfg.length()){
                if (res.length())
                    res += '\n';
                res += '[';
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
    EventClientsChanged e;
    e.process();
}

static char CONTACTS_CONF[] = "contacts.conf";
static char CONTACT[] = "Contact=";
static char GROUP[] = "Group=";
static char OWNER[] = "Owner";
static char BACKUP_SUFFIX[] = "~";

void ContactList::save()
{
    QString cfgName = user_file(CONTACTS_CONF);
    QFile f(cfgName + BACKUP_SUFFIX); // use backup file for this ...
    if (!f.open(IO_WriteOnly | IO_Truncate)){
        log(L_ERROR, "Can't create %s", (const char*)f.name().local8Bit());
        return;
    }
    QCString line = p->userData.save();
    if (line.length()){
        line += '\n';
        f.writeBlock(line, line.length());
    }
    line = save_data(contactData, &owner()->data);
    if (line.length()){
        QCString cfg  = "[";
        cfg += OWNER;
        cfg += "]\n";
        line += '\n';
        f.writeBlock(cfg, cfg.length());
        f.writeBlock(line, line.length());
    }
    for (vector<Group*>::iterator it_g = p->groups.begin(); it_g != p->groups.end(); ++it_g){
        Group *grp = *it_g;
        line = "[";
        line += GROUP;
        line += QString::number(grp->id());
        line += "]\n";
        f.writeBlock(line, line.length());
        line = save_data(groupData, &grp->data);
        if (line.length()){
            line += '\n';
            f.writeBlock(line, line.length());
        } else {
            /* Group has no name --> Not In List
               since the load_data seems to have problems with totally empty
               entries, this must be ...*/
            f.writeBlock("Name=\"NIL\"\n", 11);
        }
        line = grp->userData.save();
        if (line.length()){
            line += '\n';
            f.writeBlock(line, line.length());
        }
        line = grp->clientData.save();
        if (line.length()){
            line += '\n';
            f.writeBlock(line, line.length());
        }
    }
    for (list<Contact*>::iterator it_c = p->contacts.begin(); it_c != p->contacts.end(); ++it_c){
        Contact *contact = *it_c;
        if (contact->getFlags() & CONTACT_TEMPORARY)
            continue;
        line = "[";
        line += CONTACT;
        line += QString::number(contact->id());
        line += "]\n";
        f.writeBlock(line, line.length());
        line = save_data(contactData, &contact->data);
        if (line.length()){
            line += '\n';
            f.writeBlock(line, line.length());
        }
        line = contact->userData.save();
        if (line.length()){
            line += '\n';
            f.writeBlock(line, line.length());
        }
        line = contact->clientData.save();
        if (line.length()){
            line += '\n';
            f.writeBlock(line, line.length());
        }
    }

    const int status = f.status();
    const QString errorMessage = f.errorString();
    f.close();
    if (status != IO_Ok) {
        log(L_ERROR, "IO error during writing to file %s : %s", (const char*)f.name().local8Bit(), (const char*)errorMessage.local8Bit());
        return;
    }

    // rename to normal file
    QFileInfo fileInfo(f.name());
    QString desiredFileName = fileInfo.fileName();
    desiredFileName = desiredFileName.left(desiredFileName.length() - strlen(BACKUP_SUFFIX));
#if defined( WIN32 ) || defined( __OS2__ )
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
    QString cfgName = user_file(CONTACTS_CONF);
    QFile f(cfgName);
    if (!f.open(IO_ReadOnly)){
        log(L_ERROR, "Can't open %s", cfgName.local8Bit().data());
        return;
    }
    Buffer cfg = f.readAll();

    Contact *c = NULL;
    Group   *g = NULL;
    for (;;){
        QCString s = cfg.getSection();
		QString section = s;	// is ascii - ok here
        if (section.isEmpty())
            break;
        if (section == OWNER){
            p->flush(c, g);
            c = owner();
            g = NULL;
            s = "";
        }else if (section.startsWith(GROUP)){
            p->flush(c, g);
            c = NULL;
            unsigned long id = section.mid(strlen(GROUP)).toLong();
            g = group(id, id != 0);
            s = "";
        }else if (section.startsWith(CONTACT)){
            p->flush(c, g);
            g = NULL;
            unsigned long id = section.mid(strlen(CONTACT)).toLong();
            c = contact(id, true);
            s = "";
        }
        p->flush(c, g, s, &cfg);
    }
    p->flush(c, g);
    // Notify the clients about the newly loaded contact list
    for (unsigned i = 0; i < nClients(); i++){
        Client *client = getClient(i);
        client->contactsLoaded();
    }
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

void ContactListPrivate::flush(Contact *c, Group *g, const QCString &_section, Buffer *cfg)
{
    if (cfg == NULL)
        return;
    // section name is ascii every time
    QString section(_section);
    if (section.isEmpty()){
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
        if (section != (*it).name)
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
    EventClientsChanged eClients;
    eClients.process();
}

void *ContactList::getUserData(unsigned id)
{
    return p->userData.getUserData(id, true);
}

static QString stripPhone(const QString &phone)
{
    QString res;
    if (phone == NULL)
        return res;
    for (unsigned i = 0; i < phone.length(); i++){
        const QChar &c = phone[(int)i];
        if ((c < '0') || (c > '9'))
            continue;
        res += c;
    }
    if (res.length() > 7)
        res = res.mid(res.length() - 7);
    return res;
}

bool ContactList::cmpPhone(const QString &phone1, const QString &phone2)
{
    return stripPhone(phone1) == stripPhone(phone2);
}

Contact *ContactList::contactByPhone(const QString &_phone)
{
    QString phone = stripPhone(_phone);
    if (phone.isEmpty())
        return NULL;
    Contact *c;
    ContactIterator it;
    while ((c = ++it) != NULL){
        QString phones = c->getPhones();
        while (!phones.isEmpty()){
            QString phoneItem = getToken(phones, ';', false);
            if (cmpPhone(getToken(phoneItem, ','), _phone))
                return c;
        }
    }
    c = contact(0, true);
    c->setFlags(CONTACT_TEMP);
    c->setName(_phone);
    EventContact e(c, EventContact::eChanged);
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
        EventContact e(c, EventContact::eChanged);
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
    EventContact e(c, EventContact::eChanged);
    e.process();
    return c;
}

EXPORT ContactList *getContacts()
{
    return PluginManager::contacts;
}

// see also http://www.iana.org/assignments/character-sets
static ENCODING encodings[] =
    {
        { I18N_NOOP("Unicode"),  "UTF-8",  106, 0, 65001, true },

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

        { I18N_NOOP("Tamil"), "TSCII", 2107, 0, 0, true },

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
    const char *codecName = codec->name();
    for (e = encodings; e->language; e++){
        if (!strcmp(codecName, e->codec))
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
    if (contact && !contact->getEncoding().isEmpty()){
        codec = getCodecByName(contact->getEncoding());
        if (codec)
            return codec;
    }
    return getCodecByName(owner()->getEncoding());
}

QString ContactList::toUnicode(Contact *contact, const QCString &str, int length)
{
    if (!str.isEmpty()){
        if (length < 0)
            length = str.length();
        QString res = getCodec(contact)->toUnicode(str, length);
        return res.remove('\r');
    }
    return QString::null;
}

QCString ContactList::fromUnicode(Contact *contact, const QString &str)
{
    if (str.isEmpty())
        return "";
    QString s = str;
    s = s.replace(QRegExp("\r?\n"), "\r\n");
    return getCodec(contact)->fromUnicode(s);
}

}   // namespace sim

EXPORT QString g_i18n(const char *text, SIM::Contact *contact)
{
    QString male = i18n("male", text);
    if (contact == NULL)
        return male;
    QString female = i18n("female", text);
    if (male == female)
        return male;
    QString gender = contact->clientData.property("Gender");
    if (gender.toLong() == 1)
        return female;
    return male;
}

