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
#include "core.h"

#include <stdio.h>
#include <time.h>
#include <vector>

using namespace std;

#include <qtimer.h>

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
const unsigned short ICQ_SNACxLISTS_FUTURE_AUTH    = 0x0014;
const unsigned short ICQ_SNACxLISTS_FUTURE_GRANT   = 0x0015;
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

const unsigned LIST_REQUEST_TIMEOUT = 50;

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

static char PLEASE_UPGRADE[] = "PleaseUpgrade";

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
                        if (data->IcqID.value == 0){
                            data->bChecked.bValue = true;
                            continue;
                        }
                        data->bChecked.bValue = false;
                    }
                }
                Contact *contact;
                ContactList::ContactIterator it_c;
                while ((contact = ++it_c) != NULL){
                    ICQUserData *data;
                    ClientDataIterator it(contact->clientData, this);
                    while ((data = (ICQUserData*)(++it)) != NULL){
                        data->bChecked.bValue = false;
                        data->GrpId.value = 0;
                        data->IgnoreId.value = 0;
                        data->VisibleId.value = 0;
                        data->InvisibleId.value = 0;
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
                            if ((str.length() == strlen(PLEASE_UPGRADE) + 3) &&
                                    (str.substr(0, strlen(PLEASE_UPGRADE)) == PLEASE_UPGRADE)){
                                log(L_DEBUG, "Upgrade warning");
                                continue;
                            }
                            log(L_DEBUG, "User %s", str.c_str());
                            // check for own uin in contact list
                            if (!m_bAIM && ((unsigned)atol(str.c_str()) == getUin())) {
                                log(L_DEBUG, "Own Uin in contact list - removing!");
                                seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", grp_id, id);
                                m_listRequest = new ContactServerRequest(seq, number(id).c_str(), 0, 0);
                                time((time_t*)&m_listRequestTime);
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
                                    data->IcqID.value = id;
                                    data->GrpId.value = grp_id;
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
                                    if (!data->WaitAuth.bValue){
                                        data->WaitAuth.bValue = true;
                                        bChanged = true;
                                    }
                                } else
                                    /* if not TLV(WAIT_AUTH) we are authorized ... */
                                    if (inf && !(*inf)(TLV_WAIT_AUTH)) {
                                        if (data->WaitAuth.bValue){
                                            data->WaitAuth.bValue = false;
                                            bChanged = true;
                                        }
                                    }
                                data->IcqID.value = id;
                                data->GrpId.value = grp_id;
                                Tlv *tlv_phone = NULL;
                                if (inf) tlv_phone = (*inf)(TLV_CELLULAR);
                                if (tlv_phone){
                                    set_str(&data->Cellular.ptr, *tlv_phone);
                                    string phone = trimPhone(*tlv_phone);
                                    QString phone_str = quoteChars(QString::fromUtf8(phone.c_str()), ",");
                                    phone_str += ",Private Cellular,";
                                    phone_str += number(CELLULAR).c_str();
                                    bChanged |= contact->setPhones(phone_str, NULL);
                                }else{
                                    set_str(&data->Cellular.ptr, NULL);
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
                                if ((data->InfoFetchTime.value == 0) && data->Uin.value)
                                    addFullInfoRequest(data->Uin.value);
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
                        if (lr)
                            break;
                        Group *grp;
                        ICQUserData *data = findGroup(grp_id, str.c_str(), grp);
                        if (data->IcqID.value){
                            lr = findGroupListRequest((unsigned short)(data->IcqID.value));
                            if (lr)
                                removeListRequest(lr);
                        }
                        data->IcqID.value     = grp_id;
                        data->bChecked.bValue = true;
                        if (grp->getName() != QString::fromUtf8(str.c_str())){
                            grp->setName(QString::fromUtf8(str.c_str()));
                            Event e(EventGroupChanged, grp);
                            e.process();
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
                                data->ContactVisibleId.value = id;
                                if ((lr == NULL) && (data->VisibleId.value != id)){
                                    data->VisibleId.value = id;
                                    Event e(EventContactChanged, contact);
                                    e.process();
                                }
                                if ((data->InfoFetchTime.value == 0) && data->Uin.value)
                                    addFullInfoRequest(data->Uin.value);
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
                                data->ContactInvisibleId.value = id;
                                if ((lr == NULL) && (data->InvisibleId.value != id)){
                                    data->InvisibleId.value = id;
                                    Event e(EventContactChanged, contact);
                                    e.process();
                                }
                                if ((data->InfoFetchTime.value == 0) && data->Uin.value)
                                    addFullInfoRequest(data->Uin.value);
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
                            if (data->IgnoreId.value != id){
                                data->IgnoreId.value = id;
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
                log(L_DEBUG, "Check %u %s %X %u", grp->id(), n.c_str(), data, data ? data->bChecked.bValue : 0);
                if ((data == NULL) || data->bChecked.bValue)
                    continue;
                ListRequest *lr = findGroupListRequest((unsigned short)(data->IcqID.value));
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
                bool bOther = (contact->clientData.size() == 0);
                bool bMy = false;
                unsigned long newGroup = 0;
                while ((data = (ICQUserData*)(++it_d)) != NULL){
                    if (it_d.client() != this){
                        bOther = true;
                        continue;
                    }
                    unsigned grpId = data->GrpId.value;
                    ContactList::GroupIterator it_g;
                    while ((grp = ++it_g) != NULL){
                        ICQUserData *data = (ICQUserData*)(grp->clientData.getData(this));
                        if (data && (data->IcqID.value == grpId))
                            break;
                    }
                    if (grp)
                        newGroup = grp->id();
                    bMy = true;
                }
                if (newGroup != contact->getGroup()){
                    if ((newGroup == 0) && bOther){
                        if (bMy)
                            addContactRequest(contact);
                    }else{
                        contact->setGroup(newGroup);
                    }
                    Event e(EventContactChanged, contact);
                    e.process();
                }
            }
        }
        getContacts()->save();
    case ICQ_SNACxLISTS_ROSTERxOK:	// FALLTHROUGH
        {
            log(L_DEBUG, "Rosters OK");
            snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_UNKNOWN);
            sendPacket();
            QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
            setPreviousPassword(NULL);
            sendClientReady();
            if (m_bAIM){
                Group *grp;
                ContactList::GroupIterator it;
                while ((grp = ++it) != NULL){
                    if (grp->id())
                        break;
                }
                if (grp == NULL){
                    grp = getContacts()->group(0, true);
                    grp->setName("General");
                    Event e(EventGroupChanged, grp);
                    e.process();
                }
                time_t now;
                time(&now);
                data.owner.OnlineTime.value = now;
                if (m_logonStatus == STATUS_ONLINE){
                    m_status = STATUS_ONLINE;
                    sendCapability();
                    sendICMB(1, 11);
                    sendICMB(0, 11);
                    fetchProfiles();
                }else{
                    m_status = STATUS_AWAY;

                    ar_request req;
                    req.bDirect = false;
                    arRequests.push_back(req);

                    ARRequest ar;
                    ar.contact  = NULL;
                    ar.param    = &arRequests.back();
                    ar.receiver = this;
                    ar.status   = m_logonStatus;
                    Event eAR(EventARRequest, &ar);
                    eAR.process();
                }
                setState(Connected);
                processListRequest();
                break;
            }
            sendCapability();
            sendICMB(1, 11);
            sendICMB(0, 3);
            sendLogonStatus();
            setState(Connected);
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
            fetchProfiles();
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
            Message *m = NULL;
            if (charset.empty()){
                AuthMessage *msg = new AuthMessage(MessageAuthRequest);
                msg->setText(QString::fromUtf8(message.c_str()));
                m = msg;
            }else{
                ICQAuthMessage *msg = new ICQAuthMessage(MessageICQAuthRequest, MessageAuthRequest);
                msg->setServerText(message.c_str());
                msg->setCharset(charset.c_str());
                m = msg;
            }
            messageReceived(m, screen.c_str());
            Contact *contact;
            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
            if (data)
                data->WantAuth.bValue = true;
            break;
        }
    case ICQ_SNACxLISTS_FUTURE_GRANT:{
            /* we treat future grant as normal grant but it isn't the same...
               http://iserverd1.khstu.ru/oscar/snac_13_15.html */
            string screen = m_socket->readBuffer.unpackScreen();
            string message;
            Message *m = NULL;

            m_socket->readBuffer.unpackStr(message);
            AuthMessage *msg = new AuthMessage(MessageAuthGranted);
            msg->setText(QString::fromUtf8(message.c_str()));
            m = msg;
            messageReceived(m, screen.c_str());
            Contact *contact;
            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
            if (data){
                data->WaitAuth.bValue = false;
                Event e(EventContactChanged, contact);
                e.process();
                addPluginInfoRequest(data->Uin.value, PLUGIN_QUERYxSTATUS);
                addPluginInfoRequest(data->Uin.value, PLUGIN_QUERYxINFO);
            }
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
            Message *m = NULL;
            if (charset.empty()){
                AuthMessage *msg = new AuthMessage(auth_ok ? MessageAuthGranted : MessageAuthRefused);
                msg->setText(QString::fromUtf8(message.c_str()));
                m = msg;
            }else{
                ICQAuthMessage *msg = new ICQAuthMessage(auth_ok ? MessageICQAuthGranted : MessageICQAuthRefused, auth_ok ? MessageAuthGranted : MessageAuthRefused);
                msg->setServerText(message.c_str());
                msg->setCharset(charset.c_str());
                m = msg;
            }
            messageReceived(m, screen.c_str());
            if (auth_ok){
                Contact *contact;
                ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
                if (data){
                    data->WaitAuth.bValue = false;
                    Event e(EventContactChanged, contact);
                    e.process();
                    addPluginInfoRequest(data->Uin.value, PLUGIN_QUERYxSTATUS);
                    addPluginInfoRequest(data->Uin.value, PLUGIN_QUERYxINFO);
                }
            }
            break;
        }
    case ICQ_SNACxLISTS_DONE:
        if (m_listRequest && m_listRequest->seq() == seq){
            unsigned short res;
            const char *msg;
            m_socket->readBuffer >> res;
            switch (res) {
            case 0x00:
                msg = "No errors (success)";
                break;
            case 0x02:
                msg = "Item you want to modify not found in list";
                break;
            case 0x03:
                msg = "Item you want to add allready exists";
                break;
            case 0x0a:
                msg = "Error adding item (invalid id, allready in list, invalid data)";
                break;
            case 0x0c:
                msg = "Can't add item. Limit for this type of items exceeded";
                break;
            case 0x0d:
                msg = "Trying to add ICQ contact to an AIM list";
                break;
            case 0x0e:
                msg = "Can't add this contact because it requires authorization";
                break;
            default:
                msg = NULL;
            }
            if (msg)
                log(L_DEBUG, msg);
            else
                log(L_DEBUG, "Unknown list request answer %u", res);
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
    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_REQxROSTER);
    unsigned long	contactsTime	= getContactsTime();
    unsigned short	contactsLength	= getContactsLength();
    m_socket->writeBuffer << contactsTime << contactsLength;
    sendPacket();
}

void ICQClient::sendVisibleList()
{
    if (getContactsInvisible() == 0)
        setContactsInvisible((unsigned short)(get_random() & 0x7FFF));
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
        setContactsInvisible((unsigned short)(get_random() & 0x7FFF));
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
        switch ((*it).type){
        case LIST_GROUP_DELETED:
        case LIST_GROUP_CHANGED:
            if ((*it).icq_id == id)
                return &(*it);
            break;
        }
    }
    return NULL;
}

