/***************************************************************************
                          msn.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "msnclient.h"
#include "msnconfig.h"
#include "msnpacket.h"
#include "msn.h"
#include "msninfo.h"
#include "msnsearch.h"
#include "core.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifndef INADDR_NONE
#define INADDR_NONE     0xFFFFFFFF
#endif

#include <time.h>
#include <qtimer.h>
#include <qregexp.h>

#include <qfile.h>

using namespace std;

const unsigned long PING_TIMEOUT	= 60;

const unsigned long FT_TIMEOUT		= 60;
const unsigned long TYPING_TIME		= 10;
const unsigned MAX_FT_PACKET		= 2045;

/*
typedef struct MSNUserData
{
    char		*EMail;
    char		*ScreenName;
    unsigned	Status;
    unsigned	StatusTime;
    unsigned	OnlineTime;
	char		*PhoneHome;
	char		*PhoneWork;
	char		*PhoneMobile;
	unsigned	Mobile;
	unsigned	Group;
	unsigned	Flags;
	unsigned	sFlags;
} MSNUserData;
*/

static DataDef msnUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(3) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "Screen", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, DATA(1) },	// Status
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "PhoneHome", DATA_UTF, 1, 0 },
        { "PhoneWork", DATA_UTF, 1, 0 },
        { "PhoneMobile", DATA_UTF, 1, 0 },
        { "Mobile", DATA_BOOL, 1, 0 },
        { "Group", DATA_ULONG, 1, 0 },
        { "Flags", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// sFlags
        { "", DATA_ULONG, 1, 0 },
        { "IP", DATA_IP, 1, 0 },
        { "RealIP", DATA_IP, 1, 0 },
        { "Port", DATA_ULONG, 1, 0 },
        { "", DATA_OBJECT, 1, 0 },				// sb
        { NULL, 0, 0, 0 }
    };

/*
typedef struct MSNClientData
{
    char		*Server;
    unsigned	Port;
	MSNUserData	owner;
} MSNClientData;
*/
static DataDef msnClientData[] =
    {
        { "Server", DATA_STRING, 1, "messenger.hotmail.com" },
        { "Port", DATA_ULONG, 1, DATA(1863) },
        { "ListVer", DATA_ULONG, 1, 0 },
        { "ListRequests", DATA_UTF, 1, 0 },
        { "Version", DATA_STRING, 1, "5.0.0540" },
        { "MinPort", DATA_ULONG, 1, DATA(1024) },
        { "MaxPort", DATA_ULONG, 1, DATA(0xFFFF) },
        { "UseHTTP", DATA_ULONG, 1, 0 },
        { "AutoHTTP", DATA_ULONG, 1, DATA(1) },
        { "Deleted", DATA_STRLIST, 1, 0 },
        { "NDeleted", DATA_ULONG, 1, 0 },
        { "AutoAuth", DATA_BOOL, 1, DATA(1) },
        { "", DATA_STRUCT, sizeof(MSNUserData) / sizeof(Data), DATA(msnUserData) },
        { NULL, 0, 0, 0 }
    };

MSNClient::MSNClient(Protocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(msnClientData, &data, cfg);
    m_packetId  = 1;
    m_msg       = NULL;
    m_bFirst    = (cfg == NULL);
    QString s = getListRequests();
    while (!s.isEmpty()){
        QString item = getToken(s, ';');
        MSNListRequest lr;
        lr.Type = getToken(item, ',').toUInt();
        lr.Name = item.utf8();
    }
    setListRequests("");
    m_bJoin = false;
    m_bFirstTry = false;
}

MSNClient::~MSNClient()
{
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(msnClientData, &data);
    freeData();
}

string MSNClient::name()
{
    string res = "MSN.";
    QString s = QString::fromLocal8Bit(getLogin());
    res += s.utf8();
    return res;
}

QWidget	*MSNClient::setupWnd()
{
    return new MSNConfig(NULL, this, false);
}

string MSNClient::getConfig()
{
    QString listRequests;
    for (list<MSNListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        if (!listRequests.isEmpty())
            listRequests += ";";
        listRequests += QString::number((*it).Type) + "," + QString::fromUtf8((*it).Name.c_str());
    }
    setListRequests(listRequests);
    string res = Client::getConfig();
    if (res.length())
        res += "\n";
    res += save_data(msnClientData, &data);
    setListRequests("");
    return res;
}

const DataDef *MSNProtocol::userDataDef()
{
    return msnUserData;
}

void MSNClient::connect_ready()
{
    m_bFirstTry = false;
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    m_socket->setRaw(true);
    log(L_DEBUG, "Connect ready");
    TCPClient::connect_ready();
    MSNPacket *packet = new VerPacket(this);
    packet->send();
}

void MSNClient::setStatus(unsigned status)
{
    if (status  == m_status)
        return;
    time_t now;
    time(&now);
    if (m_status == STATUS_OFFLINE)
        data.owner.OnlineTime.value = now;
    data.owner.StatusTime.value = now;
    m_status = status;
    data.owner.Status.value = m_status;
    Event e(EventClientChanged, static_cast<Client*>(this));
    e.process();
    if (status == STATUS_OFFLINE){
        if (m_status != STATUS_OFFLINE){
            m_status = status;
            data.owner.Status.value = status;
            time_t now;
            time(&now);
            data.owner.StatusTime.value = now;
            MSNPacket *packet = new OutPacket(this);
            packet->send();
        }
        return;
    }
    if (Client::m_state != Connected){
        m_logonStatus = status;
        return;
    }
    m_status = status;
    MSNPacket *packet = new ChgPacket(this);
    packet->send();
}

void MSNClient::connected()
{
    setState(Client::Connected);
    setStatus(m_logonStatus);
    processRequests();
}

void MSNClient::setInvisible(bool bState)
{
    if (bState == getInvisible())
        return;
    TCPClient::setInvisible(bState);
    if (getStatus() == STATUS_OFFLINE)
        return;
    MSNPacket *packet = new ChgPacket(this);
    packet->send();
}

void MSNClient::disconnected()
{
    stop();
    Contact *contact;
    ContactList::ContactIterator it;
    time_t now;
    time(&now);
    while ((contact = ++it) != NULL){
        bool bChanged = false;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            if (data->Status.value != STATUS_OFFLINE){
                data->Status.value = STATUS_OFFLINE;
                data->StatusTime.value = now;
                if (data->sb.ptr){
                    delete (SBSocket*)(data->sb.ptr);
                    data->sb.ptr = NULL;
                }
                bChanged = true;
            }
            if (bChanged){
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(dataName(data).c_str());
                m.setFlags(MESSAGE_RECEIVED);
                m.setStatus(STATUS_OFFLINE);
                Event e(EventMessageReceived, &m);
                e.process();
            }
        }
    }
    m_packetId = 0;
    m_pingTime = 0;
    m_state    = None;
    m_authChallenge = "";
    clearPackets();
}

void MSNClient::clearPackets()
{
    if (m_msg){
        delete m_msg;
        m_msg = NULL;
    }
    for (list<MSNPacket*>::iterator it = m_packets.begin(); it != m_packets.end(); ++it){
        delete *it;
    }
    m_packets.clear();
}

void MSNClient::packet_ready()
{
    if (m_socket->readBuffer.writePos() == 0)
        return;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->MSNPacket);
    if (m_msg){
        if (!m_msg->packet())
            return;
        delete m_msg;
        m_msg = NULL;
    }
    for (;;){
        string s;
        if (!m_socket->readBuffer.scan("\r\n", s))
            break;
        getLine(s.c_str());
    }
    if (m_socket->readBuffer.readPos() == m_socket->readBuffer.writePos())
        m_socket->readBuffer.init(0);
}

typedef struct statusText
{
    unsigned	status;
    const char	*name;
} statusText;

statusText st[] =
    {
        { STATUS_ONLINE, "NLN" },
        { STATUS_OFFLINE, "FLN" },
        { STATUS_OFFLINE, "HDN" },
        { STATUS_NA, "IDL" },
        { STATUS_AWAY, "AWY" },
        { STATUS_DND, "BSY" },
        { STATUS_BRB, "BRB" },
        { STATUS_PHONE, "PHN" },
        { STATUS_LUNCH, "LUN" },
        { 0, NULL }
    };

static unsigned str2status(const char *str)
{
    for (const statusText *s = st; s->name; s++){
        if (!strcmp(str, s->name))
            return s->status;
    }
    return STATUS_OFFLINE;
}

void MSNClient::processLSG(unsigned id, const char *name)
{
    if (id == 0)
        return;
    Group *grp;
    MSNListRequest *lr = findRequest(id, LR_GROUPxREMOVED);
    if (lr)
        return;
    MSNUserData *data = findGroup(id, NULL, grp);
    if (data){
        lr = findRequest(grp->id(), LR_GROUPxCHANGED);
        if (lr){
            data->sFlags.value |= MSN_CHECKED;
            return;
        }
    }
    data = findGroup(id, name, grp);
    data->sFlags.value |= MSN_CHECKED;
}

void MSNClient::processLST(const char *mail, const char *name, unsigned state, unsigned grp)
{
    if ((state & MSN_FORWARD) == 0){
        for (unsigned i = 1; i <= getNDeleted(); i++){
            if (!strcmp(getDeleted(i), mail))
                return;
        }
    }

    m_curBuddy = mail;
    Contact *contact;
    MSNListRequest *lr = findRequest(mail, LR_CONTACTxREMOVED);
    if (lr)
        return;
    bool bNew = false;
    MSNUserData *data = findContact(mail, contact);
    if (data == NULL){
        data = findContact(mail, name, contact);
        bNew = true;
    }else{
        set_str(&data->EMail.ptr, mail);
        set_str(&data->ScreenName.ptr, name);
        if (name != (const char*)(contact->getName().utf8()))
            contact->setName(QString::fromUtf8(name));
    }
    data->sFlags.value |= MSN_CHECKED;
    data->Flags.value = state;
    if (state & MSN_BLOCKED)
        contact->setIgnore(true);

    lr = findRequest(mail, LR_CONTACTxCHANGED);
    data->Group.value = grp;
    set_str(&data->PhoneHome.ptr, NULL);
    set_str(&data->PhoneWork.ptr, NULL);
    set_str(&data->PhoneMobile.ptr, NULL);
    data->Mobile.bValue = false;
    Group *group = NULL;
    if ((grp == 0) || (grp == NO_GROUP)){
        group = getContacts()->group(0);
    }else{
        findGroup(grp, NULL, group);
    }
    if (lr == NULL){
        bool bChanged = ((data->Flags.value & MSN_FLAGS) != (data->sFlags.value & MSN_FLAGS));
        if (getAutoAuth() && (data->Flags.value & MSN_FORWARD) && (data->Flags.value & MSN_ACCEPT == 0) && (data->Flags.value & MSN_BLOCKED == 0))
            bChanged = true;
        unsigned grp = 0;
        if (group)
            grp = group->id();
        if (grp != contact->getGroup())
            bChanged = true;
        if (bChanged){
            MSNListRequest lr;
            lr.Type = LR_CONTACTxCHANGED;
            lr.Name = data->EMail.ptr;
            m_requests.push_back(lr);
        }
        if (data->Flags.value & MSN_FORWARD)
            contact->setGroup(grp);
    }
}

