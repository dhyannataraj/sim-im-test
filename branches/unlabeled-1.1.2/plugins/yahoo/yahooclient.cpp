/***************************************************************************
                          yahooclient.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 * Based on libyahoo2
 *
 * Some code copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * Yahoo Search copyright (C) 2003, Konstantin Klyagin <konst AT konst.org.ua>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *             1998-1999, Adam Fritzler <afritz@marko.net>
 *             1998-2002, Rob Flynn <rob@marko.net>
 *             2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *             2001-2002, Brian Macke <macke@strangelove.net>
 *                  2001, Anand Biligiri S <abiligiri@users.sf.net>
 *                  2001, Valdis Kletnieks
 *                  2002, Sean Egan <bj91704@binghamton.edu>
 *                  2002, Toby Gray <toby.gray@ntlworld.com>
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "yahoo.h"
#include "yahooclient.h"
#include "yahoocfg.h"
#include "yahooinfo.h"

#include <time.h>

static char YAHOO_PACKET_SIGN[] = "YMSG";

const unsigned	YAHOO_LOGIN_OK		= 0;
const unsigned	YAHOO_LOGIN_PASSWD	= 13;
const unsigned	YAHOO_LOGIN_LOCK	= 14;
const unsigned	YAHOO_LOGIN_DUPL	= 99;

static DataDef yahooUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(9) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "Login", DATA_UTF, 1, 0 },
        { "Nick", DATA_UTF, 1, 0 },
        { "First", DATA_UTF, 1, 0 },
        { "Last", DATA_UTF, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, DATA(-1) },	// Status
        { "", DATA_BOOL, 1, 0 },			// bAway
        { "", DATA_UTF, 1, 0 },				// AwayMessage
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "OnlineTime", DATA_ULONG, 1, 0 },
        { "Group", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },			// bChecked
        { "", DATA_BOOL, 1, 0 },			// bTyping
        { NULL, 0, 0, 0 }
    };

static DataDef yahooClientData[] =
    {
        { "Server", DATA_STRING, 1, "scs.msg.yahoo.com" },
        { "Port", DATA_ULONG, 1, DATA(5050) },
        { "", DATA_STRUCT, sizeof(YahooUserData) / sizeof(Data), DATA(yahooUserData) },
        { NULL, 0, 0, 0 }
    };

const DataDef *YahooProtocol::userDataDef()
{
    return yahooUserData;
}

YahooClient::YahooClient(Protocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(yahooClientData, &data, cfg);
    m_status = STATUS_OFFLINE;
}

YahooClient::~YahooClient()
{
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(yahooClientData, &data);
}

string YahooClient::getConfig()
{
    string res = TCPClient::getConfig();
    if (res.length())
        res += "\n";
    res += save_data(yahooClientData, &data);
    return res;
}

bool YahooClient::send(Message*, void*)
{
    return false;
}

bool YahooClient::canSend(unsigned, void*)
{
    return false;
}

void YahooClient::packet_ready()
{
    if (m_bHeader){
        char header[4];
        m_socket->readBuffer.unpack(header, 4);
        if (memcmp(header, YAHOO_PACKET_SIGN, 4)){
            m_socket->error_state("Bad packet sign");
            return;
        }
        m_socket->readBuffer.incReadPos(4);
        m_socket->readBuffer >> m_data_size >> m_service;
        unsigned long session_id;
        m_socket->readBuffer >> m_pkt_status >> session_id;
        log(L_DEBUG, "Status: %X Session: %X Data size: %X Service: %X", m_pkt_status, session_id, m_data_size, m_service);
        if (m_data_size){
            m_socket->readBuffer.add(m_data_size);
            m_bHeader = false;
            return;
        }
    }
    log_packet(m_socket->readBuffer, false, YahooPlugin::YahooPacket);
    process_packet();
    m_socket->readBuffer.init(20);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
}

void YahooClient::sendPacket(unsigned short service, unsigned long status)
{
    unsigned short size = 0;
    if (!m_values.empty()){
        for (list<PARAM>::iterator it = m_values.begin(); it != m_values.end(); ++it){
            size += 4;
            size += (*it).second.size();
            size += number((*it).first).length();
        }
    }
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer.pack(YAHOO_PACKET_SIGN, 4);
    m_socket->writeBuffer << 0x000B0000L << size << service << status << m_session;
    if (size){
        for (list<PARAM>::iterator it = m_values.begin(); it != m_values.end(); ++it){
            m_socket->writeBuffer
            << number((*it).first).c_str()
            << (unsigned short)0xC080
            << (*it).second.c_str()
            << (unsigned short)0xC080;
        }
    }
    m_values.clear();
    log_packet(m_socket->writeBuffer, true, YahooPlugin::YahooPacket);
    m_socket->write();
}

void YahooClient::addParam(unsigned key, const char *value)
{
    if (value == NULL)
        value = "";
    m_values.push_back(PARAM(key, string(value)));
}

void YahooClient::connect_ready()
{
    m_socket->readBuffer.init(20);
    m_socket->readBuffer.packetStart();
    m_session = rand();
    m_bHeader = true;
    log(L_DEBUG, "Connect ready");
    TCPClient::connect_ready();
    sendPacket(YAHOO_SERVICE_VERIFY);
}

class Params : public map<unsigned, string>
{
public:
    Params() {}
    const char *operator[](unsigned id);
};

const char *Params::operator [](unsigned id)
{
    map<unsigned, string>::iterator it = find(id);
    if (it == end())
        return NULL;
    return (*it).second.c_str();
}

void YahooClient::process_packet()
{
    Params params;
    for (;;){
        string key;
        string value;
        if (!m_socket->readBuffer.scan("\xC0\x80", key) || !m_socket->readBuffer.scan("\xC0\x80", value))
            break;
        unsigned key_id = atol(key.c_str());
        map<unsigned, string>::iterator it = params.find(key_id);
        if (it == params.end())
            params.insert(map<unsigned, string>::value_type(key_id, value));
        log(L_DEBUG, "Param: %u %s", key_id, value.c_str());
    }
    switch (m_service){
    case YAHOO_SERVICE_VERIFY:
        if (m_pkt_status != 1){
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Yahoo! login lock"));
            return;
        }
        addParam(1, getLogin().utf8());
        sendPacket(YAHOO_SERVICE_AUTH);
        break;
    case YAHOO_SERVICE_AUTH:
        process_auth(params[13], params[94], params[1]);
        break;
    case YAHOO_SERVICE_AUTHRESP:
        m_pkt_status = 0;
        if (params[66])
            m_pkt_status = atol(params[66]);
        switch (m_pkt_status){
        case YAHOO_LOGIN_OK:
            authOk();
            return;
        case YAHOO_LOGIN_PASSWD:
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Login failed"), AuthError);
            return;
        case YAHOO_LOGIN_LOCK:
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Your account has been locked"), AuthError);
            return;
        case YAHOO_LOGIN_DUPL:
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Your account is being used from another location"));
            return;
        default:
            m_socket->error_state(I18N_NOOP("Login failed"));
        }
        break;
    case YAHOO_SERVICE_LIST:
        authOk();
        loadList(params[87]);
        break;
    case YAHOO_SERVICE_LOGOFF:
        if (m_pkt_status == (unsigned long)(-1)){
            m_reconnect = NO_RECONNECT;
            m_socket->error_state(I18N_NOOP("Your account is being used from another location"));
            return;
        }
    case YAHOO_SERVICE_LOGON:
        if (params[1])
            authOk();
    case YAHOO_SERVICE_USERSTAT:
    case YAHOO_SERVICE_ISAWAY:
    case YAHOO_SERVICE_ISBACK:
    case YAHOO_SERVICE_GAMELOGON:
    case YAHOO_SERVICE_GAMELOGOFF:
    case YAHOO_SERVICE_IDACT:
    case YAHOO_SERVICE_IDDEACT:
        if (params[7] && params[13])
            processStatus(m_service, params[7], params[10], params[19], params[47], params[137]);
        break;
    case YAHOO_SERVICE_IDLE:
    case YAHOO_SERVICE_MAILSTAT:
    case YAHOO_SERVICE_CHATINVITE:
    case YAHOO_SERVICE_CALENDAR:
    case YAHOO_SERVICE_NEWPERSONALMAIL:
    case YAHOO_SERVICE_ADDIDENT:
    case YAHOO_SERVICE_ADDIGNORE:
    case YAHOO_SERVICE_PING:
    case YAHOO_SERVICE_GOTGROUPRENAME:
    case YAHOO_SERVICE_GROUPRENAME:
    case YAHOO_SERVICE_PASSTHROUGH2:
    case YAHOO_SERVICE_CHATLOGON:
    case YAHOO_SERVICE_CHATLOGOFF:
    case YAHOO_SERVICE_CHATMSG:
    case YAHOO_SERVICE_REJECTCONTACT:
    case YAHOO_SERVICE_PEERTOPEER:
        break;
    case YAHOO_SERVICE_MESSAGE:
        if (params[4] && params[14])
            process_message(params[4], params[14], params[97]);
        break;
    case YAHOO_SERVICE_NOTIFY:
        if (params[4] && params[49])
            notify(params[4], params[49], params[13]);
        break;
    case YAHOO_SERVICE_NEWCONTACT:
        if (params[1]){
            contact_added(params[3], params[14]);
            return;
        }
        if (params[7]){
            processStatus(m_service, params[7], params[10], params[14], params[47], params[137]);
            return;
        }
        if (m_pkt_status == 7)
            contact_rejected(params[3], params[14]);
        break;
    default:
        log(L_WARN, "Unknown service %X", m_service);
    }
}

class TextParser
{
public:
    TextParser(bool bUtf);
    QString parse(const char *msg);
protected:
    void addText(const char *str, unsigned size);
    unsigned m_state;
    stack<string> m_tags;
    void setState(unsigned code, bool bSet);
    void clearState(unsigned code);
    void push_tag(const char *tag);
    void pop_tag(const char *tag);
    bool m_bUtf;
    QString m_text;
};

TextParser::TextParser(bool bUtf)
{
    m_bUtf  = bUtf;
    m_state = 0;
}

QString TextParser::parse(const char *msg)
{
    Buffer b;
    b.pack(msg, strlen(msg));
    for (;;){
        string part;
        if (!b.scan("\x1B\x5B", part))
            break;
        addText(part.c_str(), part.length());

        if (!b.scan("m", part))
            break;
        if (part.empty())
            continue;
        if (part[0] == 'x'){
            unsigned code = atol(part.c_str() + 1);
            switch (code){
            case 1:
            case 2:
            case 4:
                setState(code, false);
                break;
            }
        }else{
            unsigned code = atol(part.c_str());
            if (code == 39){
                b.scan(">", part);
                part += ">";
                continue;
            }
            switch (code){
            case 1:
            case 2:
            case 4:
                setState(code, true);
                break;
            }
        }
    }
    addText(b.data(b.readPos()), b.writePos() - b.readPos());
    while (!m_tags.empty()){
        m_text += "</";
        m_text += m_tags.top().c_str();
        m_text += ">";
        m_tags.pop();
    }
    return m_text;
}

void TextParser::setState(unsigned code, bool bSet)
{
    if (bSet){
        if ((m_state & code) == code)
            return;
        m_state |= code;
    }else{
        if ((m_state & code) == 0)
            return;
        m_state &= ~code;
    }
    string tag;
    switch (code){
    case 1:
        tag = "b";
        break;
    case 2:
        tag = "i";
        break;
    case 4:
        tag = "u";
        break;
    default:
        return;
    }
    if (bSet){
        push_tag(tag.c_str());
    }else{
        pop_tag(tag.c_str());
    }
}

void TextParser::push_tag(const char *tag)
{
    m_tags.push(tag);
    m_text += "<";
    m_text += tag;
    m_text += ">";
}

void TextParser::pop_tag(const char *tag)
{
    stack<string> tags;
    while (!m_tags.empty()){
        string top = m_tags.top();
        m_tags.pop();
        if (top == tag)
            break;
        m_text += "</";
        m_text += top.c_str();
        m_text += ">";
        tags.push(top);
    }
    while (!tags.empty()){
        string top = tags.top();
        tags.pop();
        m_text += "<";
        m_text += top.c_str();
        m_text += ">";
    }
}

void TextParser::addText(const char *str, unsigned size)
{
    if (size == 0)
        return;
    if (m_bUtf){
        m_text += quoteString(QString::fromUtf8(str, size));
    }else{
        m_text += quoteString(QString::fromLocal8Bit(str, size));
    }
}

void YahooClient::process_message(const char *id, const char *msg, const char *utf)
{
    bool bUtf = false;
    if (utf && atol(utf))
        bUtf = true;
    Message *m = new Message(MessageGeneric);
    m->setFlags(MESSAGE_RICHTEXT);
    TextParser parser(bUtf);
    m->setText(parser.parse(msg));
    messageReceived(m, id);
}

void YahooClient::notify(const char *id, const char *msg, const char *state)
{
    Contact *contact;
    YahooUserData *data = findContact(id, NULL, contact);
    if (data == NULL)
        return;
    bool bState = false;
    if (state && atol(state))
        bState = true;
    if (!strcasecmp(msg, "TYPING")){
        if (data->bTyping.bValue != bState){
            data->bTyping.bValue = bState;
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
}

void YahooClient::contact_added(const char *id, const char *message)
{
    Message *msg = new AuthMessage(MessageAdded);
    if (message)
        msg->setText(QString::fromUtf8(message));
    messageReceived(msg, id);
}

void YahooClient::contact_rejected(const char *id, const char *message)
{
    Message *msg = new AuthMessage(MessageRemoved);
    if (message)
        msg->setText(QString::fromUtf8(message));
    messageReceived(msg, id);
}

static bool _cmp(const char *s1, const char *s2)
{
    if (s1 == NULL)
        s1 = "";
    if (s2 == NULL)
        s2 = "";
    return strcmp(s1, s2) != 0;
}

void YahooClient::processStatus(unsigned short service, const char *id,
                                const char *_state, const char *_msg,
                                const char *_away, const char *_idle)
{
    Contact *contact;
    YahooUserData *data = findContact(id, NULL, contact);
    if (data == NULL)
        return;
    unsigned state = 0;
    unsigned away  = 0;
    unsigned idle  = 0;
    if (_state)
        state = atol(_state);
    if (_away)
        away  = atol(_away);
    if (_idle)
        idle  = atol(_idle);
    if (service == YAHOO_SERVICE_LOGOFF)
        state = YAHOO_STATUS_OFFLINE;
    if ((state != data->Status.value) ||
            ((state == YAHOO_STATUS_CUSTOM) &&
             (((away != 0) != data->bAway.bValue) || _cmp(_msg, data->AwayMessage.ptr)))){

        unsigned long old_status = STATUS_UNKNOWN;
        unsigned style  = 0;
        const char *statusIcon = NULL;
        contactInfo(data, old_status, style, statusIcon);

        time_t now;
        time(&now);
        now -= idle;
        if (data->Status.value == YAHOO_STATUS_OFFLINE)
            data->OnlineTime.value = now;
        data->Status.value = state;
        data->bAway.bValue = (away != 0);
        data->StatusTime.value = now;

        unsigned long new_status = STATUS_UNKNOWN;
        contactInfo(data, old_status, style, statusIcon);

        if (old_status != new_status){
            StatusMessage m;
            m.setContact(contact->id());
            m.setClient(dataName(data).c_str());
            m.setFlags(MESSAGE_RECEIVED);
            m.setStatus(STATUS_OFFLINE);
            Event e(EventMessageReceived, &m);
            e.process();
            if ((new_status == STATUS_ONLINE) && !contact->getIgnore()){
                Event e(EventContactOnline, contact);
                e.process();
            }
        }else{
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
}

string YahooClient::name()
{
    string res = "Yahoo.";
    if (data.owner.Login.ptr)
        res += data.owner.Login.ptr;
    return res;
}

string YahooClient::dataName(void *_data)
{
    string res = name();
    YahooUserData *data = (YahooUserData*)_data;
    res += "+";
    res += data->Login.ptr;
    return res;
}

void YahooClient::setStatus(unsigned status)
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
        }
        return;
    }
    m_status = status;
}

void YahooClient::disconnected()
{
    m_values.clear();
}

bool YahooClient::isMyData(clientData *&_data, Contact*&)
{
    if (_data->Sign.value != YAHOO_SIGN)
        return false;
    return true;
}

bool YahooClient::createData(clientData *&_data, Contact *contact)
{
    YahooUserData *data = (YahooUserData*)_data;
    YahooUserData *new_data = (YahooUserData*)(contact->clientData.createData(this));
    set_str(&new_data->Nick.ptr, data->Nick.ptr);
    _data = (clientData*)new_data;
    return true;
}

void YahooClient::setupContact(Contact*, void*)
{
}

QWidget	*YahooClient::setupWnd()
{
    return new YahooConfig(NULL, this, false);
}

QString YahooClient::getLogin()
{
    if (data.owner.Login.ptr)
        return QString::fromUtf8(data.owner.Login.ptr);
    return "";
}

void YahooClient::setLogin(const QString &login)
{
    set_str(&data.owner.Login.ptr, login.utf8());
}

void YahooClient::authOk()
{
    if (getState() == Connected)
        return;
    setState(Connected);
    setPreviousPassword(NULL);
    setStatus(m_logonStatus);
}

void YahooClient::loadList(const char *str)
{
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData);
        while ((data = (YahooUserData*)(++itd)) != NULL){
            data->bChecked.bValue = false;
        }
    }
    if (str){
        string s = str;
        while (!s.empty()){
            string line = getToken(s, '\n');
            string grp = getToken(line, ':');
            if (line.empty()){
                line = grp;
                grp = "";
            }
            while (!line.empty()){
                string id = getToken(line, ',');
                Contact *contact;
                YahooUserData *data = findContact(id.c_str(), grp.c_str(), contact);
                data->bChecked.bValue = true;
            }
        }
    }
    it.reset();
    list<Contact*> forRemove;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        list<YahooUserData*> dataForRemove;
        bool bChanged = false;
        while ((data = (YahooUserData*)(++itd)) != NULL){
            if (!data->bChecked.bValue){
                dataForRemove.push_back(data);
                bChanged = true;
            }
        }
        if (!bChanged)
            continue;
        for (list<YahooUserData*>::iterator it = dataForRemove.begin(); it != dataForRemove.end(); ++it)
            contact->clientData.freeData(*it);
        if (contact->clientData.size()){
            Event e(EventContactChanged, contact);
            e.process();
        }else{
            forRemove.push_back(contact);
        }
    }
    for (list<Contact*>::iterator itr = forRemove.begin(); itr != forRemove.end(); ++itr)
        delete *itr;
}

YahooUserData *YahooClient::findContact(const char *id, const char *grpname, Contact *&contact)
{
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        YahooUserData *data;
        ClientDataIterator itd(contact->clientData);
        while ((data = (YahooUserData*)(++itd)) != NULL){
            if (!strcmp(id, data->Login.ptr))
                return data;
        }
    }
    it.reset();
    while ((contact = ++it) != NULL){
        if (contact->getName() == id){
            YahooUserData *data = (YahooUserData*)contact->clientData.createData(this);
            set_str(&data->Login.ptr, id);
            set_str(&data->Group.ptr, grpname);
            Event e(EventContactChanged, contact);
            e.process();
            return data;
        }
    }
    Group *grp = NULL;
    if (grpname && *grpname){
        ContactList::GroupIterator it;
        while ((grp = ++it) != NULL)
            if (grp->getName() == QString::fromLocal8Bit(grpname))
                break;
        if (grp == NULL){
            grp = getContacts()->group(0, true);
            grp->setName(QString::fromLocal8Bit(grpname));
            Event e(EventGroupChanged, grp);
            e.process();
        }
    }
    if (grp == NULL)
        grp = getContacts()->group(0);
    contact = getContacts()->contact(0, true);
    YahooUserData *data = (YahooUserData*)(contact->clientData.createData(this));
    set_str(&data->Login.ptr, id);
    contact->setName(id);
    contact->setGroup(grp->id());
    Event e(EventContactChanged, contact);
    e.process();
    return data;
}

void YahooClient::messageReceived(Message *msg, const char *id)
{
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    if (msg->contact() == 0){
        Contact *contact;
        YahooUserData *data = findContact(id, NULL, contact);
        if (data == NULL){
            data = findContact(id, "", contact);
            if (data == NULL){
                delete msg;
                return;
            }
            contact->setTemporary(CONTACT_TEMP);
            Event e(EventContactChanged, contact);
            e.process();
        }
        msg->setClient(dataName(data).c_str());
        msg->setContact(contact->id());
    }
    Event e(EventMessageReceived, msg);
    e.process();
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

void YahooClient::contactInfo(void *_data, unsigned long &status, unsigned&, const char *&statusIcon, string *icons)
{
    YahooUserData *data = (YahooUserData*)_data;
    unsigned cmp_status = STATUS_OFFLINE;
    switch (data->Status.value){
    case YAHOO_STATUS_AVAILABLE:
        cmp_status = STATUS_ONLINE;
        break;
    case YAHOO_STATUS_BUSY:
        cmp_status = STATUS_DND;
        break;
    case YAHOO_STATUS_NOTATHOME:
    case YAHOO_STATUS_NOTATDESK:
    case YAHOO_STATUS_NOTINOFFICE:
    case YAHOO_STATUS_ONVACATION:
        cmp_status = STATUS_NA;
        break;
    case YAHOO_STATUS_OFFLINE:
        break;
    case YAHOO_STATUS_CUSTOM:
        cmp_status = data->bAway.bValue ? STATUS_AWAY : STATUS_ONLINE;
        break;
    default:
        cmp_status = STATUS_AWAY;
    }

    const CommandDef *def;
    for (def = protocol()->statusList(); def->text; def++){
        if (def->id == cmp_status)
            break;
    }
    if (cmp_status > status){
        status = cmp_status;
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
    if (icons && data->bTyping.bValue)
        addIcon(icons, "typing", statusIcon);
}

QString YahooClient::contactTip(void *_data)
{
    YahooUserData *data = (YahooUserData*)_data;
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
    res += QString::fromUtf8(data->Login.ptr);
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
    return res;
}

const unsigned MAIN_INFO = 1;
const unsigned NETWORK	 = 2;

static CommandDef yahooWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "Yahoo!_online",
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

static CommandDef cfgYahooWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "Yahoo!_online",
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

CommandDef *YahooClient::infoWindows(Contact*, void *_data)
{
    YahooUserData *data = (YahooUserData*)_data;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data->Login.ptr);
    yahooWnd[0].text_wrk = strdup(name.utf8());
    return yahooWnd;
}

CommandDef *YahooClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data.owner.Login.ptr);
    cfgYahooWnd[0].text_wrk = strdup(name.utf8());
    return cfgYahooWnd;
}

QWidget *YahooClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    YahooUserData *data = (YahooUserData*)_data;
    switch (id){
    case MAIN_INFO:
        return new YahooInfo(parent, data, this);
    }
    return NULL;
}

QWidget *YahooClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new YahooInfo(parent, NULL, this);
    case NETWORK:
        return new YahooConfig(parent, this, true);
    }
    return NULL;
}

#ifndef WIN32
#include "yahooclient.moc"
#endif