void ICQClient::removeListRequest(ListRequest *lr)
{
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (&(*it) == lr){
            listRequests.erase(it);
            return;
        }
    }
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
    data->IcqID.value = m_icqId;
    set_str(&data->Alias.ptr, m_name.c_str());
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
    if ((res == 0x0E) && !data->WaitAuth.bValue){
        data->WaitAuth.bValue = true;
        Event e(EventContactChanged, contact);
        e.process();
        return;
    }
    data->IcqID.value = m_icqId;
    data->GrpId.value = m_grpId;
    if ((data->GrpId.value == 0) && data->WaitAuth.bValue){
        data->WaitAuth.bValue = false;
        Event e(EventContactChanged, contact);
        e.process();
    }
    if (m_tlv){
        Tlv *tlv_alias = (*m_tlv)(TLV_ALIAS);
        if (tlv_alias){
            set_str(&data->Alias.ptr, *tlv_alias);
        }else{
            set_str(&data->Alias.ptr, NULL);
        }
        Tlv *tlv_cell = (*m_tlv)(TLV_CELLULAR);
        if (tlv_cell){
            set_str(&data->Cellular.ptr, *tlv_cell);
        }else{
            set_str(&data->Cellular.ptr, NULL);
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
        data->ContactVisibleId.value = m_icqId;
        break;
    case ICQ_INVISIBLE_LIST:
        data->ContactInvisibleId.value = m_icqId;
        break;
    case ICQ_IGNORE_LIST:
        data->IgnoreId.value = m_icqId;
        break;
    }
}