void MSNClient::checkEndSync()
{
    if (m_nBuddies || m_nGroups)
        return;
    ContactList::GroupIterator itg;
    Group *grp;
    list<Group*>	grpRemove;
    list<Contact*>	contactRemove;
    while ((grp = ++itg) != NULL){
        ClientDataIterator it(grp->clientData, this);
        MSNUserData *data = (MSNUserData*)(++it);
        if (grp->id() && (data == NULL)){
            MSNListRequest lr;
            lr.Type = LR_GROUPxCHANGED;
            lr.Name = number(grp->id());
            m_requests.push_back(lr);
            continue;
        }
        if (data == NULL)
            continue;
        if ((data->sFlags.value & MSN_CHECKED) == 0)
            grpRemove.push_back(grp);
    }
    Contact *contact;
    ContactList::ContactIterator itc;
    while ((contact = ++itc) != NULL){
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        list<void*> forRemove;
        while ((data = (MSNUserData*)(++it)) != NULL){
            if (data->sFlags.value & MSN_CHECKED){
                if ((data->sFlags.value & MSN_REVERSE) && ((data->Flags.value & MSN_REVERSE) == 0))
                    auth_message(contact, MessageRemoved, data);
                if (!m_bFirst && ((data->sFlags.value & MSN_REVERSE) == 0) && (data->Flags.value & MSN_REVERSE)){
                    if ((data->Flags.value & MSN_ACCEPT) || getAutoAuth()){
                        auth_message(contact, MessageAdded, data);
                    }else{
                        auth_message(contact, MessageAuthRequest, data);
                    }
                }
                setupContact(contact, data);
                Event e(EventContactChanged, contact);
                e.process();
            }else{
                forRemove.push_back(data);
            }
        }
        if (forRemove.empty())
            continue;
        for (list<void*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
            contact->clientData.freeData(*itr);
        if (contact->clientData.size() == 0)
            contactRemove.push_back(contact);
    }
    for (list<Contact*>::iterator rc = contactRemove.begin(); rc != contactRemove.end(); ++rc)
        delete *rc;
    for (list<Group*>::iterator rg = grpRemove.begin(); rg != grpRemove.end(); ++rg)
        delete *rg;
    if (m_bJoin){
        Event e(EventJoinAlert, this);
        e.process();
    }
    m_bFirst = false;
    connected();
}

static unsigned toInt(const QString &str)
{
    if (str.isEmpty())
        return 0;
    return atol(str.latin1());
}

void MSNClient::getLine(const char *line)
{
    QString l = QString::fromUtf8(line);
    l = l.replace(QRegExp("\r"), "");
    QCString ll = l.local8Bit();
    log(L_DEBUG, "Get: %s", (const char*)ll);
    QString cmd = getToken(l, ' ');
    if ((cmd == "715") || (cmd == "228"))
        return;
    if (cmd == "XFR"){
        QString id   = getToken(l, ' ');	// ID
        QString type = getToken(l, ' ');	// NS
        if (type == "NS"){
            l = getToken(l, ' ');				// from
            QString host = getToken(l, ':');
            unsigned short port = l.toUShort();
            if (host.isEmpty() || (port == 0)){
                log(L_WARN, "Bad host on XFR");
                m_socket->error_state(I18N_NOOP("MSN protocol error"));
                return;
            }
            clearPackets();
            m_socket->close();
            m_socket->readBuffer.init(0);
            m_socket->connect(host.latin1(), port, this);
            return;
        }
        l = id + " " + type + " " + l;
    }
    if (cmd == "MSG"){
        getToken(l, ' ');
        getToken(l, ' ');
        unsigned size = getToken(l, ' ').toUInt();
        if (size == 0){
            log(L_WARN, "Bad server message size");
            m_socket->error_state(I18N_NOOP("MSN protocol error"));
            return;
        }
        m_msg = new MSNServerMessage(this, size);
        packet_ready();
        return;
    }
    if (cmd == "CHL"){
        getToken(l, ' ');
        MSNPacket *packet = new QryPacket(this, getToken(l, ' ').latin1());
        packet->send();
        return;
    }
    if (cmd == "QNG")
        return;
    if (cmd == "BPR"){
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
        if (data){
            QString info = getToken(l, ' ');
            QString type = getToken(l, ' ');
            bool bChanged = false;
            if (type == "PHH"){
                bChanged = set_str(&data->PhoneHome.ptr, unquote(info).utf8());
            }else if (type == "PHW"){
                bChanged = set_str(&data->PhoneWork.ptr, unquote(info).utf8());
            }else if (type == "PHM"){
                bChanged = set_str(&data->PhoneMobile.ptr, unquote(info).utf8());
            }else if (type == "MOB"){
                data->Mobile.bValue = ((info[0] == 'Y') != 0);
            }else{
                log(L_DEBUG, "Unknown BPR type %s", type.latin1());
            }
            if (bChanged){
                setupContact(contact, data);
                Event e(EventContactChanged, contact);
                e.process();
            }
        }
        return;
    }
    if (cmd == "ILN"){
        getToken(l, ' ');
        unsigned status = str2status(getToken(l, ' '));
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
        if (data && (data->Status.value != status)){
            time_t now;
            time(&now);
            data->Status.value = status;
            if (data->Status.value == STATUS_OFFLINE){
                data->OnlineTime.value = now;
                set_ip(&data->IP, 0);
                set_ip(&data->RealIP, 0);
            }
            data->StatusTime.value = now;
            StatusMessage m;
            m.setContact(contact->id());
            m.setClient(dataName(data).c_str());
            m.setFlags(MESSAGE_RECEIVED);
            m.setStatus(status);
            Event e(EventMessageReceived, &m);
            e.process();
        }
        return;
    }
    if (cmd == "NLN"){
        unsigned status = str2status(getToken(l, ' '));
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
        if (data && (data->Status.value != status)){
            time_t now;
            time(&now);
            if (data->Status.value == STATUS_OFFLINE){
                data->OnlineTime.value = now;
                set_ip(&data->IP, 0);
                set_ip(&data->RealIP, 0);
            }
            data->StatusTime.value = now;
            data->Status.value = status;
            StatusMessage m;
            m.setContact(contact->id());
            m.setClient(dataName(data).c_str());
            m.setFlags(MESSAGE_RECEIVED);
            m.setStatus(status);
            Event e(EventMessageReceived, &m);
            e.process();
            if ((status == STATUS_ONLINE) && !contact->getIgnore()){
                Event e(EventContactOnline, contact);
                e.process();
            }
        }
        return;
    }
    if (cmd == "FLN"){
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
        if (data && (data->Status.value != STATUS_OFFLINE)){
            time_t now;
            time(&now);
            data->StatusTime.value = now;
            data->Status.value = STATUS_OFFLINE;
            StatusMessage m;
            m.setContact(contact->id());
            m.setClient(dataName(data).c_str());
            m.setFlags(MESSAGE_RECEIVED);
            m.setStatus(STATUS_OFFLINE);
            Event e(EventMessageReceived, &m);
            e.process();
        }
        return;
    }
    if (cmd == "ADD"){
        getToken(l, ' ');
        if (getToken(l, ' ') == "RL"){
            setListVer(getToken(l, ' ').toUInt());
            Contact *contact;
            MSNUserData *data = findContact(getToken(l, ' ').utf8(), getToken(l, ' ').utf8(), contact);
            if (data){
                data->Flags.value |= MSN_REVERSE;
                if ((data->Flags.value & MSN_ACCEPT) || getAutoAuth()){
                    auth_message(contact, MessageAdded, data);
                }else{
                    auth_message(contact, MessageAuthRequest, data);
                }
            }
        }
        return;
    }
    if (cmd == "REM"){
        getToken(l, ' ');
        if (getToken(l, ' ') == "RL"){
            setListVer(getToken(l, ' ').toUInt());
            Contact *contact;
            MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
            if (data){
                data->Flags.value &= ~MSN_REVERSE;
                auth_message(contact, MessageRemoved, data);
            }
        }
        return;
    }
    if (cmd == "RNG"){
        string session = getToken(l, ' ').latin1();
        string addr = getToken(l, ' ').latin1();
        getToken(l, ' ');
        string cookie, email, nick;
        cookie = getToken(l, ' ').utf8();
        email  = getToken(l, ' ').utf8();
        nick   = getToken(l, ' ').utf8();
        Contact *contact;
        MSNUserData *data = findContact(email.c_str(), contact);
        if (data == NULL){
            data = findContact(email.c_str(), nick.c_str(), contact);
            contact->setTemporary(CONTACT_TEMP);
            Event e(EventContactChanged, contact);
            e.process();
        }
        if (data->sb.ptr){
            delete (SBSocket*)(data->sb.ptr);
            data->sb.ptr = NULL;
        }
        data->sb.ptr = (char*)(new SBSocket(this, contact, data));
        ((SBSocket*)(data->sb.ptr))->connect(addr.c_str(), session.c_str(), cookie.c_str(), false);
        return;
    }
    if (cmd == "OUT"){
        m_reconnect = NO_RECONNECT;
        m_socket->error_state(I18N_NOOP("Youur account is being used from another location"));
        return;
    }
    if (cmd == "GTC")
        return;
    if (cmd == "BLP")
        return;
    if (cmd == "LSG"){
        unsigned id = toInt(getToken(l, ' '));
        processLSG(id, unquote(getToken(l, ' ')).utf8());
        m_nGroups--;
        checkEndSync();
        return;
    }
    if (cmd == "LST"){
        string mail;
        mail = unquote(getToken(l, ' ')).utf8();
        string name;
        name = unquote(getToken(l, ' ')).utf8();
        unsigned state = toInt(getToken(l, ' '));
        unsigned grp   = toInt(getToken(l, ' '));
        processLST(mail.c_str(), name.c_str(), state, grp);
        m_nBuddies--;
        checkEndSync();
        return;
    }
    if (cmd == "PRP"){
        QString cmd = getToken(l, ' ');
        if (cmd == "PHH")
            set_str(&data.owner.PhoneHome.ptr, unquote(getToken(l, ' ')).utf8());
        if (cmd == "PHW")
            set_str(&data.owner.PhoneWork.ptr, unquote(getToken(l, ' ')).utf8());
        if (cmd == "PHM")
            set_str(&data.owner.PhoneMobile.ptr, unquote(getToken(l, ' ')).utf8());
        if (cmd == "MBE")
            data.owner.Mobile.bValue = (getToken(l, ' ') == "Y");
        return;
    }
    if (cmd == "BPR"){
        Contact *contact;
        MSNUserData *data = findContact(m_curBuddy.c_str(), contact);
        if (data == NULL)
            return;
        Event e(EventContactChanged, contact);
        e.process();
        QString cmd = getToken(l, ' ');
        if (cmd == "PHH")
            set_str(&data->PhoneHome.ptr, unquote(getToken(l, ' ')).utf8());
        if (cmd == "PHW")
            set_str(&data->PhoneWork.ptr, unquote(getToken(l, ' ')).utf8());
        if (cmd == "PHM")
            set_str(&data->PhoneMobile.ptr, unquote(getToken(l, ' ')).utf8());
        if (cmd == "MBE")
            data->Mobile.bValue = (getToken(l, ' ') == "Y");
        return;
    }
    unsigned code = cmd.toUInt();
    if (code){
        MSNPacket *packet = NULL;
        unsigned id = getToken(l, ' ').toUInt();
        list<MSNPacket*>::iterator it;
        for (it = m_packets.begin(); it != m_packets.end(); ++it){
            if ((*it)->id() == id){
                packet = *it;
                break;
            }
        }
        if (it == m_packets.end()){
            m_socket->error_state("Bad packet id");
            return;
        }
        m_packets.erase(it);
        packet->error(code);
        delete packet;
        return;
    }
    if (m_packets.empty()){
        log(L_DEBUG, "Packet not found");
        return;
    }
    MSNPacket *packet = NULL;
    unsigned id = getToken(l, ' ').toUInt();
    list<MSNPacket*>::iterator it;
    for (it = m_packets.begin(); it != m_packets.end(); ++it){
        if ((*it)->id() == id){
            packet = *it;
            break;
        }
    }
    if (it == m_packets.end()){
        m_socket->error_state("Bad packet id");
        return;
    }
    if (cmd != packet->cmd()){
        m_socket->error_state("Bad answer cmd");
        return;
    }
    vector<string> args;
    while (l.length())
        args.push_back(string(getToken(l, ' ', false).utf8()));
    packet->answer(args);
    m_packets.erase(it);
    delete packet;
}

void MSNClient::sendLine(const char *line, bool crlf)
{
    log(L_DEBUG, "Send: %s", line);
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer << line;
    if (crlf)
        m_socket->writeBuffer << "\r\n";
    MSNPlugin *plugin = static_cast<MSNPlugin*>(protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->MSNPacket);
    m_socket->write();
}

void MSNClient::authFailed()
{
    m_reconnect = NO_RECONNECT;
    m_socket->error_state(I18N_NOOP("Login failed"), AuthError);
}

void MSNClient::authOk()
{
    m_state    = None;
    m_authChallenge = "";
    time_t now;
    time(&now);
    m_pingTime = now;
    QTimer::singleShot(TYPING_TIME * 1000, this, SLOT(ping()));
    setPreviousPassword(NULL);
    MSNPacket *packet = new SynPacket(this);
    packet->send();
}

void MSNClient::ping()
{
    if (getState() != Connected)
        return;
    time_t now;
    time(&now);
    if ((unsigned)now >= m_pingTime + PING_TIMEOUT){
        sendLine("PNG");
        m_pingTime = now;
    }
    for (list<SBSocket*>::iterator it = m_SBsockets.begin(); it != m_SBsockets.end(); ++it)
        (*it)->timer(now);
    QTimer::singleShot(TYPING_TIME * 1000, this, SLOT(ping()));
}

QString MSNClient::getLogin()
{
    if (data.owner.EMail.ptr == NULL)
        return "";
    return QString::fromUtf8(data.owner.EMail.ptr);
}

void MSNClient::setLogin(const QString &str)
{
    set_str(&data.owner.EMail.ptr, str.utf8());
}

const unsigned MAIN_INFO = 1;
const unsigned NETWORK	 = 2;

static CommandDef msnWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "MSN_online",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
    };

