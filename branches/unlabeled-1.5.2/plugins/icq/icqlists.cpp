/***************************************************************************
                          icqlists.cpp  -  description
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

#include "icqclient.h"
#include "icqmessage.h"

#include <stdio.h>
#include <time.h>
#include <vector>

using namespace std;

const unsigned short ICQ_SNACxLISTS_ERROR          = 0x0001;
const unsigned short ICQ_SNACxLISTS_REQxRIGHTS     = 0x0002;
const unsigned short ICQ_SNACxLISTS_RIGHTS         = 0x0003;
const unsigned short ICQ_SNACxLISTS_REQxROSTER     = 0x0005;
const unsigned short ICQ_SNACxLISTS_ROSTER         = 0x0006;
const unsigned short ICQ_SNACxLISTS_UNKNOWN		   = 0x0007;
const unsigned short ICQ_SNACxLISTS_CREATE         = 0x0008;
const unsigned short ICQ_SNACxLISTS_RENAME         = 0x0009;
const unsigned short ICQ_SNACxLISTS_DELETE         = 0x000A;
const unsigned short ICQ_SNACxLISTS_DONE           = 0x000E;
const unsigned short ICQ_SNACxLISTS_ROSTERxOK      = 0x000F;
const unsigned short ICQ_SNACxLISTS_EDIT           = 0x0011;
const unsigned short ICQ_SNACxLISTS_SAVE           = 0x0012;
const unsigned short ICQ_SNACxLISTS_CREATE_USER    = 0x0014;
const unsigned short ICQ_SNACxLISTS_REQUEST_AUTH   = 0x0018;
const unsigned short ICQ_SNACxLISTS_AUTHxREQUEST   = 0x0019;
const unsigned short ICQ_SNACxLISTS_AUTHxSEND	   = 0x001A;
const unsigned short ICQ_SNACxLISTS_AUTH           = 0x001B;
const unsigned short ICQ_SNACxLISTS_ADDED          = 0x001C;

const unsigned short ICQ_USER					= 0x0000;
const unsigned short ICQ_GROUPS					= 0x0001;
const unsigned short ICQ_VISIBLE_LIST			= 0x0002;
const unsigned short ICQ_INVISIBLE_LIST			= 0x0003;
const unsigned short ICQ_INVISIBLE_STATE		= 0x0004;
const unsigned short ICQ_IGNORE_LIST			= 0x000E;

const unsigned short TLV_ALIAS		= 0x0131;
const unsigned short TLV_WAIT_AUTH	= 0x0066;
const unsigned short TLV_CELLULAR	= 0x013A;

class ListServerRequest
{
public:
    ListServerRequest(unsigned short seq) { m_seq = seq; }
    virtual ~ListServerRequest() {}
    unsigned short seq() { return m_seq; }
    virtual void process(ICQClient *client, unsigned short res) = 0;
protected:
    unsigned short m_seq;
};

class GroupServerRequest : public ListServerRequest
{
public:
    GroupServerRequest(unsigned short seq, unsigned long id, unsigned short icq_id, const char *name);
    virtual void process(ICQClient *client, unsigned short res);
protected:
    unsigned long      m_id;
    unsigned short     m_icqId;
    string             m_name;
};

class ContactServerRequest : public ListServerRequest
{
public:
    ContactServerRequest(unsigned short seq, const char *screen,
                         unsigned short icq_id, unsigned short grp_id, TlvList *tlv = NULL);
    ~ContactServerRequest();
    virtual void process(ICQClient *client, unsigned short res);
protected:
    string		       m_screen;
    unsigned short     m_icqId;
    unsigned short     m_grpId;
    TlvList            *m_tlv;
};

class SetListRequest : public ListServerRequest
{
public:
    SetListRequest(unsigned short seq, const char *screen,
                   unsigned short icq_id, unsigned short type);
    virtual void process(ICQClient *client, unsigned short res);
protected:
    string			m_screen;
    unsigned short	m_icqId;
    unsigned short	m_type;
};

void ICQClient::snac_lists(unsigned short type, unsigned short seq)
{
    switch (type){
    case ICQ_SNACxLISTS_ERROR:
        break;
    case ICQ_SNACxLISTS_RIGHTS:
        log(L_DEBUG, "List rights");
        break;
    case ICQ_SNACxLISTS_ROSTER:{
            char c;
            unsigned short list_len;
            log(L_DEBUG,"Rosters");
            m_socket->readBuffer >> c;
            if (c){
                log(L_WARN, "Bad first roster byte %02X", c);
                break;
            }
            bool bIgnoreTime = false;
            if (!m_bRosters){
                m_bRosters = true;
                setContactsInvisible(0);
                Group *grp;
                ContactList::GroupIterator it_g;
                while ((grp = ++it_g) != NULL){
                    ICQUserData *data;
                    ClientDataIterator it(grp->clientData, this);
                    while ((data = (ICQUserData*)(++it)) != NULL){
                        if (data->IcqID == 0){
                            data->bChecked = true;
                            continue;
                        }
                        data->bChecked = false;
                    }
                }
                Contact *contact;
                ContactList::ContactIterator it_c;
                while ((contact = ++it_c) != NULL){
                    ICQUserData *data;
                    ClientDataIterator it(contact->clientData, this);
                    while ((data = (ICQUserData*)(++it)) != NULL){
                        data->bChecked = false;
                        data->GrpId = 0;
                        data->IgnoreId = 0;
                        data->VisibleId = 0;
                        data->InvisibleId = 0;
                    }
                }
            }
            m_socket->readBuffer >> list_len;
            for (unsigned i = 0; i < list_len; i++){
                string str;
                unsigned short id, grp_id, type, len;
                m_socket->readBuffer.unpackStr(str);
                m_socket->readBuffer >> grp_id >> id >> type >> len;
                TlvList *inf = NULL;
                if (len){
                    Buffer b(len);
                    b.pack(m_socket->readBuffer.data(m_socket->readBuffer.readPos()), len);
                    m_socket->readBuffer.incReadPos(len);
                    inf = new TlvList(b);
                }
                switch (type){
                case ICQ_USER: {
                        if (str.length()){
                            log(L_DEBUG, "User %s", str.c_str());
                            // check for own uin in contact lsit
                            if ((unsigned)atol(str.c_str()) == getUin()) {
                                log(L_DEBUG, "Own Uin in contact list - removing!");
                                seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", grp_id, id);
                                m_listRequest = new ContactServerRequest(seq, number(id).c_str(), 0, 0);
                                break;
                            }
                            ListRequest *lr = findContactListRequest(str.c_str());
                            if (lr){
                                log(L_DEBUG, "Request found");
                                lr->icq_id = id;
                                lr->grp_id = grp_id;
                                Contact *contact;
                                ICQUserData *data = findContact(lr->screen.c_str(), NULL, false, contact);
                                if (data){
                                    data->IcqID = id;
                                    data->GrpId = grp_id;
                                }
                            }else{
                                bool bChanged = false;
                                string alias;
                                Tlv *tlv_name = NULL;
                                if (inf) tlv_name = (*inf)(TLV_ALIAS);
                                if (tlv_name)
                                    alias = (char*)(*tlv_name);
                                log(L_DEBUG, "User %s [%s] - %u", str.c_str(), alias.c_str(), grp_id);
                                Contact *contact;
                                Group *grp = NULL;
                                ICQUserData *data = findGroup(grp_id, NULL, grp);
                                data = findContact(str.c_str(), alias.c_str(), true, contact, grp);
                                if (inf && (*inf)(TLV_WAIT_AUTH)){
                                    if (!data->WaitAuth){
                                        data->WaitAuth = true;
                                        bChanged = true;
                                    }
                                }
                                data->IcqID = id;
                                data->GrpId = grp_id;
                                Tlv *tlv_phone = NULL;
                                if (inf) tlv_phone = (*inf)(TLV_CELLULAR);
                                if (tlv_phone){
                                    set_str(&data->Cellular, *tlv_phone);
                                    string phone = trimPhone(*tlv_phone);
                                    QString phone_str = quoteChars(QString::fromUtf8(phone.c_str()), ",");
                                    phone_str += ",Private Cellular,";
                                    phone_str += number(CELLULAR).c_str();
                                    bChanged |= contact->setPhones(phone_str, NULL);
                                }else{
                                    set_str(&data->Cellular, NULL);
                                }
                                if ((unsigned)atol(str.c_str()) == getUin()) {
                                    log(L_DEBUG,"Own Uin in contact list");
                                    Event e(EventContactDeleted, contact);
                                    e.process();
                                    break;
                                }
                                if (bChanged){
                                    Event e(EventContactChanged, contact);
                                    e.process();
                                }
                                if ((data->InfoFetchTime == 0) && data->Uin)
                                    addFullInfoRequest(data->Uin);
                            }
                        }else{
                            bIgnoreTime = true;
                        }
                        break;
                    }
                case ICQ_GROUPS:{
                        if (str.size() == 0) break;
                        log(L_DEBUG, "group %s %u", str.c_str(), grp_id);
                        ListRequest *lr = findGroupListRequest(grp_id);
                        if (lr){
                            lr->icq_id = grp_id;
                        }else{
                            Group *grp;
                            ICQUserData *data = findGroup(grp_id, str.c_str(), grp);
                            data->IcqID = grp_id;
                            data->bChecked = true;
                            if (grp->getName() != QString::fromUtf8(str.c_str())){
                                grp->setName(QString::fromUtf8(str.c_str()));
                                Event e(EventGroupChanged, grp);
                                e.process();
                            }
                        }
                        break;
                    }
                case ICQ_VISIBLE_LIST:{
                        if (str.length()){
                            log(L_DEBUG, "Visible %s", str.c_str());
                            ListRequest *lr = findContactListRequest(str.c_str());
                            if (lr)
                                lr->visible_id = id;
                            if ((lr == NULL) || (lr->type != LIST_USER_DELETED)){
                                Contact *contact;
                                ICQUserData *data = findContact(str.c_str(), NULL, true, contact);
                                data->ContactVisibleId = id;
                                if ((lr == NULL) && (data->VisibleId != id)){
                                    data->VisibleId = id;
                                    Event e(EventContactChanged, contact);
                                    e.process();
                                }
                                if ((data->InfoFetchTime == 0) && data->Uin)
                                    addFullInfoRequest(data->Uin);
                            }
                        }
                        break;
                    }
                case ICQ_INVISIBLE_LIST:{
                        if (str.length()){
                            log(L_DEBUG, "Invisible %s", str.c_str());
                            ListRequest *lr = findContactListRequest(str.c_str());
                            if (lr)
                                lr->invisible_id = id;
                            if ((lr == NULL) || (lr->type != LIST_USER_DELETED)){
                                Contact *contact;
                                ICQUserData *data = findContact(str.c_str(), NULL, true, contact);
                                data->ContactInvisibleId = id;
                                if ((lr == NULL) && (data->InvisibleId != id)){
                                    data->InvisibleId = id;
                                    Event e(EventContactChanged, contact);
                                    e.process();
                                }
                                if ((data->InfoFetchTime == 0) && data->Uin)
                                    addFullInfoRequest(data->Uin);
                            }
                        }
                        break;
                    }
                case ICQ_IGNORE_LIST:{
                        if (str.length()){
                            log(L_DEBUG, "Ignore %s", str.c_str());
                            ListRequest *lr = findContactListRequest(str.c_str());
                            if (lr)
                                lr->ignore_id = id;
                            Contact *contact;
                            ICQUserData *data = findContact(str.c_str(), NULL, true, contact);
                            if (data->IgnoreId != id){
                                data->IgnoreId = id;
                                if (lr == NULL){
                                    contact->setIgnore(true);
                                    Event e(EventContactChanged, contact);
                                    e.process();
                                }
                            }
                        }
                        break;
                    }
                case ICQ_INVISIBLE_STATE:
                    setContactsInvisible(id);
                    break;
                case 0x0009:
                case 0x0011:
                case 0x0013:
                    break;
                default:
                    log(L_WARN,"Unknown roster type %04X", type);
                }
                if (inf) delete inf;
            }
            unsigned long time;
            m_socket->readBuffer >> time;
            if ((time == 0) && list_len && !bIgnoreTime)
                break;

            setContactsTime(time);

            Group *grp;
            ContactList::GroupIterator it_g;
            list<Group*> forRemove;
            while ((grp = ++it_g) != NULL){
                ICQUserData *data = (ICQUserData*)(grp->clientData.getData(this));

	string n;
	if (grp->id())
		n = grp->getName().local8Bit();
	log(L_DEBUG, "Check %u %s %X %u", grp->id(), n.c_str(), data, data ? data->bChecked : 0);

                
				if ((data == NULL) || data->bChecked)
                    continue;
                ListRequest *lr = findGroupListRequest(data->IcqID);
                if (lr)
                    continue;
                forRemove.push_back(grp);
            }
            for (list<Group*>::iterator it = forRemove.begin(); it != forRemove.end(); ++it){
                delete *it;
            }

            Contact *contact;
            ContactList::ContactIterator it_c;
            while ((contact = ++it_c) != NULL){
                ICQUserData *data;
                ClientDataIterator it_d(contact->clientData);
                bool bOther = false;
                unsigned long newGroup = 0;
                while ((data = (ICQUserData*)(++it_d)) != NULL){
                    if (it_d.client() != this){
                        bOther = true;
                        continue;
                    }
                    unsigned grpId = data->GrpId;
                    ContactList::GroupIterator it_g;
                    while ((grp = ++it_g) != NULL){
                        ICQUserData *data = (ICQUserData*)(grp->clientData.getData(this));
                        if (data && (data->IcqID == grpId))
                            break;
                    }
                    if (grp)
                        newGroup = grp->id();
                }
                if (newGroup != contact->getGroup()){
                    if ((newGroup == 0) && bOther){
                        addContactRequest(contact);
                    }else{
                        contact->setGroup(newGroup);
                    }
                    Event e(EventContactChanged, contact);
                    e.process();
                }
            }
        }
    case ICQ_SNACxLISTS_ROSTERxOK:	// FALLTHROUGH
        {
            log(L_DEBUG, "Rosters OK");
            snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_UNKNOWN);
            sendPacket();
            sendCapability();
            sendICMB(1, 11);
            sendICMB(0, 3);
            sendLogonStatus();
            sendClientReady();
            sendMessageRequest();
            if (getContactsInvisible() == 0){
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                sendPacket();
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_CREATE, true);
                m_socket->writeBuffer
                << 0x00000000L << 0x00000001L
                << 0x000400C8L << (unsigned short)0;
                sendPacket();
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                sendPacket();
            }
            break;
        }
    case ICQ_SNACxLISTS_ADDED:{
            string screen = m_socket->readBuffer.unpackScreen();
            messageReceived(new AuthMessage(MessageAdded), screen.c_str());
            break;
        }
    case ICQ_SNACxLISTS_AUTHxREQUEST:{
            string screen = m_socket->readBuffer.unpackScreen();
            string message;
            string charset;
            unsigned short have_charset;
            m_socket->readBuffer.unpackStr(message);
            m_socket->readBuffer >> have_charset;
            if (have_charset){
                m_socket->readBuffer.incReadPos(2);
                m_socket->readBuffer.unpackStr(charset);
            }
            log(L_DEBUG, "Auth request %s", screen.c_str());
            ICQAuthMessage *msg = new ICQAuthMessage(MessageICQAuthRequest);
            msg->setServerText(message.c_str());
            msg->setCharset(charset.c_str());
            messageReceived(msg, screen.c_str());
            Contact *contact;
            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
            if (data)
                data->WantAuth = true;
            break;
        }
    case ICQ_SNACxLISTS_AUTH:{
            string screen = m_socket->readBuffer.unpackScreen();
            char auth_ok;
            m_socket->readBuffer >> auth_ok;
            string message;
            string charset;
            unsigned short have_charset;
            m_socket->readBuffer.unpackStr(message);
            m_socket->readBuffer >> have_charset;
            if (have_charset){
                m_socket->readBuffer.incReadPos(2);
                m_socket->readBuffer.unpackStr(charset);
            }
            log(L_DEBUG, "Auth %u %s", auth_ok, screen.c_str());
            ICQAuthMessage *msg = new ICQAuthMessage(auth_ok ? MessageICQAuthGranted : MessageICQAuthRefused);
            msg->setServerText(message.c_str());
            msg->setCharset(charset.c_str());
            messageReceived(msg, screen.c_str());
            if (auth_ok){
                Contact *contact;
                ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
                if (data){
                    data->WaitAuth = false;
                    Event e(EventContactChanged, contact);
                    e.process();
                    addPluginInfoRequest(data->Uin, PLUGIN_QUERYxSTATUS);
                    addPluginInfoRequest(data->Uin, PLUGIN_QUERYxINFO);
                }
            }
            break;
        }
    case ICQ_SNACxLISTS_DONE:
        if (m_listRequest && m_listRequest->seq() == seq){
            unsigned short res;
            m_socket->readBuffer >> res;
            log(L_DEBUG, "List request answer %u", res);
            m_listRequest->process(this, res);
            delete m_listRequest;
            m_listRequest = NULL;
            processListRequest();
        }
        break;
    default:
        log(L_WARN, "Unknown lists family type %04X", type);
    }
}

void ICQClient::listsRequest()
{
    log(L_DEBUG, "lists request");
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxRIGHTS);
    sendPacket();
    if (getContactsInvisible()){
        setContactsTime(0);
        setContactsLength(0);
    }
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxROSTER);
    unsigned long	contactsTime	= getContactsTime();
    unsigned short	contactsLength	= getContactsLength();
    m_socket->writeBuffer << contactsTime << contactsLength;
    sendPacket();
}

void ICQClient::sendVisibleList()
{
    if (getContactsInvisible() == 0)
        setContactsInvisible(rand() & 0x7FFF);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    m_socket->writeBuffer
    << 0x00000000L << getContactsInvisible()
    << (unsigned short)0x0004
    << (unsigned short)0x0005
    << 0x00CA0001L
    << (char)3;
    sendPacket();
}

void ICQClient::sendInvisibleList()
{
    if (getContactsInvisible() == 0)
        setContactsInvisible(rand() & 0x7FFF);
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    m_socket->writeBuffer
    << 0x00000000L << getContactsInvisible()
    << (unsigned short)0x0004
    << (unsigned short)0x0005
    << 0x00CA0001L
    << (char)4;
    sendPacket();
}

ListRequest *ICQClient::findContactListRequest(const char *screen)
{
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if ((((*it).type == LIST_USER_CHANGED) || ((*it).type == LIST_USER_DELETED)) &&
                ((*it).screen == screen))
            return &(*it);
    }
    return NULL;
}

ListRequest *ICQClient::findGroupListRequest(unsigned short id)
{
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (((*it).type == LIST_GROUP_DELETED) && ((*it).grp_id = id))
            return &(*it);
        if ((*it).type == LIST_GROUP_CHANGED){
            Group *group;
            ContactList::GroupIterator grp_it;
            while ((group = ++grp_it) != NULL){
                if (group->id() == 0)
                    continue;
                ICQUserData *data = (ICQUserData*)(group->clientData.getData(this));
                if (data && (data->IcqID == id))
                    return &(*it);
            }
        }
    }
    return NULL;
}

void ICQClient::clearListServerRequest()
{
    if (m_listRequest){
        delete m_listRequest;
        m_listRequest = NULL;
    }
}

//-----------------------------------------------------------------------------

GroupServerRequest::GroupServerRequest(unsigned short seq, unsigned long id, unsigned short icq_id, const char *name)
        : ListServerRequest(seq)
{
    m_id	= id;
    m_icqId = icq_id;
    if (name)
        m_name = name;
}

void GroupServerRequest::process(ICQClient *client, unsigned short)
{
    ListRequest *lr = client->findGroupListRequest(m_icqId);
    if (lr && (lr->type == LIST_GROUP_DELETED)){
        lr->icq_id = 0;
        return;
    }
    Group *group = getContacts()->group(m_id);
    if (group == NULL)
        return;
    ICQUserData *data = (ICQUserData*)(group->clientData.getData(client));
    if (data == NULL)
        data = (ICQUserData*)(group->clientData.createData(client));
    data->IcqID = m_icqId;
    set_str(&data->Alias, m_name.c_str());
}

//-----------------------------------------------------------------------------

ContactServerRequest::ContactServerRequest(unsigned short seq, const char *screen,
        unsigned short icq_id, unsigned short grp_id, TlvList *tlv)
        : ListServerRequest(seq)
{
    m_screen	= screen;
    m_icqId		= icq_id;
    m_grpId		= grp_id;
    m_tlv		= tlv;
}

ContactServerRequest::~ContactServerRequest()
{
    if (m_tlv)
        delete m_tlv;
}

void ContactServerRequest::process(ICQClient *client, unsigned short res)
{
    ListRequest *lr = client->findContactListRequest(m_screen.c_str());
    if (lr && (lr->type == LIST_USER_DELETED)){
        lr->screen = "";
        lr->icq_id = 0;
        lr->grp_id = 0;
        return;
    }
    Contact *contact;
    ICQUserData *data = client->findContact(m_screen.c_str(), NULL, true, contact);
    if ((res == 0x0E) && !data->WaitAuth){
        data->WaitAuth = true;
        Event e(EventContactChanged, contact);
        e.process();
        return;
    }
    data->IcqID = m_icqId;
    data->GrpId = m_grpId;
    if ((data->GrpId == 0) && data->WaitAuth){
        data->WaitAuth = false;
        Event e(EventContactChanged, contact);
        e.process();
    }
    if (m_tlv){
        Tlv *tlv_alias = (*m_tlv)(TLV_ALIAS);
        if (tlv_alias){
            set_str(&data->Alias, *tlv_alias);
        }else{
            set_str(&data->Alias, NULL);
        }
        Tlv *tlv_cell = (*m_tlv)(TLV_CELLULAR);
        if (tlv_cell){
            set_str(&data->Cellular, *tlv_cell);
        }else{
            set_str(&data->Cellular, NULL);
        }
    }
}

//-----------------------------------------------------------------------------

SetListRequest::SetListRequest(unsigned short seq, const char *screen,
                               unsigned short icq_id, unsigned short type)
        : ListServerRequest(seq)
{
    m_screen	= screen;
    m_icqId		= icq_id;
    m_type		= type;
}

void SetListRequest::process(ICQClient *client, unsigned short)
{
    ListRequest *lr = client->findContactListRequest(m_screen.c_str());
    if (lr && (lr->type == LIST_USER_DELETED)){
        switch (m_type){
        case ICQ_VISIBLE_LIST:
            lr->visible_id = 0;
            break;
        case ICQ_INVISIBLE_LIST:
            lr->invisible_id = 0;
            break;
        case ICQ_IGNORE_LIST:
            lr->ignore_id = 0;
            break;
        }
        return;
    }
    Contact *contact;
    ICQUserData *data = client->findContact(m_screen.c_str(), NULL, true, contact);
    switch (m_type){
    case ICQ_VISIBLE_LIST:
        data->ContactVisibleId = m_icqId;
        break;
    case ICQ_INVISIBLE_LIST:
        data->ContactInvisibleId = m_icqId;
        break;
    case ICQ_IGNORE_LIST:
        data->IgnoreId = m_icqId;
        break;
    }
}

//-----------------------------------------------------------------------------

unsigned short ICQClient::getListId()
{
    time_t now;
    time(&now);
    unsigned short id;
    for (id = now & 0x7FFF;;id++){
        id &= 0x7FFF;
        if (id == 0) continue;
        Group *group;
        ContactList::GroupIterator it_grp;
        while ((group = ++it_grp) != NULL){
            ICQUserData *data = (ICQUserData*)(group->clientData.getData(this));
            if (data == NULL)
                continue;
            if (data->IcqID == id)
                break;
        }
        if (group)
            continue;
        Contact *contact;
        ContactList::ContactIterator it_cnt;
        while ((contact = ++it_cnt) != NULL){
            ClientDataIterator it(contact->clientData, this);
            ICQUserData *data;
            while ((data = (ICQUserData*)(++it)) != NULL){
                if ((data->IcqID == id) || (data->IgnoreId == id) ||
                        (data->VisibleId == id) || (data->InvisibleId == id))
                    break;
            }
            if (data)
                break;
        }
        if (contact)
            continue;
        break;
    }
    return id;
}

TlvList *ICQClient::createListTlv(ICQUserData *data, Contact *contact)
{
    TlvList *tlv = new TlvList;
    QCString name = contact->getName().utf8();
    *tlv + new Tlv(TLV_ALIAS, name.length(), name);
    if (data->WaitAuth)
        *tlv + new Tlv(TLV_WAIT_AUTH, 0, NULL);
    string cell = getUserCellular(contact);
    if (cell.length())
        *tlv + new Tlv(TLV_CELLULAR, cell.length(), cell.c_str());
    return tlv;
}

unsigned short ICQClient::sendRoster(unsigned short cmd, const char *name, unsigned short grp_id,
                                     unsigned short usr_id, unsigned short subCmd, TlvList *tlv)
{
    snac(ICQ_SNACxFAM_LISTS, cmd, true);
    string sName;
    if (name) sName = name;
    m_socket->writeBuffer.pack(sName);
    m_socket->writeBuffer
    << grp_id
    << usr_id
    << subCmd;
    if (tlv){
        m_socket->writeBuffer << *tlv;
    }else{
        m_socket->writeBuffer << (unsigned short)0;
    }
    sendPacket();
    return m_nMsgSequence;
}

void ICQClient::sendRosterGrp(const char *name, unsigned short grpId, unsigned short usrId)
{
    string sName;
    if (name)
        sName = name;
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_RENAME, true);
    m_socket->writeBuffer.pack(sName);
    m_socket->writeBuffer
    << grpId
    << (unsigned long) ICQ_GROUPS;
    if (usrId){
        m_socket->writeBuffer
        << (unsigned short) 6
        << (unsigned short) 0xC8
        << (unsigned short) 2
        << (unsigned short) usrId;
    }else{
        m_socket->writeBuffer
        << (unsigned short) 4
        << (unsigned short) 0xC8
        << (unsigned short) 0;
    }
    sendPacket();
}

static string userStr(Contact *contact, ICQUserData *data)
{
    string res;
    char buf[20];
    sprintf(buf, "%lu [", data->Uin);
    res += buf;
    if (!contact->getName().isEmpty())
        res += contact->getName().local8Bit();
    res += "]";
    return res;
}

void ICQClient::processListRequest()
{
    if (m_listRequest || (getState() != Connected))
        return;
    for (;;){
        if (listRequests.size() == 0)
            return;
        ListRequest &lr = listRequests.front();
        string name;
        unsigned short seq = 0;
        unsigned short icq_id;
        Group *group = NULL;
        Contact *contact;
        ICQUserData *data;
        unsigned short grp_id = 0;
        switch (lr.type){
        case LIST_USER_CHANGED:
            data = findContact(lr.screen.c_str(), NULL, false, contact);
            if (data == NULL)
                break;
            if (data->VisibleId != data->ContactVisibleId){
                if ((data->VisibleId == 0) || (data->ContactVisibleId == 0)){
                    if (data->VisibleId){
                        log(L_DEBUG, "%s add to visible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), 0, data->VisibleId, ICQ_VISIBLE_LIST);
                    }else{
                        log(L_DEBUG, "%s remove from visible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data).c_str(), 0, data->ContactVisibleId, ICQ_VISIBLE_LIST);
                    }
                    m_listRequest = new SetListRequest(seq, screen(data).c_str(), data->VisibleId, ICQ_VISIBLE_LIST);
                    break;
                }
                data->VisibleId = data->ContactVisibleId;
            }
            if (data->InvisibleId != data->ContactInvisibleId){
                if ((data->InvisibleId == 0) || (data->ContactInvisibleId == 0)){
                    if (data->InvisibleId){
                        log(L_DEBUG, "%s add to invisible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), 0, data->InvisibleId, ICQ_INVISIBLE_LIST);
                    }else{
                        log(L_DEBUG, "%s remove from invisible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data).c_str(), 0, data->ContactInvisibleId, ICQ_INVISIBLE_LIST);
                    }
                    m_listRequest = new SetListRequest(seq, screen(data).c_str(), data->InvisibleId, ICQ_INVISIBLE_LIST);
                    break;
                }
                data->InvisibleId = data->ContactInvisibleId;
            }
            if (contact->getIgnore() != (data->IgnoreId != 0)){
                unsigned short ignore_id = 0;
                if (data->IgnoreId){
                    log(L_DEBUG, "%s remove from ignore list", userStr(contact, data).c_str());
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data).c_str(), 0, data->IgnoreId, ICQ_IGNORE_LIST);
                }else{
                    ignore_id = getListId();
                    log(L_DEBUG, "%s add to ignore list", userStr(contact, data).c_str());
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), 0, ignore_id, ICQ_IGNORE_LIST);
                }
                m_listRequest = new SetListRequest(seq, screen(data).c_str(), ignore_id, ICQ_IGNORE_LIST);
                break;
            }
            if (contact->getGroup()){
                group = getContacts()->group(contact->getGroup());
                ICQUserData *grp_data = (ICQUserData*)(group->clientData.getData(this));
                if (grp_data)
                    grp_id = grp_data->IcqID;
            }
            if (data->GrpId != grp_id){
                if (grp_id){
                    if (data->GrpId == 0){
                        snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_CREATE_USER);
                        m_socket->writeBuffer.packScreen(screen(data).c_str());
                        m_socket->writeBuffer << 0x00000000L;
                        sendPacket();
                    }
                    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                    sendPacket();
                    if (data->IcqID == 0)
                        data->IcqID = getListId();
                    TlvList *tlv = createListTlv(data, contact);
                    if (data->GrpId)
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", data->GrpId, data->IcqID);
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), grp_id, data->IcqID, 0, tlv);
                    sendRosterGrp(group->getName().utf8(), grp_id, data->IcqID);
                    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                    sendPacket();
                    log(L_DEBUG, "%s move to group %s", userStr(contact, data).c_str(), (const char*)group->getName().local8Bit());
                    m_listRequest = new ContactServerRequest(seq, screen(data).c_str(), data->IcqID, grp_id, tlv);
                }else{
                    log(L_DEBUG, "%s remove from contact list", userStr(contact, data).c_str());
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", data->GrpId, data->IcqID);
                    m_listRequest = new ContactServerRequest(seq, screen(data).c_str(), 0, 0);
                }
                break;
            }
            if (data->IcqID == 0)
                break;
            if (isContactRenamed(data, contact)){
                log(L_DEBUG, "%s rename", userStr(contact, data).c_str());
                TlvList *tlv = createListTlv(data, contact);
                seq = sendRoster(ICQ_SNACxLISTS_RENAME, screen(data).c_str(), data->GrpId, data->IcqID, 0, tlv);
                m_listRequest = new ContactServerRequest(seq, screen(data).c_str(), data->IcqID, data->GrpId, tlv);
                break;
            }
            break;
        case LIST_USER_DELETED:
            if (lr.visible_id){
                log(L_DEBUG, "%s remove from visible list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen.c_str(), 0, lr.visible_id, ICQ_VISIBLE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen.c_str(), 0, ICQ_VISIBLE_LIST);
                break;
            }
            if (lr.invisible_id){
                log(L_DEBUG, "%s remove from invisible list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen.c_str(), 0, lr.invisible_id, ICQ_INVISIBLE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen.c_str(), 0, ICQ_INVISIBLE_LIST);
                break;
            }
            if (lr.ignore_id){
                log(L_DEBUG, "%s remove from ignore list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen.c_str(), 0, lr.ignore_id, ICQ_IGNORE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen.c_str(), 0, ICQ_IGNORE_LIST);
                break;
            }
            if (lr.screen.length()){
                log(L_DEBUG, "%s remove from contact list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", lr.grp_id, lr.icq_id);
                m_listRequest = new ContactServerRequest(seq, lr.screen.c_str(), 0, 0);
            }
            break;
        case LIST_GROUP_CHANGED:
            group = getContacts()->group(atol(lr.screen.c_str()));
            if (group){
                name = group->getName().utf8();
                data = (ICQUserData*)(group->clientData.getData(this));
                if (data){
                    icq_id = data->IcqID;
                    string alias;
                    if (data->Alias)
                        alias = data->Alias;
                    if (alias != name){
                        log(L_DEBUG, "rename group %s", (const char*)group->getName().local8Bit());
                        seq = sendRoster(ICQ_SNACxLISTS_RENAME, name.c_str(), icq_id, 0, ICQ_GROUPS);
                    }
                }else{
                    log(L_DEBUG, "create group %s", (const char*)group->getName().local8Bit());
                    icq_id = getListId();
                    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                    sendPacket();
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, name.c_str(), icq_id, 0, ICQ_GROUPS);
                    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                    sendPacket();
                }
                if (seq)
                    m_listRequest = new GroupServerRequest(seq, group->id(), icq_id, name.c_str());
            }
            break;
        case LIST_GROUP_DELETED:
            if (lr.icq_id){
                log(L_DEBUG, "delete group");
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                sendPacket();
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", lr.icq_id, 0, ICQ_GROUPS);
                snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                sendPacket();
                m_listRequest = new GroupServerRequest(seq, 0, lr.icq_id, name.c_str());
            }
            break;
        }
        if (m_listRequest)
            return;
        listRequests.erase(listRequests.begin());
    }
}

void ICQClient::addGroupRequest(Group *group)
{
    string name;
    name = group->getName().utf8();
    ICQUserData *data = (ICQUserData*)(group->clientData.getData(this));
    if (data == NULL){
        list<ListRequest>::iterator it;
        for (it = listRequests.begin(); it != listRequests.end(); it++){
            if ((*it).type != LIST_GROUP_CHANGED)
                continue;
            if ((unsigned)atol((*it).screen.c_str()) == group->id())
                return;
        }
        ListRequest lr;
        lr.type   = LIST_GROUP_CHANGED;
        lr.screen = number(group->id());
        listRequests.push_back(lr);
        processListRequest();
        return;
    }
    list<ListRequest>::iterator it;
    for (it = listRequests.begin(); it != listRequests.end(); it++){
        if ((*it).type != LIST_GROUP_CHANGED)
            continue;
        if ((*it).icq_id == data->IcqID)
            return;
    }
    string alias;
    if (data->Alias)
        alias = data->Alias;
    if (alias == name)
        return;
    ListRequest lr;
    lr.type = LIST_GROUP_CHANGED;
    lr.icq_id  = data->IcqID;
    lr.screen  = number(group->id());
    listRequests.push_back(lr);
    processListRequest();
}

void ICQClient::addContactRequest(Contact *contact)
{
    ICQUserData *data;
    ClientDataIterator it(contact->clientData, this);
    while ((data = (ICQUserData*)(++it)) != NULL){
        list<ListRequest>::iterator it;
        for (it = listRequests.begin(); it != listRequests.end(); it++){
            if ((*it).type != LIST_USER_CHANGED)
                continue;
            if ((*it).screen == screen(data))
                return;
        }

        bool bChanged = false;
        if (data->VisibleId != data->ContactVisibleId){
            if ((data->VisibleId == 0) || (data->ContactVisibleId == 0)){
                bChanged = true;
                log(L_DEBUG, "%s change visible state", userStr(contact, data).c_str());
            }else{
                data->VisibleId = data->ContactVisibleId;
            }
        }
        if (data->InvisibleId != data->ContactInvisibleId){
            if ((data->InvisibleId == 0) || (data->ContactInvisibleId == 0)){
                bChanged = true;
                log(L_DEBUG, "%s change invisible state", userStr(contact, data).c_str());
            }else{
                data->InvisibleId = data->ContactInvisibleId;
            }
        }
        if (contact->getIgnore() != (data->IgnoreId != 0)){
            log(L_DEBUG, "%s change ignore state", userStr(contact, data).c_str());
            bChanged = true;
        }
        if (!bChanged){
            unsigned short grp_id = 0;
            if (contact->getGroup()){
                Group *group = getContacts()->group(contact->getGroup());
                if (group){
                    ICQUserData *grp_data = (ICQUserData*)(group->clientData.getData(this));
                    if (grp_data){
                        grp_id = grp_data->IcqID;
                    }else{
                        addGroupRequest(group);
                    }
                }
            }
            if (data->GrpId != grp_id){
                log(L_DEBUG, "%s change group %u->%u", userStr(contact, data).c_str(), data->GrpId, grp_id);
                bChanged = true;
            }
            if (!bChanged && (data->IcqID == 0))
                return;
            if (!bChanged && !isContactRenamed(data, contact))
                return;
        }

        ListRequest lr;
        lr.type   = LIST_USER_CHANGED;
        lr.screen = screen(data);
        listRequests.push_back(lr);
        processListRequest();
    }
}

bool ICQClient::isContactRenamed(ICQUserData *data, Contact *contact)
{
    string name;
    name = contact->getName().utf8();
    string alias;
    if (data->Alias){
        alias = data->Alias;
    }else{
        char b[20];
        sprintf(b, "%lu", data->Uin);
        alias = b;
    }
    if (name != alias){
        log(L_DEBUG, "%u renamed %s->%s", data->Uin, alias.c_str(), name.c_str());
        return true;
    }
    string cell = getUserCellular(contact);
    string phone;
    if (data->Cellular)
        phone = data->Cellular;
    if (cell != phone){
        log(L_DEBUG, "%s phone changed %s->%s", userStr(contact, data).c_str(), phone.c_str(), cell.c_str());
        return true;
    }
    return false;
}

string ICQClient::getUserCellular(Contact *contact)
{
    string res;
    QString phones = contact->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        QString phone = getToken(phoneItem, '/', false);
        if (phoneItem != "-")
            continue;
        QString value = getToken(phone, ',');
        getToken(phone, ',');
        if (phone.toUInt() == CELLULAR){
            res = value.utf8();
            return res;
        }
    }
    return res;
}

bool ICQClient::sendAuthRequest(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    ICQUserData *data = (ICQUserData*)_data;

    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQUEST_AUTH);
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    string message;
    string charset;
    if (hasCap(data, CAP_RTF) || hasCap(data, CAP_UTF)){
        message = msg->getPlainText().utf8();
        charset = "utf-8";
    }else{
        message = fromUnicode(msg->getPlainText(), data);
    }
    m_socket->writeBuffer
    << (unsigned short)(message.length())
    << message.c_str()
    << (char)0x00;
    if (charset.empty()){
        m_socket->writeBuffer << (char)0x00;
    }else{
        m_socket->writeBuffer
        << (char)0x01
        << (unsigned short)1
        << (unsigned short)(charset.length())
        << charset.c_str();
    }
    sendPacket();

    msg->setClient(dataName(data).c_str());
    Event eSent(EventSent, msg);
    eSent.process();
    Event e(EventMessageSent, msg);
    e.process();
    delete msg;
    return true;
}

bool ICQClient::sendAuthGranted(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    data->WantAuth = false;

    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_AUTHxSEND);
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    m_socket->writeBuffer
    << (char)0x01
    << (unsigned long)0;
    sendPacket();

    msg->setClient(dataName(data).c_str());
    Event eSent(EventSent, msg);
    eSent.process();
    Event e(EventMessageSent, msg);
    e.process();
    delete msg;
    return true;
}

bool ICQClient::sendAuthRefused(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    data->WantAuth = false;

    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_AUTHxSEND);
    m_socket->writeBuffer.packScreen(screen(data).c_str());

    string message;
    string charset;
    if (hasCap(data, CAP_RTF) || hasCap(data, CAP_UTF)){
        message = msg->getPlainText().utf8();
        charset = "utf-8";
    }else{
        message = fromUnicode(msg->getPlainText(), data);
    }
    m_socket->writeBuffer
    << (char) 0
    << (unsigned short)(message.length())
    << message.c_str()
    << (char)0x00;
    if (charset.empty()){
        m_socket->writeBuffer << (char)0x00;
    }else{
        m_socket->writeBuffer << (char)0x01
        << (unsigned short)1
        << (unsigned short)(charset.length())
        << charset.c_str();
    }
    sendPacket();

    msg->setClient(dataName(data).c_str());
    Event eSent(EventSent, msg);
    eSent.process();
    Event e(EventMessageSent, msg);
    e.process();
    delete msg;
    return true;
}