//-----------------------------------------------------------------------------

unsigned short ICQClient::getListId()
{
    unsigned short id;
    for (id = (unsigned short)(get_random() & 0x7FFF) ;; id++){
        id &= 0x7FFF;
        if (id == 0) continue;
        Group *group;
        ContactList::GroupIterator it_grp;
        while ((group = ++it_grp) != NULL){
            ICQUserData *data = (ICQUserData*)(group->clientData.getData(this));
            if (data == NULL)
                continue;
            if (data->IcqID.value == id)
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
                if ((data->IcqID.value == id) || (data->IgnoreId.value == id) ||
                        (data->VisibleId.value == id) || (data->InvisibleId.value == id))
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
    *tlv + new Tlv(TLV_ALIAS, (unsigned short)(name.length()), name);
    if (data->WaitAuth.bValue)
        *tlv + new Tlv(TLV_WAIT_AUTH, 0, NULL);
    string cell = getUserCellular(contact);
    if (cell.length())
        *tlv + new Tlv(TLV_CELLULAR, (unsigned short)(cell.length()), cell.c_str());
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
    sprintf(buf, "%lu [", data->Uin.value);
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
            if (data->VisibleId.value != data->ContactVisibleId.value){
                if ((data->VisibleId.value == 0) || (data->ContactVisibleId.value == 0)){
                    if (data->VisibleId.value){
                        log(L_DEBUG, "%s add to visible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), 0, (unsigned short)(data->VisibleId.value), ICQ_VISIBLE_LIST);
                    }else{
                        log(L_DEBUG, "%s remove from visible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data).c_str(), 0, (unsigned short)(data->ContactVisibleId.value), ICQ_VISIBLE_LIST);
                    }
                    m_listRequest = new SetListRequest(seq, screen(data).c_str(), (unsigned short)(data->VisibleId.value), ICQ_VISIBLE_LIST);
                    time((time_t*)&m_listRequestTime);
                    break;
                }
                data->VisibleId.value = data->ContactVisibleId.value;
            }
            if (data->InvisibleId.value != data->ContactInvisibleId.value){
                if ((data->InvisibleId.value == 0) || (data->ContactInvisibleId.value == 0)){
                    if (data->InvisibleId.value){
                        log(L_DEBUG, "%s add to invisible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), 0, (unsigned short)(data->InvisibleId.value), ICQ_INVISIBLE_LIST);
                    }else{
                        log(L_DEBUG, "%s remove from invisible list", userStr(contact, data).c_str());
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data).c_str(), 0, (unsigned short)(data->ContactInvisibleId.value), ICQ_INVISIBLE_LIST);
                    }
                    m_listRequest = new SetListRequest(seq, screen(data).c_str(), (unsigned short)(data->InvisibleId.value), ICQ_INVISIBLE_LIST);
                    time((time_t*)&m_listRequestTime);
                    break;
                }
                data->InvisibleId.value = data->ContactInvisibleId.value;
            }
            if (contact->getIgnore() != (data->IgnoreId.value != 0)){
                unsigned short ignore_id = 0;
                if (data->IgnoreId.value){
                    log(L_DEBUG, "%s remove from ignore list", userStr(contact, data).c_str());
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data).c_str(), 0, (unsigned short)(data->IgnoreId.value), ICQ_IGNORE_LIST);
                }else{
                    ignore_id = getListId();
                    log(L_DEBUG, "%s add to ignore list", userStr(contact, data).c_str());
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), 0, ignore_id, ICQ_IGNORE_LIST);
                }
                m_listRequest = new SetListRequest(seq, screen(data).c_str(), ignore_id, ICQ_IGNORE_LIST);
                time((time_t*)&m_listRequestTime);
                break;
            }
            if (contact->getGroup()){
                group = getContacts()->group(contact->getGroup());
                ICQUserData *grp_data = (ICQUserData*)(group->clientData.getData(this));
                if (grp_data)
                    grp_id = (unsigned short)(grp_data->IcqID.value);
            }
            if (data->GrpId.value != grp_id){
                if (grp_id){
                    if (data->GrpId.value == 0){
                        snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_FUTURE_AUTH);
                        m_socket->writeBuffer.packScreen(screen(data).c_str());
                        m_socket->writeBuffer << 0x00000000L;
                        sendPacket();
                    }
                    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_EDIT);
                    sendPacket();
                    if (data->IcqID.value == 0)
                        data->IcqID.value = getListId();
                    TlvList *tlv = createListTlv(data, contact);
                    if (data->GrpId.value)
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", (unsigned short)(data->GrpId.value), (unsigned short)(data->IcqID.value));
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data).c_str(), grp_id, (unsigned short)(data->IcqID.value), 0, tlv);
                    sendRosterGrp(group->getName().utf8(), grp_id, (unsigned short)(data->IcqID.value));
                    snac(ICQ_SNACxFAM_LISTS, ICQ_SNACxLISTS_SAVE);
                    sendPacket();
                    log(L_DEBUG, "%s move to group %s", userStr(contact, data).c_str(), (const char*)group->getName().local8Bit());
                    m_listRequest = new ContactServerRequest(seq, screen(data).c_str(), (unsigned short)(data->IcqID.value), grp_id, tlv);
                    time((time_t*)&m_listRequestTime);
                }else{
                    log(L_DEBUG, "%s remove from contact list", userStr(contact, data).c_str());
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", (unsigned short)(data->GrpId.value), (unsigned short)(data->IcqID.value));
                    m_listRequest = new ContactServerRequest(seq, screen(data).c_str(), 0, 0);
                    time((time_t*)&m_listRequestTime);
                }
                break;
            }
            if (data->IcqID.value == 0)
                break;
            if (isContactRenamed(data, contact)){
                log(L_DEBUG, "%s rename", userStr(contact, data).c_str());
                TlvList *tlv = createListTlv(data, contact);
                seq = sendRoster(ICQ_SNACxLISTS_RENAME, screen(data).c_str(), (unsigned short)(data->GrpId.value), (unsigned short)(data->IcqID.value), 0, tlv);
                m_listRequest = new ContactServerRequest(seq, screen(data).c_str(), (unsigned short)(data->IcqID.value), (unsigned short)(data->GrpId.value), tlv);
                time((time_t*)&m_listRequestTime);
                break;
            }
            break;
        case LIST_USER_DELETED:
            if (lr.visible_id){
                log(L_DEBUG, "%s remove from visible list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen.c_str(), 0, lr.visible_id, ICQ_VISIBLE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen.c_str(), 0, ICQ_VISIBLE_LIST);
                time((time_t*)&m_listRequestTime);
                break;
            }
            if (lr.invisible_id){
                log(L_DEBUG, "%s remove from invisible list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen.c_str(), 0, lr.invisible_id, ICQ_INVISIBLE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen.c_str(), 0, ICQ_INVISIBLE_LIST);
                time((time_t*)&m_listRequestTime);
                break;
            }
            if (lr.ignore_id){
                log(L_DEBUG, "%s remove from ignore list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen.c_str(), 0, lr.ignore_id, ICQ_IGNORE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen.c_str(), 0, ICQ_IGNORE_LIST);
                time((time_t*)&m_listRequestTime);
                break;
            }
            if (lr.screen.length() && lr.grp_id){
                log(L_DEBUG, "%s remove from contact list", lr.screen.c_str());
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, "", lr.grp_id, lr.icq_id);
                m_listRequest = new ContactServerRequest(seq, lr.screen.c_str(), 0, 0);
                time((time_t*)&m_listRequestTime);
            }
            break;
        case LIST_GROUP_CHANGED:
            group = getContacts()->group(atol(lr.screen.c_str()));
            if (group){
                name = group->getName().utf8();
                data = (ICQUserData*)(group->clientData.getData(this));
                if (data){
                    icq_id = (unsigned short)(data->IcqID.value);
                    string alias;
                    if (data->Alias.ptr)
                        alias = data->Alias.ptr;
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
                time((time_t*)&m_listRequestTime);
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
                time((time_t*)&m_listRequestTime);
            }
            break;
        }
        if (m_listRequest)
            return;
        listRequests.erase(listRequests.begin());
    }
}