static CommandDef cfgMsnWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "MSN_online",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            NETWORK,
            I18N_NOOP("Network"),
            "network",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
    };

CommandDef *MSNClient::infoWindows(Contact*, void *_data)
{
    MSNUserData *data = (MSNUserData*)_data;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data->EMail.ptr);
    msnWnd[0].text_wrk = strdup(name.utf8());
    return msnWnd;
}

CommandDef *MSNClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data.owner.EMail.ptr);
    cfgMsnWnd[0].text_wrk = strdup(name.utf8());
    return cfgMsnWnd;
}

QWidget *MSNClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    MSNUserData *data = (MSNUserData*)_data;
    switch (id){
    case MAIN_INFO:
        return new MSNInfo(parent, data, this);
    }
    return NULL;
}

QWidget *MSNClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new MSNInfo(parent, NULL, this);
    case NETWORK:
        return new MSNConfig(parent, this, true);
    }
    return NULL;
}

bool MSNClient::canSend(unsigned type, void *_data)
{
    if ((_data == NULL) || (((clientData*)_data)->Sign.value != MSN_SIGN))
        return false;
    MSNUserData *data = (MSNUserData*)_data;
    if (getState() != Connected)
        return false;
    switch (type){
    case MessageGeneric:
    case MessageFile:
    case MessageUrl:
        if (getInvisible())
            return false;
        return true;
    case MessageAuthGranted:
    case MessageAuthRefused:
        return (data->Flags.value & MSN_ACCEPT) == 0;
    }
    return false;
}

bool MSNClient::send(Message *msg, void *_data)
{
    if ((_data == NULL) || (getState() != Connected))
        return false;
    MSNUserData *data = (MSNUserData*)_data;
    MSNPacket *packet;
    switch (msg->type()){
    case MessageAuthGranted:
        if (data->Flags.value & MSN_ACCEPT)
            return false;
        packet = new AddPacket(this, "AL", data->EMail.ptr, quote(QString::fromUtf8(data->ScreenName.ptr)).utf8(), 0);
        packet->send();
    case MessageAuthRefused:
        if (data->Flags.value & MSN_ACCEPT)
            return false;
        if (msg->getText().isEmpty()){
            if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                msg->setClient(dataName(data).c_str());
                Event e(EventSent, msg);
                e.process();
            }
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            return true;
        }
    case MessageGeneric:
    case MessageFile:
    case MessageUrl:
        if (data->sb.ptr == NULL){
            if (getInvisible())
                return false;
            Contact *contact;
            findContact(data->EMail.ptr, contact);
            data->sb.ptr = (char*)(new SBSocket(this, contact, data));
            ((SBSocket*)(data->sb.ptr))->connect();
        }
        return ((SBSocket*)(data->sb.ptr))->send(msg);
    case MessageTypingStart:
        if (data->sb.ptr == NULL){
            if (getInvisible())
                return false;
            Contact *contact;
            findContact(data->EMail.ptr, contact);
            data->sb.ptr = (char*)(new SBSocket(this, contact, data));
            ((SBSocket*)(data->sb.ptr))->connect();
        }
        ((SBSocket*)(data->sb.ptr))->setTyping(true);
        delete msg;
        return true;
    case MessageTypingStop:
        if (data->sb.ptr == NULL)
            return false;
        ((SBSocket*)(data->sb.ptr))->setTyping(false);
        delete msg;
        return true;
    }
    return false;
}

string MSNClient::dataName(void *_data)
{
    string res = name();
    MSNUserData *data = (MSNUserData*)_data;
    res += "+";
    res += data->EMail.ptr;
    return res;
}

bool MSNClient::isMyData(clientData *&_data, Contact *&contact)
{
    if (_data->Sign.value != MSN_SIGN)
        return false;
    MSNUserData *data = (MSNUserData*)_data;
    if (data->EMail.ptr && this->data.owner.EMail.ptr &&
            QString(data->EMail.ptr).lower() == QString(this->data.owner.EMail.ptr).lower())
        return false;
    MSNUserData *my_data = findContact(data->EMail.ptr, contact);
    if (my_data){
        data = my_data;
    }else{
        contact = NULL;
    }
    return true;
}

bool MSNClient::createData(clientData *&_data, Contact *contact)
{
    MSNUserData *data = (MSNUserData*)_data;
    MSNUserData *new_data = (MSNUserData*)(contact->clientData.createData(this));
    set_str(&new_data->EMail.ptr, data->EMail.ptr);
    _data = (clientData*)new_data;
    return true;
}

void MSNClient::setupContact(Contact *contact, void *_data)
{
    MSNUserData *data = (MSNUserData*)_data;
    QString phones;
    if (data->PhoneHome.ptr){
        phones += QString::fromUtf8(data->PhoneHome.ptr);
        phones += ",Home Phone,1";
    }
    if (data->PhoneWork.ptr){
        if (!phones.isEmpty())
            phones += ";";
        phones += QString::fromUtf8(data->PhoneWork.ptr);
        phones += ",Work Phone,1";
    }
    if (data->PhoneMobile.ptr){
        if (!phones.isEmpty())
            phones += ";";
        phones += QString::fromUtf8(data->PhoneMobile.ptr);
        phones += ",Private Cellular,2";
    }
    bool bChanged = contact->setPhones(phones, name().c_str());
    bChanged |= contact->setEMails(data->EMail.ptr, name().c_str());
    if (contact->getName().isEmpty()){
        QString name = QString::fromUtf8(data->ScreenName.ptr);
        if (name.isEmpty())
            name = QString::fromUtf8(data->EMail.ptr);
        int n = name.find('@');
        if (n > 0)
            name = name.left(n);
        bChanged |= contact->setName(name);
    }
    if (bChanged){
        Event e(EventContactChanged, contact);
        e.process();
    }
}

QString MSNClient::unquote(const QString &s)
{
    QString res;
    for (int i = 0; i < (int)(s.length()); i++){
        QChar c = s[i];
        if (c != '%'){
            res += c;
            continue;
        }
        i++;
        if (i + 2 > (int)(s.length()))
            break;
        res += QChar((char)((fromHex(s[i++]) << 4) + fromHex(s[i])));
    }
    return res;
}

QString MSNClient::quote(const QString &s)
{
    QString res;
    for (int i = 0; i < (int)(s.length()); i++){
        QChar c = s[i];
        if ((c == '%') || (c == ' ')){
            char b[4];
            sprintf(b, "%%%2X", (unsigned)c);
            res += b;
        }else{
            res += QChar(c);
        }
    }
    return res;
}

MSNUserData *MSNClient::findContact(const char *mail, Contact *&contact)
{
    ContactList::ContactIterator itc;
    while ((contact = ++itc) != NULL){
        MSNUserData *res;
        ClientDataIterator it(contact->clientData, this);
        while ((res = (MSNUserData*)(++it)) != NULL){
            if (!strcmp(res->EMail.ptr, mail))
                return res;
        }
    }
    return NULL;
}

QString MSNClient::contactName(void *clientData)
{
    MSNUserData *data = (MSNUserData*)clientData;
    QString res = "MSN: ";
    res += QString::fromUtf8(data->EMail.ptr);
    return res;
}

MSNUserData *MSNClient::findContact(const char *mail, const char *name, Contact *&contact)
{
    unsigned i;
    for (i = 1; i <= getNDeleted(); i++){
        if (!strcmp(getDeleted(i), mail))
            break;
    }
    if (i <= getNDeleted()){
        list<string> deleted;
        for (i = 1; i <= getNDeleted(); i++){
            if (!strcmp(getDeleted(i), mail))
                continue;
            deleted.push_back(getDeleted(i));
        }
        setNDeleted(0);
        for (list<string>::iterator it = deleted.begin(); it != deleted.end(); ++it){
            setNDeleted(getNDeleted() + 1);
            setDeleted(getNDeleted(), (*it).c_str());
        }
    }
    QString name_str = unquote(QString::fromUtf8(name));
    MSNUserData *data = findContact(mail, contact);
    if (data){
        set_str(&data->ScreenName.ptr, name);
        setupContact(contact, data);
        return data;
    }
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        if (contact->getName() == name_str){
            data = (MSNUserData*)(contact->clientData.createData(this));
            set_str(&data->EMail.ptr, mail);
            set_str(&data->ScreenName.ptr, name);
            setupContact(contact, data);
            Event e(EventContactChanged, contact);
            e.process();
            return data;
        }
    }
    it.reset();
    while ((contact = ++it) != NULL){
        if (contact->getName().lower() == name_str.lower()){
            data = (MSNUserData*)(contact->clientData.createData(this));
            set_str(&data->EMail.ptr, mail);
            set_str(&data->ScreenName.ptr, name);
            setupContact(contact, data);
            Event e(EventContactChanged, contact);
            e.process();
            m_bJoin = true;
            return data;
        }
    }
    int n = name_str.find('@');
    if (n > 0){
        name_str = name_str.left(n);
        it.reset();
        while ((contact = ++it) != NULL){
            if (contact->getName().lower() == name_str.lower()){
                data = (MSNUserData*)(contact->clientData.createData(this));
                set_str(&data->EMail.ptr, mail);
                set_str(&data->ScreenName.ptr, name);
                setupContact(contact, data);
                Event e(EventContactChanged, contact);
                e.process();
                m_bJoin = true;
                return data;
            }
        }
    }
    contact = getContacts()->contact(0, true);
    data = (MSNUserData*)(contact->clientData.createData(this));
    set_str(&data->EMail.ptr, mail);
    set_str(&data->ScreenName.ptr, name);
    contact->setName(name_str);
    Event e(EventContactChanged, contact);
    e.process();
    return data;
}

MSNUserData *MSNClient::findGroup(unsigned long id, const char *name, Group *&grp)
{
    ContactList::GroupIterator itg;
    while ((grp = ++itg) != NULL){
        ClientDataIterator it(grp->clientData, this);
        MSNUserData *res = (MSNUserData*)(++it);
        if ((res == NULL) || (res->Group.value != id))
            continue;
        if ((name != NULL) && set_str(&res->ScreenName.ptr, name)){
            grp->setName(QString::fromUtf8(name));
            Event e(EventGroupChanged, grp);
            e.process();
        }
        return res;
    }
    if (name == NULL)
        return NULL;
    QString grpName = QString::fromUtf8(name);
    itg.reset();
    while ((grp = ++itg) != NULL){
        if (grp->getName() != grpName)
            continue;
        MSNUserData *res = (MSNUserData*)(grp->clientData.createData(this));
        res->Group.value = id;
        set_str(&res->ScreenName.ptr, name);
        return res;
    }
    grp = getContacts()->group(0, true);
    MSNUserData *res = (MSNUserData*)(grp->clientData.createData(this));
    res->Group.value = id;
    set_str(&res->ScreenName.ptr, name);
    grp->setName(grpName);
    Event e(EventGroupChanged, grp);
    e.process();
    return res;
}

void MSNClient::auth_message(Contact *contact, unsigned type, MSNUserData *data)
{
    AuthMessage msg(type);
    msg.setClient(dataName(data).c_str());
    msg.setContact(contact->id());
    msg.setFlags(MESSAGE_RECEIVED);
    Event e(EventMessageReceived, &msg);
    e.process();
}

bool MSNClient::done(unsigned code, Buffer&, const char *headers)
{
    string h;
    switch (m_state){
    case LoginHost:
        if (code == 200){
            h = getHeader("PassportURLs", headers);
            if (h.empty()){
                m_socket->error_state("No PassportURLs answer");
                break;
            }
            string loginHost = getValue("DALogin", h.c_str());
            if (loginHost.empty()){
                m_socket->error_state("No DALogin in PassportURLs answer");
                break;
            }
            string loginUrl = "https://";
            loginUrl += loginHost;
            requestTWN(loginUrl.c_str());
        }else{
            m_socket->error_state("Bad answer code");
        }
        break;
    case TWN:
        if (code == 200){
            h = getHeader("Authentication-Info", headers);
            if (h.empty()){
                m_socket->error_state("No Authentication-Info answer");
                break;
            }
            string twn = getValue("from-PP", h.c_str());
            if (twn.empty()){
                m_socket->error_state("No from-PP in Authentication-Info answer");
                break;
            }
            MSNPacket *packet = new UsrPacket(this, twn.c_str());
            packet->send();
        }else if (code == 401){
            authFailed();
        }else{
            m_socket->error_state("Bad answer code");
        }
        break;
    default:
        log(L_WARN, "Fetch done in bad state");
    }
    return false;
}

void *MSNClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    if (e->type() == EventAddContact){
        addContact *ac = (addContact*)(e->param());
        if (ac->proto && !strcmp(protocol()->description()->text, ac->proto)){
            Contact *contact = NULL;
            findContact(ac->addr, ac->nick, contact);
            if (contact && (contact->getGroup() != ac->group)){
                contact->setGroup(ac->group);
                Event e(EventContactChanged, contact);
                e.process();
            }
            return contact;
        }
        return NULL;
    }
    if (e->type() == EventDeleteContact){
        char *addr = (char*)(e->param());
        ContactList::ContactIterator it;
        Contact *contact;
        while ((contact = ++it) != NULL){
            MSNUserData *data;
            ClientDataIterator itc(contact->clientData, this);
            while ((data = (MSNUserData*)(++itc)) != NULL){
                if (!strcmp(data->EMail.ptr, addr)){
                    contact->clientData.freeData(data);
                    ClientDataIterator itc(contact->clientData);
                    if (++itc == NULL)
                        delete contact;
                    return e->param();
                }
            }
        }
        return NULL;
    }
    if (e->type() == EventGetContactIP){
        Contact *contact = (Contact*)(e->param());
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            if (data->IP.ptr)
                return data->IP.ptr;
        }
        return NULL;
    }
    if (e->type() == EventMessageAccept){
        messageAccept *ma = (messageAccept*)(e->param());
        Contact *contact = getContacts()->contact(ma->msg->contact());
        if (contact == NULL)
            return NULL;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            if (dataName(data) == ma->msg->client()){
                if (data->sb.ptr)
                    ((SBSocket*)(data->sb.ptr))->acceptMessage(ma->msg, ma->dir, ma->overwrite);
                return e->param();
            }
        }
    }
    if (e->type() == EventMessageDecline){
        messageDecline *md = (messageDecline*)(e->param());
        Contact *contact = getContacts()->contact(md->msg->contact());
        if (contact == NULL)
            return NULL;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            if (dataName(data) == md->msg->client()){
                if (data->sb.ptr)
                    ((SBSocket*)(data->sb.ptr))->declineMessage(md->msg, md->reason);
                return e->param();
            }
        }
    }
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            bool bChanged = false;
            if (contact->getIgnore() != ((data->Flags.value & MSN_BLOCKED) != 0))
                bChanged = true;
            if (contact->getGroup() != (data->Group.value))
                bChanged = true;
            if (contact->getName() != QString::fromUtf8(data->ScreenName.ptr))
                bChanged = true;
            if (!bChanged)
                continue;
            findRequest(data->EMail.ptr, LR_CONTACTxCHANGED, true);
            MSNListRequest lr;
            lr.Type = LR_CONTACTxCHANGED;
            lr.Name = data->EMail.ptr;
            m_requests.push_back(lr);
        }
        processRequests();
        return NULL;
    }
    if (e->type() == EventContactDeleted){
        Contact *contact = (Contact*)(e->param());
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            findRequest(data->EMail.ptr, LR_CONTACTxCHANGED, true);
            MSNListRequest lr;
            if (data->Group.value != NO_GROUP){
                lr.Type  = LR_CONTACTxREMOVED;
                lr.Name  = data->EMail.ptr;
                lr.Group = data->Group.value;
                m_requests.push_back(lr);
            }
            if (data->Flags.value & MSN_BLOCKED){
                lr.Type = LR_CONTACTxREMOVED_BL;
                lr.Name  = data->EMail.ptr;
                m_requests.push_back(lr);
            }
        }
        processRequests();
        return NULL;
    }
    if (e->type() == EventGroupChanged){
        Group *grp = (Group*)(e->param());
        ClientDataIterator it(grp->clientData, this);
        MSNUserData *data = (MSNUserData*)(++it);
        if ((data == NULL) || (grp->getName() != QString::fromUtf8(data->ScreenName.ptr))){
            findRequest(grp->id(), LR_GROUPxCHANGED, true);
            MSNListRequest lr;
            lr.Type = LR_GROUPxCHANGED;
            lr.Name = number(grp->id());
            m_requests.push_back(lr);
            processRequests();
        }
        return NULL;
    }
    if (e->type() == EventGroupDeleted){
        Group *grp = (Group*)(e->param());
        ClientDataIterator it(grp->clientData, this);
        MSNUserData *data = (MSNUserData*)(++it);
        if (data){
            findRequest(grp->id(), LR_GROUPxCHANGED, true);
            MSNListRequest lr;
            lr.Type = LR_GROUPxREMOVED;
            lr.Name = number(data->Group.value);
            m_requests.push_back(lr);
            processRequests();
        }
    }
    if (e->type() == EventMessageCancel){
        Message *msg = (Message*)(e->param());
        for (list<SBSocket*>::iterator it = m_SBsockets.begin(); it != m_SBsockets.end(); ++it){
            if ((*it)->cancelMessage(msg))
                return msg;
        }
    }
    return NULL;
}

void MSNClient::requestLoginHost(const char *url)
{
    if (!isDone())
        return;
    m_state = LoginHost;
    fetch(url);
}

void MSNClient::requestTWN(const char *url)
{
    if (!isDone())
        return;
    string auth = "Authorization: Passport1.4 OrgVerb=GET,OrgURL=http%%3A%%2F%%2Fmessenger%%2Emsn%%2Ecom,sign-in=";
    auth += quote(getLogin()).utf8();
    auth += ",pwd=";
    auth += quote(getPassword()).utf8();
    auth += ",";
    auth += m_authChallenge;
    m_state = TWN;
    fetch(url, auth.c_str());
}

string MSNClient::getValue(const char *key, const char *str)
{
    string s = str;
    while (!s.empty()){
        string k = getToken(s, '=');
        string v;
        if (s[0] == '\''){
            getToken(s, '\'');
            v = getToken(s, '\'');
            getToken(s, ',');
        }else{
            v = getToken(s, ',');
        }
        if (k == key)
            return v;
    }
    return "";
}