void ICQClient::checkListRequest()
{
    if (m_listRequest == NULL)
        return;
    unsigned now;
    time((time_t*)&now);
    if (now > m_listRequestTime + LIST_REQUEST_TIMEOUT){
        log(L_WARN, "List request timeout");
        m_listRequest->process(this, (unsigned short)(-1));
        delete m_listRequest;
        m_listRequest = NULL;
        processListRequest();
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
        if ((*it).icq_id == data->IcqID.value)
            return;
    }
    string alias;
    if (data->Alias.ptr)
        alias = data->Alias.ptr;
    if (alias == name)
        return;
    ListRequest lr;
    lr.type = LIST_GROUP_CHANGED;
    lr.icq_id  = (unsigned short)(data->IcqID.value);
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
        if (data->VisibleId.value != data->ContactVisibleId.value){
            if ((data->VisibleId.value == 0) || (data->ContactVisibleId.value == 0)){
                bChanged = true;
                log(L_DEBUG, "%s change visible state", userStr(contact, data).c_str());
            }else{
                data->VisibleId.value = data->ContactVisibleId.value;
            }
        }
        if (data->InvisibleId.value != data->ContactInvisibleId.value){
            if ((data->InvisibleId.value == 0) || (data->ContactInvisibleId.value == 0)){
                bChanged = true;
                log(L_DEBUG, "%s change invisible state", userStr(contact, data).c_str());
            }else{
                data->InvisibleId.value = data->ContactInvisibleId.value;
            }
        }
        if (contact->getIgnore() != (data->IgnoreId.value != 0)){
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
                        grp_id = (unsigned short)(grp_data->IcqID.value);
                    }else{
                        addGroupRequest(group);
                    }
                }
            }
            if (data->GrpId.value != grp_id){
                log(L_DEBUG, "%s change group %u->%u", userStr(contact, data).c_str(), data->GrpId, grp_id);
                bChanged = true;
            }
            if (!bChanged && (data->IcqID.value == 0))
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
    if (data->Alias.ptr){
        alias = data->Alias.ptr;
    }else{
        char b[20];
        sprintf(b, "%lu", data->Uin.value);
        alias = b;
    }
    if (name != alias){
        log(L_DEBUG, "%u renamed %s->%s", data->Uin, alias.c_str(), name.c_str());
        return true;
    }
    string cell = getUserCellular(contact);
    string phone;
    if (data->Cellular.ptr)
        phone = data->Cellular.ptr;
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
    data->WantAuth.bValue = false;

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
    data->WantAuth.bValue = false;

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