string MSNClient::getHeader(const char *name, const char *headers)
{
    for (const char *h = headers; *h; h += strlen(h) + 1){
        string header = h;
        string key = getToken(header, ':');
        if (key != name)
            continue;
        const char *p;
        for (p = header.c_str(); *p; p++)
            if (*p != ' ')
                break;
        return p;
    }
    return "";
}

MSNListRequest *MSNClient::findRequest(unsigned long id, unsigned type, bool bDelete)
{
    if (m_requests.empty())
        return NULL;
    string name = number(id);
    return findRequest(name.c_str(), type, bDelete);
}

MSNListRequest *MSNClient::findRequest(const char *name, unsigned type, bool bDelete)
{
    if (m_requests.empty())
        return NULL;
    for (list<MSNListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        if (((*it).Type == type) && ((*it).Name == name)){
            if (bDelete){
                m_requests.erase(it);
                return NULL;
            }
            return &(*it);
        }
    }
    return NULL;
}

void MSNClient::processRequests()
{
    if (m_requests.empty() || (getState() != Connected))
        return;
    for (list<MSNListRequest>::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        Group *grp;
        Contact *contact;
        MSNPacket *packet = NULL;
        MSNUserData *data;
        switch ((*it).Type){
        case LR_CONTACTxCHANGED:
            data = findContact((*it).Name.c_str(), contact);
            if (data){
                bool bBlock = (data->Flags.value & MSN_BLOCKED) != 0;
                if (contact->getIgnore() != bBlock){
                    if (contact->getIgnore()){
                        if (data->Flags.value & MSN_FORWARD)
                            packet = new RemPacket(this, "FL", (*it).Name.c_str());
                        if (data->Flags.value & MSN_ACCEPT){
                            if (packet)
                                packet->send();
                            packet = new RemPacket(this, "AL", (*it).Name.c_str());
                        }
                        data->Flags.value &= ~(MSN_FORWARD | MSN_ACCEPT);
                        if (packet)
                            packet->send();
                        packet = new AddPacket(this, "BL", data->EMail.ptr, quote(contact->getName()).utf8());
                        set_str(&data->ScreenName.ptr, contact->getName().utf8());
                        data->Flags.value |= MSN_BLOCKED;
                    }else{
                        packet = new RemPacket(this, "BL", data->EMail.ptr);
                        data->Flags.value &= ~MSN_BLOCKED;
                    }
                }
                if (data->Flags.value & MSN_BLOCKED)
                    break;
                unsigned grp_id = 0;
                if (contact->getGroup()){
                    Group *grp = getContacts()->group(contact->getGroup());
                    ClientDataIterator it(grp->clientData, this);
                    MSNUserData *res = (MSNUserData*)(++it);
                    if (res)
                        grp_id = res->Group.value;
                }
                if (((data->Flags.value & MSN_FORWARD) == 0) || (data->Group.value == NO_GROUP)){
                    if (packet)
                        packet->send();
                    packet = new AddPacket(this, "FL", data->EMail.ptr, quote(QString::fromUtf8(data->ScreenName.ptr)).utf8(), grp_id);
                    data->Group.value = grp_id;
                    data->Flags.value |= MSN_FORWARD;
                }
                if (getAutoAuth() && (data->Flags.value & MSN_FORWARD) && ((data->Flags.value & MSN_ACCEPT) == 0) && ((data->Flags.value & MSN_BLOCKED) == 0)){
                    if (packet)
                        packet->send();
                    packet = new AddPacket(this, "AL", data->EMail.ptr, quote(QString::fromUtf8(data->ScreenName.ptr)).utf8(), 0);
                    data->Group.value = grp_id;
                    data->Flags.value |= MSN_ACCEPT;
                }
                if (data->Group.value != grp_id){
                    if (packet)
                        packet->send();
                    packet = new AddPacket(this, "FL", data->EMail.ptr, quote(QString::fromUtf8(data->ScreenName.ptr)).utf8(), grp_id);
                    packet->send();
                    packet = NULL;
                    packet = new RemPacket(this, "FL", data->EMail.ptr, data->Group.value);
                    data->Group.value = grp_id;
                }
                if (contact->getName() != QString::fromUtf8(data->ScreenName.ptr)){
                    if (packet)
                        packet->send();
                    packet = new ReaPacket(this, data->EMail.ptr, quote(contact->getName()).utf8());
                    set_str(&data->ScreenName.ptr, contact->getName().utf8());
                }
            }
            break;
        case LR_CONTACTxREMOVED:
            packet = new RemPacket(this, "AL", (*it).Name.c_str());
            packet->send();
            packet = new RemPacket(this, "FL", (*it).Name.c_str());
            setNDeleted(getNDeleted() + 1);
            setDeleted(getNDeleted(), (*it).Name.c_str());
            break;
        case LR_CONTACTxREMOVED_BL:
            packet = new RemPacket(this, "BL", (*it).Name.c_str());
            break;
        case LR_GROUPxCHANGED:
            grp = getContacts()->group(strtoul((*it).Name.c_str(), NULL, 10));
            if (grp){
                ClientDataIterator it(grp->clientData, this);
                data = (MSNUserData*)(++it);
                if (data){
                    packet = new RegPacket(this, data->Group.value, quote(grp->getName()).utf8());
                }else{
                    packet = new AdgPacket(this, grp->id(), quote(grp->getName()).utf8());
                    data = (MSNUserData*)(grp->clientData.createData(this));
                }
                set_str(&data->ScreenName.ptr, grp->getName().utf8());
            }
            break;
        case LR_GROUPxREMOVED:
            packet = new RmgPacket(this, strtoul((*it).Name.c_str(), NULL, 10));
            break;
        }
        if (packet)
            packet->send();
    }
    m_requests.clear();
}

bool MSNClient::add(const char *mail, const char *name, unsigned grp)
{
    Contact *contact;
    MSNUserData *data = findContact(mail, contact);
    if (data){
        if (contact->getGroup() != grp){
            contact->setGroup(grp);
            Event e(EventContactChanged, contact);
            e.process();
        }
        return false;
    }
    data = findContact(mail, name, contact);
    if (data == NULL)
        return false;
    contact->setGroup(grp);
    Event e(EventContactChanged, contact);
    e.process();
    return true;
}

bool MSNClient::compareData(void *d1, void *d2)
{
    return strcmp(((MSNUserData*)d1)->EMail.ptr, ((MSNUserData*)d2)->EMail.ptr) == 0;
}

static void addIcon(string *s, const char *icon, const char *statusIcon)
{
    if (s == NULL)
        return;
    if (statusIcon && !strcmp(statusIcon, icon))
        return;
    string str = *s;
    while (!str.empty()){
        string item = getToken(str, ',');
        if (item == icon)
            return;
    }
    if (!s->empty())
        *s += ',';
    *s += icon;
}

void MSNClient::contactInfo(void *_data, unsigned long &curStatus, unsigned&, const char *&statusIcon, string *icons)
{
    MSNUserData *data = (MSNUserData*)_data;
    unsigned cmp_status = data->Status.value;
    const CommandDef *def;
    for (def = protocol()->statusList(); def->text; def++){
        if (def->id == cmp_status)
            break;
    }
    if ((cmp_status == STATUS_BRB) || (cmp_status == STATUS_PHONE) || (cmp_status == STATUS_LUNCH))
        cmp_status = STATUS_AWAY;
    if (data->Status.value > curStatus){
        curStatus = data->Status.value;
        if (statusIcon && icons){
            string iconSave = *icons;
            *icons = statusIcon;
            if (iconSave.length())
                addIcon(icons, iconSave.c_str(), statusIcon);
        }
        statusIcon = def->icon;
    }else{
        if (statusIcon){
            addIcon(icons, def->icon, statusIcon);
        }else{
            statusIcon = def->icon;
        }
    }
    if (icons && data->typing_time.value)
        addIcon(icons, "typing", statusIcon);
}

QString MSNClient::contactTip(void *_data)
{
    MSNUserData *data = (MSNUserData*)_data;
    unsigned long status = STATUS_UNKNOWN;
    unsigned style  = 0;
    const char *statusIcon = NULL;
    contactInfo(data, status, style, statusIcon);
    QString res;
    res += "<img src=\"icon:";
    res += statusIcon;
    res += "\">";
    QString statusText;
    for (const CommandDef *cmd = protocol()->statusList(); cmd->text; cmd++){
        if (!strcmp(cmd->icon, statusIcon)){
            res += " ";
            statusText = i18n(cmd->text);
            res += statusText;
            break;
        }
    }
    res += "<br>";
    res += QString::fromUtf8(data->EMail.ptr);
    res += "</b>";
    if (data->Status.value == STATUS_OFFLINE){
        if (data->StatusTime.value){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatDateTime(data->StatusTime.value);
        }
    }else{
        if (data->OnlineTime.value){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatDateTime(data->OnlineTime.value);
        }
        if (data->Status.value != STATUS_ONLINE){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatDateTime(data->StatusTime.value);
        }
    }
    if (data->IP.ptr){
        res += "<br>";
        res += formatAddr(data->IP, data->Port.value);
    }
    if (data->RealIP.ptr && ((data->IP.ptr == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        res += "<br>";
        res += formatAddr(data->RealIP, data->Port.value);
    }
    return res;
}

QWidget *MSNClient::searchWindow()
{
    return new MSNSearch(this);
}

SBSocket::SBSocket(MSNClient *client, Contact *contact, MSNUserData *data)
{
    m_state		= Unknown;
    m_client	= client;
    m_contact	= contact;
    m_data		= data;
    m_socket	= new ClientSocket(this, client->createSBSocket());
    m_packet_id = 0;
    m_messageSize = 0;
    m_invite_cookie = get_random();
    m_bTyping	= false;
    m_client->m_SBsockets.push_back(this);
}

SBSocket::~SBSocket()
{
    if (m_packet)
        m_packet->clear();
    if (m_socket)
        delete m_socket;
    list<SBSocket*>::iterator it = find(m_client->m_SBsockets.begin(), m_client->m_SBsockets.end(), this);
    if (it != m_client->m_SBsockets.end())
        m_client->m_SBsockets.erase(it);
    if (m_data){
        m_data->sb.ptr = NULL;
        if (m_data->typing_time.value){
            m_data->typing_time.value = 0;
            Event e(EventContactStatus, m_contact);
            e.process();
        }
    }
    list<Message*>::iterator itm;
    for (itm = m_queue.begin(); itm != m_queue.end(); ++itm){
        Message *msg = (*itm);
        msg->setError(I18N_NOOP("Contact go offline"));
        Event e(EventMessageSent, msg);
        e.process();
        delete msg;
    }
    list<msgInvite>::iterator itw;
    for (itw = m_waitMsg.begin(); itw != m_waitMsg.end(); ++itw){
        Message *msg = (*itw).msg;
        msg->setError(I18N_NOOP("Contact go offline"));
        Event e(EventMessageSent, msg);
        e.process();
        delete msg;
    }
    for (itw = m_acceptMsg.begin(); itw != m_acceptMsg.end(); ++itw){
        Message *msg = (*itw).msg;
        Event e(EventMessageDeleted, msg);
        e.process();
        delete msg;
    }
}

void SBSocket::connect()
{
    m_packet = new XfrPacket(m_client, this);
    m_packet->send();
}

void SBSocket::connect(const char *addr, const char *session, const char *cookie, bool bDirection)
{
    m_packet = NULL;
    if (m_state != Unknown){
        log(L_DEBUG, "Connect in bad state");
        return;
    }
    if (bDirection){
        m_state = ConnectingSend;
    }else{
        m_state = ConnectingReceive;
    }
    m_cookie = cookie;
    m_session = session;
    string ip = addr;
    unsigned short port = 0;
    int n = ip.find(':');
    if (n > 0){
        port = (unsigned short)atol(ip.substr(n + 1).c_str());
        ip = ip.substr(0, n);
    }
    if (port == 0){
        m_socket->error_state("Bad address");
        return;
    }
    m_socket->connect(ip.c_str(), port, m_client);
}

bool SBSocket::send(Message *msg)
{

    m_bTyping = false;
    m_queue.push_back(msg);
    switch (m_state){
    case Unknown:
        connect();
        break;
    case Connected:
        process();
        break;
    default:
        break;
    }
    return true;
}

bool SBSocket::error_state(const char*, unsigned)
{
    if (m_queue.size()){
        m_socket->close();
        connect();
        return false;
    }
    return true;
}

void SBSocket::connect_ready()
{
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    m_socket->setRaw(true);
    string args = m_client->data.owner.EMail.ptr;
    args += " ";
    args += m_cookie;
    m_cookie = "";
    switch (m_state){
    case ConnectingSend:
        send("USR", args.c_str());
        m_state = WaitJoin;
        break;
    case ConnectingReceive:
        args += " ";
        args += m_session;
        send("ANS", args.c_str());
        m_state = Connected;
        process();
        break;
    default:
        log(L_WARN, "Bad state for connect ready");
    }
}

void SBSocket::packet_ready()
{
    if (m_socket->readBuffer.writePos() == 0)
        return;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->MSNPacket);
    for (;;){
        if (m_messageSize && !getMessage())
            break;
        string s;
        if (!m_socket->readBuffer.scan("\r\n", s))
            break;
        getLine(s.c_str());
    }
    if (m_socket->readBuffer.readPos() == m_socket->readBuffer.writePos())
        m_socket->readBuffer.init(0);
}

void SBSocket::getMessage(unsigned size)
{
    m_messageSize = size;
    m_message = "";
    getMessage();
}

bool SBSocket::getMessage()
{
    unsigned tail = m_socket->readBuffer.writePos() - m_socket->readBuffer.readPos();
    if (tail > m_messageSize)
        tail = m_messageSize;
    unsigned len = m_message.length();
    m_message.append(tail, '\x00');
    m_socket->readBuffer.unpack((char*)m_message.c_str() + len, tail);
    m_messageSize -= tail;
    if (m_messageSize)
        return false;
    messageReady();
    return true;
}

void SBSocket::send(const char *cmd, const char *args)
{
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer
    << cmd
    << " "
    << number(++m_packet_id).c_str();
    if (args){
        m_socket->writeBuffer
        << " "
        << args;
    }
    m_socket->writeBuffer << "\r\n";
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->MSNPacket);
    m_socket->write();
}

void SBSocket::getLine(const char *_line)
{
    string line = _line;
    string cmd = getToken(line, ' ');
    if (cmd == "BYE"){
        m_socket->error_state("");
        return;
    }
    if (cmd == "MSG"){
        string email = getToken(line, ' ');
        getToken(line, ' ');
        unsigned size = atol(line.c_str());
        getMessage(size);
    }
    if (cmd == "JOI"){
        if (m_state != WaitJoin){
            log(L_WARN, "JOI in bad state");
            return;
        }
        m_state = Connected;
        process();
    }
    if (cmd == "USR")
        send("CAL", m_data->EMail.ptr);
    if ((cmd == "ACK") || (cmd == "NAK")){
        unsigned id = atol(getToken(line, ' ').c_str());
        if (id != m_msg_id){
            log(L_WARN, "Bad ACK id");
            return;
        }
        if (m_queue.empty())
            return;
        Message *msg = m_queue.front();
        if (cmd == "NAK"){
            m_msgText = "";
            msg->setError(I18N_NOOP("Send message failed"));
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            m_queue.erase(m_queue.begin());
            process(false);
            return;
        }
        if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
            Message m(MessageGeneric);
            m.setContact(m_contact->id());
            m.setClient(m_client->dataName(m_data).c_str());
            m.setText(m_msgPart);
            m.setForeground(msg->getForeground());
            m.setBackground(0xFFFFFF);
            m.setFont(msg->getFont());
            Event e(EventSent, &m);
            e.process();
        }
        if (m_msgText.isEmpty()){
            if (msg->type() == MessageFile){
                sendFile();
            }else{
                Event e(EventMessageSent, msg);
                e.process();
                delete msg;
                m_queue.erase(m_queue.begin());
            }
        }
        process();
    }
}

typedef map<string, string>	STR_VALUES;

static STR_VALUES parseValues(const char *str)
{
    STR_VALUES res;
    string s = trim(str);
    while (!s.empty()){
        string p = trim(getToken(s, ';', false).c_str());
        string key = getToken(p, '=', false);
        STR_VALUES::iterator it = res.find(key);
        if (it == res.end()){
            res.insert(STR_VALUES::value_type(key, p));
        }else{
            (*it).second = p;
        }
        s = trim(s.c_str());
    }
    return res;
}

static char FT_GUID[] = "{5D3E02AB-6190-11d3-BBBB-00C04F795683}";

void SBSocket::messageReady()
{
    log(L_DEBUG, "MSG: [%s]", m_message.c_str());
    string content_type;
    string charset;
    string font;
    string typing;
    unsigned color = 0;
    bool bColor = false;
    while (!m_message.empty()){
        int n = m_message.find("\r\n");
        if (n < 0){
            log(L_DEBUG, "Error parse message");
            return;
        }
        if (n == 0){
            m_message = m_message.substr(2);
            break;
        }
        string line = m_message.substr(0, n);
        m_message = m_message.substr(n + 2);
        string key = getToken(line, ':', false);
        if (key == "Content-Type"){
            line = trim(line.c_str());
            content_type = trim(getToken(line, ';').c_str());
            STR_VALUES v = parseValues(trim(line.c_str()).c_str());
            STR_VALUES::iterator it = v.find("charset");
            if (it != v.end())
                charset = (*it).second;
            continue;
        }
        if (key == "X-MMS-IM-Format"){
            STR_VALUES v = parseValues(trim(line.c_str()).c_str());
            STR_VALUES::iterator it = v.find("FN");
            if (it != v.end())
                font = m_client->unquote(QString::fromUtf8((*it).second.c_str())).utf8();
            it = v.find("EF");
            if (it != v.end()){
                string effects = (*it).second;
                if (effects.find('B') < effects.length())
                    font += ", bold";
                if (effects.find('I') < effects.length())
                    font += ", italic";
                if (effects.find('S') < effects.length())
                    font += ", strikeout";
                if (effects.find('U') < effects.length())
                    font += ", underline";
            }
            it = v.find("CO");
            if (it != v.end())
                color = QString((*it).second.c_str()).toULong(&bColor, 16);
            continue;
        }
        if (key == "TypingUser"){
            typing = trim(line.c_str());
            continue;
        }
    }
    if (content_type == "text/plain"){
        if (m_data->typing_time.value){
            m_data->typing_time.value = 0;
            Event e(EventContactStatus, m_contact);
            e.process();
        }
        QString msg_text = QString::fromUtf8(m_message.c_str());
        msg_text = msg_text.replace(QRegExp("\\r"), "");
        Message *msg = new Message(MessageGeneric);
        msg->setFlags(MESSAGE_RECEIVED);
        if (bColor){
            msg->setBackground(0xFFFFFF);
            msg->setForeground(color);
        }
        msg->setFont(font.c_str());
        msg->setText(msg_text);
        msg->setContact(m_contact->id());
        msg->setClient(m_client->dataName(m_data).c_str());
        Event e(EventMessageReceived, msg);
        if (!e.process())
            delete msg;
        return;
    }
    if (content_type == "text/x-msmsgscontrol"){
        if (QString(typing.c_str()).lower() == QString(m_data->EMail.ptr).lower()){
            time_t now;
            time(&now);
            bool bEvent = (m_data->typing_time.value == 0);
            m_data->typing_time.value = now;
            if (bEvent){
                Event e(EventContactStatus, m_contact);
                e.process();
            }
        }
    }
    if (content_type == "text/x-msmsgsinvite"){
        string file;
        string command;
        string guid;
        string code;
        string ip_address;
        string ip_address_internal;
        unsigned short port = 0;
        unsigned short port_x = 0;
        unsigned cookie = 0;
        unsigned auth_cookie = 0;
        unsigned fileSize = 0;
        while (!m_message.empty()){
            string line;
            int n = m_message.find("\r\n");
            if (n < 0){
                line = m_message;
                m_message = "";
            }else{
                line = m_message.substr(0, n);
                m_message = m_message.substr(n + 2);
            }
            string key = getToken(line, ':', false);
            if (key == "Application-GUID"){
                guid = trim(line.c_str());
                continue;
            }
            if (key == "Invitation-Command"){
                command = trim(line.c_str());
                continue;
            }
            if (key == "Invitation-Cookie"){
                cookie = strtoul(trim(line.c_str()).c_str(), NULL, 10);
                continue;
            }
            if (key == "Application-File"){
                file = trim(line.c_str());
                continue;
            }
            if (key == "Application-FileSize"){
                fileSize = strtoul(trim(line.c_str()).c_str(), NULL, 10);
                continue;
            }
            if (key == "Cancel-Code"){
                code = trim(line.c_str());
                continue;
            }
            if (key == "IP-Address"){
                ip_address = trim(line.c_str());
                continue;
            }
            if (key == "IP-Address-Internal"){
                ip_address_internal = trim(line.c_str());
                continue;
            }
            if (key == "Port"){
                port = (unsigned short)atol(trim(line.c_str()).c_str());
                continue;
            }
            if (key == "PortX"){
                port_x = (unsigned short)atol(trim(line.c_str()).c_str());
                continue;
            }
            if (key == "AuthCookie"){
                auth_cookie = strtoul(trim(line.c_str()).c_str(), NULL, 10);
                continue;
            }

        }
        if (cookie == 0){
            log(L_WARN, "No cookie in message");
            return;
        }
        if (command == "INVITE"){
            if (guid != FT_GUID){
                log(L_WARN, "Unknown GUID %s", guid.c_str());
                return;
            }
            if (file.empty()){
                log(L_WARN, "No file in message");
                return;
            }
            FileMessage *msg = new FileMessage;
            msg->setDescription(m_client->unquote(QString::fromUtf8(file.c_str())));
            msg->setSize(fileSize);
            msg->setFlags(MESSAGE_RECEIVED | MESSAGE_TEMP);
            msg->setContact(m_contact->id());
            msg->setClient(m_client->dataName(m_data).c_str());
            msgInvite m;
            m.msg    = msg;
            m.cookie = cookie;
            m_acceptMsg.push_back(m);
            Event e(EventMessageReceived, msg);
            if (e.process()){
                for (list<msgInvite>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
                    if ((*it).msg == msg){
                        m_acceptMsg.erase(it);
                        break;
                    }
                }
            }
        }else if (command == "ACCEPT"){
            unsigned ip      = inet_addr(ip_address.c_str());
            unsigned real_ip = inet_addr(ip_address_internal.c_str());
            if (ip != INADDR_NONE)
                set_ip(&m_data->IP, ip);
            if (real_ip != INADDR_NONE)
                set_ip(&m_data->RealIP, real_ip);
            if (port)
                m_data->Port.value = port;
            list<msgInvite>::iterator it;
            for (it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
                if ((*it).cookie == cookie){
                    Message *msg = (*it).msg;
                    if (msg->type() == MessageFile){
                        m_waitMsg.erase(it);
                        FileMessage *m = static_cast<FileMessage*>(msg);
                        MSNFileTransfer *ft;
                        bool bNew = false;
                        if (m->m_transfer){
                            ft = static_cast<MSNFileTransfer*>(m->m_transfer);
                        }else{
                            ft = new MSNFileTransfer(m, m_client, m_data);
                            bNew = true;
                        }
                        ft->ip1   = real_ip;
                        ft->port1 = port;
                        ft->ip2	  = ip;
                        ft->port2 = port_x;
                        ft->auth_cookie = auth_cookie;

                        if (bNew){
                            Event e(EventMessageAcked, msg);
                            e.process();
                        }
                        ft->connect();
                        break;
                    }
                    msg->setError("Bad type");
                    Event e(EventMessageSent, msg);
                    e.process();
                    delete msg;
                    return;
                }
            }
            if (it == m_waitMsg.end())
                log(L_WARN, "No message for accept");
            return;
        }else if (command == "CANCEL"){
            if (code == "REJECT"){
                list<msgInvite>::iterator it;
                for (it = m_waitMsg.begin(); it != m_waitMsg.end(); ++it){
                    if ((*it).cookie == cookie){
                        Message *msg = (*it).msg;
                        msg->setError(I18N_NOOP("Message declined"));
                        Event e(EventMessageSent, msg);
                        e.process();
                        delete msg;
                        m_waitMsg.erase(it);
                        break;
                    }
                }
                if (it == m_waitMsg.end())
                    log(L_WARN, "No message for cancel");
                return;
            }
            list<msgInvite>::iterator it;
            for (it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
                if ((*it).cookie == cookie){
                    Message *msg = (*it).msg;
                    Event e(EventMessageDeleted, msg);
                    e.process();
                    delete msg;
                    m_acceptMsg.erase(it);
                    break;
                }
            }
            if (it == m_acceptMsg.end())
                log(L_WARN, "No message for cancel");
        }else{
            log(L_WARN, "Unknown command %s", command.c_str());
            return;
        }
    }
}

void SBSocket::timer(unsigned now)
{
    if (m_data->typing_time.value){
        if (now >= m_data->typing_time.value + TYPING_TIME){
            m_data->typing_time.value = 0;
            Event e(EventContactStatus, m_contact);
            e.process();
        }
    }
    sendTyping();
}

void SBSocket::setTyping(bool bTyping)
{
    if (m_bTyping == bTyping)
        return;
    m_bTyping = bTyping;
    sendTyping();
}

void SBSocket::sendTyping()
{
    if (m_bTyping && (m_state == Connected)){
        string message;
        message += "MIME-Version: 1.0\r\n";
        message += "Content-Type: text/x-msmsgcontrol\r\n";
        message += "TypingUser: ";
        message += m_client->data.owner.EMail.ptr;
        message += "\r\n";
        message += "\r\n";
        sendMessage(message.c_str(), "U");
    }
}

void SBSocket::sendMessage(const char *str, const char *type)
{
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer
    << "MSG "
    << number(++m_packet_id).c_str()
    << " "
    << type
    << " "
    << number(strlen(str)).c_str()
    << "\r\n"
    << str;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->MSNPacket);
    m_socket->write();
}

bool SBSocket::cancelMessage(Message *msg)
{
    if (m_queue.empty())
        return false;
    if (m_queue.front() == msg){
        m_msgPart = "";
        m_msgText = "";
        m_msg_id = 0;
        m_queue.erase(m_queue.begin());
        process();
        return true;
    }
    list<Message*>::iterator it = find(m_queue.begin(), m_queue.end(), msg);
    if (it == m_queue.end())
        return false;
    m_queue.erase(it);
    delete msg;
    return true;
}

void SBSocket::sendFile()
{
    if (m_queue.empty())
        return;
    Message *msg = m_queue.front();
    if (msg->type() != MessageFile)
        return;
    m_queue.erase(m_queue.begin());
    FileMessage *m = static_cast<FileMessage*>(msg);
    if (++m_invite_cookie == 0)
        m_invite_cookie++;
    msgInvite mi;
    mi.msg    = msg;
    mi.cookie = m_invite_cookie;
    m_waitMsg.push_back(mi);
    string message;
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
               "Application-Name: File Transfer\r\n"
               "Application-GUID: ";
    message += FT_GUID;
    message += "\r\n"
               "Invitation-Command: INVITE\r\n"
               "Invitation-Cookie: ";
    message += number(m_invite_cookie);
    message += "\r\n"
               "Application-File: ";
    QString name;
    unsigned size;
    if (m->m_transfer){
        name = m->m_transfer->m_file->name();
        size = m->m_transfer->fileSize();
    }else{
        FileMessage::Iterator it(*m);
        if (it[0])
            name = *it[0];
        size = it.size();
    }
    name = name.replace(QRegExp("\\\\"), "/");
    int n = name.findRev("/");
    if (n >= 0)
        name = name.mid(n + 1);
    message += m_client->quote(name).utf8();
    message += "\r\n"
               "Application-FileSize: ";
    message += number(size);
    message += "\r\n"
               "Connectivity: N\r\n\r\n";
    sendMessage(message.c_str(), "S");
}

void SBSocket::process(bool bTyping)
{
    if (bTyping)
        sendTyping();
    if (m_msgText.isEmpty() && !m_queue.empty()){
        Message *msg = m_queue.front();
        string text;
        text = msg->getPlainText().utf8();
        messageSend ms;
        ms.msg  = msg;
        ms.text = &text;
        Event e(EventSend, &ms);
        e.process();
        m_msgText = QString::fromUtf8(text.c_str());
        if (msg->type() == MessageUrl){
            UrlMessage *m = static_cast<UrlMessage*>(msg);
            QString msgText = m->getUrl();
            msgText += "\r\n";
            msgText += m_msgText;
            m_msgText = msgText;
        }
        if ((msg->type() == MessageFile) && static_cast<FileMessage*>(msg)->m_transfer)
            m_msgText = "";
        if (m_msgText.isEmpty()){
            if (msg->type() == MessageFile){
                sendFile();
                return;
            }
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            m_queue.erase(m_queue.begin());
        }
        m_msgText = m_msgText.replace(QRegExp("\\n"), "\r\n");
    }
    if (m_msgText.isEmpty())
        return;
    m_msgPart = getPart(m_msgText, 1664);
    Message *msg = m_queue.front();
    char color[10];
    sprintf(color, "%06lX", msg->getBackground());
    string message;
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/plain; charset=UTF-8\r\n";
    message += "X-MMS_IM-Format: ";
    if (msg->getFont()){
        string font = msg->getFont();
        if (!font.empty()){
            string font_type;
            int n = font.find(", ");
            if (n > 0){
                font_type = font.substr(n + 2);
                font = font.substr(0, n);
            }
            message += "FN=";
            message += m_client->quote(QString::fromUtf8(font.c_str())).utf8();
            string effect;
            while (!font_type.empty()){
                string type = font_type;
                int n = font_type.find(", ");
                if (n > 0){
                    type = font_type.substr(n);
                    font_type = font_type.substr(n + 2);
                }else{
                    font_type = "";
                }
                if (type == "bold")
                    effect += "B";
                if (type == "italic")
                    effect += "I";
                if (type == "strikeout")
                    effect += "S";
                if (type == "underline")
                    effect += "U";
            }
            if (!effect.empty()){
                message += "; EF=";
                message += effect;
            }
        }
    }
    message += "; CO=";
    message += color;
    message += "; CS=0\r\n";
    message += "\r\n";
    message += m_msgPart.utf8();
    sendMessage(message.c_str(), "A");
    m_msg_id = m_packet_id;
}

MSNFileTransfer::MSNFileTransfer(FileMessage *msg, MSNClient *client, MSNUserData *data)
        : FileTransfer(msg)
{
    m_socket = new ClientSocket(this);
    m_client = client;
    m_state  = None;
    m_data	 = data;
    m_timer  = NULL;
    m_size   = msg->getSize();
    m_bHeader  = false;
    m_nFiles   = 1;
}

MSNFileTransfer::~MSNFileTransfer()
{
    if (m_socket)
        delete m_socket;
}


void MSNFileTransfer::setSocket(Socket *s)
{
    m_state  = Incoming;
    m_socket->setSocket(s);
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    m_socket->setRaw(true);
    send("VER MSNFTP");
    FileTransfer::m_state = FileTransfer::Negotiation;
    if (m_notify)
        m_notify->process();
}

void MSNFileTransfer::listen()
{
    if (m_notify)
        m_notify->createFile(m_msg->getDescription(), m_size, false);
}

void MSNFileTransfer::connect()
{
    FileTransfer::m_state = FileTransfer::Connect;
    if (m_notify)
        m_notify->process();
    if ((m_state == None) || (m_state == Wait)){
        m_state = ConnectIP1;
        if (ip1 && port1){
            struct in_addr addr;
            addr.s_addr = ip1;
            m_socket->connect(inet_ntoa(addr), port1, NULL);
            return;
        }
    }
    if (m_state == ConnectIP1){
        m_state = ConnectIP2;
        if (ip2 && port2){
            struct in_addr addr;
            addr.s_addr = ip2;
            m_socket->connect(inet_ntoa(addr), port2, NULL);
            return;
        }
    }
    if (m_state == ConnectIP2){
        m_state = ConnectIP3;
        if (ip2 && port1){
            struct in_addr addr;
            addr.s_addr = ip2;
            m_socket->connect(inet_ntoa(addr), port1, NULL);
            return;
        }
    }
    error_state(I18N_NOOP("Can't established direct connection"), 0);
}

bool MSNFileTransfer::error_state(const char *err, unsigned)
{
    if (m_state == WaitDisconnect)
        FileTransfer::m_state = FileTransfer::Done;
    if (m_state == ConnectIP1){
        connect();
        return false;
    }
    if (m_state == Wait)
        return false;
    if (FileTransfer::m_state != FileTransfer::Done){
        m_state = None;
        FileTransfer::m_state = FileTransfer::Error;
        m_msg->setError(err);
    }
    m_msg->m_transfer = NULL;
    m_msg->setFlags(m_msg->getFlags() & ~MESSAGE_TEMP);
    Event e(EventMessageSent, m_msg);
    e.process();
    return true;
}

void MSNFileTransfer::packet_ready()
{
    if (m_state == Receive){
        if (m_bHeader){
            char cmd;
            char s1, s2;
            m_socket->readBuffer >> cmd >> s1 >> s2;
            log(L_DEBUG, "MSN FT header: %02X %02X %02X", cmd & 0xFF, s1 & 0xFF, s2 & 0xFF);
            if (cmd != 0){
                m_socket->error_state(I18N_NOOP("Transfer canceled"), 0);
                return;
            }
            unsigned size = (unsigned char)s1 + ((unsigned char)s2 << 8);
            m_bHeader = false;
            log(L_DEBUG, "MSN FT header: %u", size);
            m_socket->readBuffer.init(size);
        }else{
            unsigned size = m_socket->readBuffer.size();
            if (size == 0)
                return;
            log(L_DEBUG, "MSN FT data: %u", size);
            m_file->writeBlock(m_socket->readBuffer.data(), size);
            m_socket->readBuffer.incReadPos(size);
            m_bytes      += size;
            m_totalBytes += size;
            if (m_notify)
                m_notify->process();
            m_size -= size;
            if (m_size <= 0){
                m_socket->readBuffer.init(0);
                m_socket->setRaw(true);
                send("BYE 16777989");
                m_state = WaitDisconnect;
                if (m_notify)
                    m_notify->transfer(false);
                return;
            }
            m_bHeader = true;
            m_socket->readBuffer.init(3);
        }
        return;
    }
    if (m_socket->readBuffer.writePos() == 0)
        return;
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->MSNPacket);
    for (;;){
        string s;
        if (!m_socket->readBuffer.scan("\r\n", s))
            break;
        if (getLine(s.c_str()))
            return;
    }
    if (m_socket->readBuffer.readPos() == m_socket->readBuffer.writePos())
        m_socket->readBuffer.init(0);
}

void MSNFileTransfer::connect_ready()
{
    log(L_DEBUG, "Connect ready");
    m_state = Connected;
    FileTransfer::m_state = Negotiation;
    if (m_notify)
        m_notify->process();
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    m_socket->setRaw(true);
}

void MSNFileTransfer::startReceive(unsigned pos)
{
    if (pos > m_size){
        FileTransfer::m_state = FileTransfer::Done;
        m_state = None;
        if (m_data->sb.ptr)
            ((SBSocket*)(m_data->sb.ptr))->declineMessage(cookie);
        m_socket->error_state("", 0);
        return;
    }
    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(FT_TIMEOUT * 1000);
    m_state = Listen;
    FileTransfer::m_state = FileTransfer::Listen;
    if (m_notify)
        m_notify->process();
    bind(m_client->getMinPort(), m_client->getMaxPort(), m_client);
}

void MSNFileTransfer::send(const char *line)
{
    log(L_DEBUG, "Send: %s", line);
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer << line;
    m_socket->writeBuffer << "\r\n";
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->MSNPacket);
    m_socket->write();
}

bool MSNFileTransfer::getLine(const char *line)
{
    QString l = QString::fromUtf8(line);
    l = l.replace(QRegExp("\r"), "");
    QCString ll = l.local8Bit();
    log(L_DEBUG, "Get: %s", (const char*)ll);
    QString cmd = getToken(l, ' ');
    if ((cmd == "VER") && (l == "MSNFTP")){
        if (m_state == Incoming){
            string usr = "USR ";
            usr += m_client->quote(QString::fromUtf8(m_client->data.owner.EMail.ptr)).utf8();
            usr += " ";
            usr += number(auth_cookie);
            send(usr.c_str());
        }else{
            send("VER MSNFTP");
        }
        return false;
    }
    if (cmd == "USR"){
        QString mail = m_client->unquote(getToken(l, ' '));
        unsigned auth = l.toUInt();
        if (mail.lower() != QString(m_data->EMail.ptr).lower()){
            error_state("Bad address", 0);
            return false;
        }
        if (auth != auth_cookie){
            error_state("Bad auth cookie", 0);
            return false;
        }
        if ((m_file == NULL) && !openFile()){
            if (FileTransfer::m_state == FileTransfer::Done)
                m_socket->error_state("");
            if (m_notify)
                m_notify->transfer(false);
            return false;
        }
        string cmd = "FIL ";
        cmd += number(m_fileSize);
        send(cmd.c_str());
        return false;
    }
    if (cmd == "TFR"){
        FileTransfer::m_state = FileTransfer::Write;
        m_state = Send;
        if (m_notify)
            m_notify->transfer(true);
        write_ready();
        return false;
    }
    if (cmd == "FIL"){
        send("TFR");
        m_bHeader = true;
        m_socket->readBuffer.init(3);
        m_socket->readBuffer.packetStart();
        m_state = Receive;
        m_socket->setRaw(false);
        FileTransfer::m_state = FileTransfer::Read;
        m_size = strtoul(l.latin1(), NULL, 10);
        m_bytes = 0;
        if (m_notify){
            m_notify->transfer(true);
            m_notify->process();
        }
        return true;
    }
    if (cmd == "BYE"){
        if (m_notify)
            m_notify->transfer(false);
        if (!openFile()){
            if (FileTransfer::m_state == FileTransfer::Done)
                m_socket->error_state("");
        }else{
            m_state = Wait;
            FileTransfer::m_state = FileTransfer::Wait;
            if (!((Client*)m_client)->send(m_msg, m_data))
                error_state(I18N_NOOP("File transfer failed"), 0);
        }
        if (m_notify)
            m_notify->process();
        m_socket->close();
        return true;
    }
    error_state("Bad line", 0);
    return false;
}

void MSNFileTransfer::timeout()
{
}

void MSNFileTransfer::write_ready()
{
    if (m_state != Send){
        ClientSocketNotify::write_ready();
        return;
    }
    if (m_transfer){
        m_transferBytes += m_transfer;
        m_transfer = 0;
        if (m_notify)
            m_notify->process();
    }
    if (m_bytes >= m_fileSize){
        m_state = WaitBye;
        return;
    }
    time_t now;
    time(&now);
    if ((unsigned)now != m_sendTime){
        m_sendTime = now;
        m_sendSize = 0;
    }
    if (m_sendSize > (m_speed << 18)){
        m_socket->pause(1);
        return;
    }
    unsigned long tail = m_fileSize - m_bytes;
    if (tail > MAX_FT_PACKET) tail = MAX_FT_PACKET;
    m_socket->writeBuffer.packetStart();
    char buf[MAX_FT_PACKET + 3];
    buf[0] = 0;
    buf[1] = (char)(tail & 0xFF);
    buf[2] = (char)((tail >> 8) & 0xFF);
    int readn = m_file->readBlock(&buf[3], tail);
    if (readn <= 0){
        m_socket->error_state("Read file error");
        return;
    }
    m_transfer    = readn;
    m_bytes      += readn;
    m_totalBytes += readn;
    m_sendSize   += readn;
    m_socket->writeBuffer.pack(buf, readn + 3);
    m_socket->write();
}

bool MSNFileTransfer::accept(Socket *s, unsigned long ip)
{
    struct in_addr addr;
    addr.s_addr = ip;
    log(L_DEBUG, "Accept direct connection %s", inet_ntoa(addr));
    m_socket->setSocket(s);
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    m_socket->setRaw(true);
    FileTransfer::m_state = Negotiation;
    m_state = Incoming;
    if (m_notify)
        m_notify->process();
    send("VER MSNFTP");
    return true;
}

void MSNFileTransfer::bind_ready(unsigned short port)
{
    if (m_data->sb.ptr == NULL){
        error_state("No switchboard socket", 0);
        return;
    }
    ((SBSocket*)(m_data->sb.ptr))->acceptMessage(port, cookie, auth_cookie);
}

bool MSNFileTransfer::error(const char *err)
{
    return error_state(err, 0);
}

void SBSocket::acceptMessage(unsigned short port, unsigned cookie, unsigned auth_cookie)
{
    string message;
    message += "MIME-Version: 1.0\r\n"
               "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
               "IP-Address: ";
    struct in_addr addr;
    addr.s_addr = get_ip(m_client->data.owner.IP);
    message += inet_ntoa(addr);
    message += "\r\n"
               "IP-Address-Internal: ";
    addr.s_addr = m_client->m_socket->localHost();
    message += inet_ntoa(addr);
    message += "\r\n"
               "Port: ";
    message += number(port);
    message += "\r\n"
               "AuthCookie: ";
    message += number(auth_cookie);
    message += "\r\n"
               "Sender-Connect: TRUE\r\n"
               "Invitation-Command: ACCEPT\r\n"
               "Invitation-Cookie: ";
    message += number(cookie);
    message += "\r\n"
               "Launch-Application: FALSE\r\n"
               "Request-Data: IP-Address:\r\n\r\n";
    sendMessage(message.c_str(), "N");
}

bool SBSocket::acceptMessage(Message *msg, const char *dir, OverwriteMode mode)
{
    for (list<msgInvite>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
        if ((*it).msg->id() != msg->id())
            continue;
        Message *msg = (*it).msg;
        unsigned cookie = (*it).cookie;
        m_acceptMsg.erase(it);
        MSNFileTransfer *ft = new MSNFileTransfer(static_cast<FileMessage*>(msg), m_client, m_data);
        ft->setDir(QFile::encodeName(dir));
        ft->setOverwrite(mode);
        ft->auth_cookie = get_random();
        ft->cookie = cookie;
        Event e(EventMessageAcked, msg);
        e.process();
        ft->listen();
        Event eDel(EventMessageDeleted, msg);
        eDel.process();
        return true;
    }
    return false;
}

void SBSocket::declineMessage(unsigned cookie)
{
    string message;
    message += "MIME-Version: 1.0\r\n"
               "Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
               "Invitation-Command: CANCEL\r\n"
               "Invitation-Cookie: ";
    message += number(cookie);
    message += "\r\n"
               "Cancel-Code: REJECT\r\n\r\n";
    sendMessage(message.c_str(), "S");
}

bool SBSocket::declineMessage(Message *msg, const char *reason)
{
    for (list<msgInvite>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
        if ((*it).msg->id() != msg->id())
            continue;
        Message *msg = (*it).msg;
        unsigned cookie = (*it).cookie;
        m_acceptMsg.erase(it);
        declineMessage(cookie);
        if (reason && *reason){
            Message *msg = new Message(MessageGeneric);
            msg->setText(QString::fromUtf8(reason));
            msg->setFlags(MESSAGE_NOHISTORY);
            if (!m_client->send(msg, m_data))
                delete msg;
        }
        delete msg;
        return true;
    }
    return false;
}

#ifndef WIN32
#include "msnclient.moc"
#endif
