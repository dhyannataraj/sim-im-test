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
#include "simapi.h"

#include <time.h>
#include <qtimer.h>
#include <qregexp.h>

#include <map>
#include <algorithm>
using namespace std;

unsigned long PING_TIMEOUT	= 60;
unsigned long TYPING_TIME   = 10;

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
        { "", DATA_ULONG, 1, MSN_SIGN },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "EMail", DATA_UTF, 1, 0 },
        { "Screen", DATA_UTF, 1, 0 },
        { "", DATA_ULONG, 1, STATUS_OFFLINE },	// Status
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
        { "Server", DATA_STRING, 1, (unsigned)"messenger.hotmail.com" },
        { "Port", DATA_ULONG, 1, 1863 },
        { "ListVer", DATA_ULONG, 1, 0 },
        { "ListRequests", DATA_UTF, 1, 0 },
        { "Version", DATA_STRING, 1, (unsigned)"5.0.0540" },
        { "", DATA_STRUCT, sizeof(MSNUserData) / sizeof(unsigned), (unsigned)msnUserData },
        { NULL, 0, 0, 0 }
    };

MSNClient::MSNClient(Protocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(msnClientData, &data, cfg);
    m_packetId  = 1;
    m_msg = NULL;
    QString s = getListRequests();
    while (!s.isEmpty()){
        QString item = getToken(s, ';');
        MSNListRequest lr;
        lr.Type = getToken(item, ',').toUInt();
        lr.Name = item.utf8();
    }
    setListRequests("");
}

MSNClient::~MSNClient()
{
    TCPClient::setStatus(STATUS_OFFLINE, false);
    free_data(msnClientData, &data);
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
        data.owner.OnlineTime = now;
    data.owner.StatusTime = now;
    m_status = status;
    data.owner.Status = m_status;
    Event e(EventClientChanged, static_cast<Client*>(this));
    e.process();
    if (status == STATUS_OFFLINE){
        if (m_status != STATUS_OFFLINE){
            m_status = status;
            data.owner.Status = status;
            time_t now;
            time(&now);
            data.owner.StatusTime = now;
            MSNPacket *packet = new OutPacket(this);
            packet->send();
        }
        return;
    }
    m_status = status;
    MSNPacket *packet = new ChgPacket(this);
    packet->send();
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
    Contact *contact;
    ContactList::ContactIterator it;
    time_t now;
    time(&now);
    while ((contact = ++it) != NULL){
        bool bChanged = false;
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            if (data->Status != STATUS_OFFLINE){
                data->Status = STATUS_OFFLINE;
                data->StatusTime = now;
                if (data->sb){
                    delete data->sb;
                    data->sb = NULL;
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
    m_fetchId  = 0;
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

void MSNClient::getLine(const char *line)
{
    QString l = QString::fromUtf8(line);
    l = l.replace(QRegExp("\r"), "");
    QCString ll = l.local8Bit();
    log(L_DEBUG, "Get: %s", (const char*)ll);
    QString cmd = getToken(l, ' ');
    if (cmd == "XFR"){
        QString id   = getToken(l, ' ');	// ID
        QString type = getToken(l, ' ');	// NS
        if (type == "NS"){
            l = getToken(l, ' ');				// from
            QString host = getToken(l, ':');
            unsigned port = l.toUInt();
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
        unsigned id = getToken(l, ' ').toUInt();
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
        if (data == NULL){
            log(L_WARN, "Contact %u not found", id);
            return;
        }
        cmd = getToken(l, ' ').latin1();
        bool bChanged = false;
        if (cmd == "PHH")
            bChanged |= set_str(&data->PhoneHome, unquote(getToken(l, ' ')).utf8());
        if (cmd == "PHW")
            bChanged |= set_str(&data->PhoneHome, unquote(getToken(l, ' ')).utf8());
        if (cmd == "PHM")
            bChanged |= set_str(&data->PhoneHome, unquote(getToken(l, ' ')).utf8());
        if (cmd == "MOB")
            data->Mobile = (getToken(l, ' ') == "Y");
        if (bChanged)
            setupContact(contact, data);
        return;
    }
    if (cmd == "ILN"){
        getToken(l, ' ');
        unsigned status = str2status(getToken(l, ' '));
        Contact *contact;
        MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
        if (data && (data->Status != status)){
            time_t now;
            time(&now);
            data->Status = status;
            data->OnlineTime = now;
            data->StatusTime = now;
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
        if (data && (data->Status != status)){
            time_t now;
            time(&now);
            if (data->Status == STATUS_OFFLINE)
                data->OnlineTime = now;
            data->StatusTime = now;
            data->Status = status;
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
        if (data && (data->Status != STATUS_OFFLINE)){
            time_t now;
            time(&now);
            data->StatusTime = now;
            data->Status = STATUS_OFFLINE;
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
            MSNUserData *data = findContact(getToken(l, ' ').utf8(), contact);
            if (data){
                data->Flags |= MSN_REVERSE;
                auth_message(contact, MessageAdded, data);
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
                data->Flags &= ~MSN_REVERSE;
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
        if (data->sb){
            delete data->sb;
            data->sb = NULL;
        }
        data->sb = new SBSocket(this, contact, data);
        data->sb->connect(addr.c_str(), session.c_str(), cookie.c_str(), false);
        return;
    }
    if (cmd == "OUT"){
        m_reconnect = NO_RECONNECT;
        m_socket->error_state(I18N_NOOP("Youur account is being used from another location"));
        return;
    }
    unsigned code = cmd.toUInt();
    if (code){
        if (!m_packets.empty()){
            MSNPacket *packet = m_packets.front();
            m_packets.erase(m_packets.begin());
            packet->error(code);
            delete packet;
        }
        return;
    }

    if (m_packets.empty()){
        log(L_DEBUG, "Packet not found");
        return;
    }
    MSNPacket *packet = m_packets.front();
    if (packet->m_bAnswer){
        QString save_l = l;
        vector<string> args;
        while (l.length())
            args.push_back(string(getToken(l, ' ', false).utf8()));
        if (packet->answer(cmd.latin1(), args))
            return;
        l = save_l;
        m_packets.erase(m_packets.begin());
        delete packet;
        if (m_packets.empty()){
            log(L_DEBUG, "Packet not found");
            return;
        }
        packet = m_packets.front();
    }

    unsigned id = getToken(l, ' ').toUInt();
    if (packet->id() != id){
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
    if (packet->answer(cmd.latin1(), args)){
        packet->m_bAnswer = true;
        return;
    }
    m_packets.erase(m_packets.begin());
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
    setStatus(m_logonStatus);
    QTimer::singleShot(TYPING_TIME * 1000, this, SLOT(ping()));
    setState(Connected);

    setPreviousPassword(NULL);
    MSNPacket *packet = new SynPacket(this);
    packet->send();
    setStatus(m_logonStatus);
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
    if (data.owner.EMail == NULL)
        return "";
    return QString::fromUtf8(data.owner.EMail);
}

void MSNClient::setLogin(const QString &str)
{
    set_str(&data.owner.EMail, str.utf8());
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
    name += QString::fromUtf8(data->EMail);
    msnWnd[0].text_wrk = strdup(name.utf8());
    return msnWnd;
}

CommandDef *MSNClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::fromUtf8(data.owner.EMail);
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
    if ((_data == NULL) || (getState() != Connected))
        return false;
    MSNUserData *data = (MSNUserData*)_data;
    switch (type){
    case MessageGeneric:
        return true;
    }
    return false;
}

bool MSNClient::send(Message *msg, void *_data)
{
    if ((_data == NULL) || (getState() != Connected))
        return false;
    MSNUserData *data = (MSNUserData*)_data;
    MSNPacket *packet = NULL;
    switch (msg->type()){
    case MessageGeneric:
        if (data->sb == NULL){
            Contact *contact;
            findContact(data->EMail, contact);
            data->sb = new SBSocket(this, contact, data);
        }
        return data->sb->send(msg);
    }
    if (packet == NULL)
        return false;
    packet->send();
    return true;
}

string MSNClient::dataName(void *_data)
{
    string res = name();
    MSNUserData *data = (MSNUserData*)_data;
    res += "+";
    res += data->EMail;
    return res;
}

bool MSNClient::isMyData(clientData *_data, Contact *&contact)
{
    if (_data->Sign != MSN_SIGN)
        return false;
    MSNUserData *data = (MSNUserData*)_data;
    if (findContact(data->EMail, contact) == NULL)
        contact = NULL;
    return true;
}

void MSNClient::setupContact(Contact *contact, void *_data)
{
    MSNUserData *data = (MSNUserData*)_data;
    QString phones;
    if (data->PhoneHome){
        phones += QString::fromUtf8(data->PhoneHome);
        phones += ",Home Phone,1";
    }
    if (data->PhoneWork){
        if (!phones.isEmpty())
            phones += ";";
        phones += QString::fromUtf8(data->PhoneWork);
        phones += ",Work Phone,1";
    }
    if (data->PhoneMobile){
        if (!phones.isEmpty())
            phones += ";";
        phones += QString::fromUtf8(data->PhoneMobile);
        phones += ",Private Cellular,3";
    }
    bool bChanged = contact->setPhones(phones, name().c_str());
    bChanged |= contact->setEMails(data->EMail, name().c_str());
	if (contact->getName().isEmpty()){
		QString name = QString::fromUtf8(data->ScreenName);
		if (name.isEmpty())
			name = QString::fromUtf8(data->EMail);
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

static char fromHex(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    return 0;
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
        char cc = 0;
        cc = fromHex(s[i++]) << 4;
        cc += fromHex(s[i]);
        res += QChar(cc);
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
            if (!strcmp(res->EMail, mail))
                return res;
        }
    }
    return NULL;
}

QString MSNClient::contactName(void *clientData)
{
    MSNUserData *data = (MSNUserData*)clientData;
    QString res = "MSN: ";
    res += QString::fromUtf8(data->EMail);
    return res;
}

MSNUserData *MSNClient::findContact(const char *mail, const char *name, Contact *&contact)
{
    QString name_str = unquote(QString::fromUtf8(name));
    MSNUserData *data = findContact(mail, contact);
    if (data){
        set_str(&data->ScreenName, name);
		setupContact(contact, data);
        return data;
    }
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        if (contact->getName() == name_str){
            data = (MSNUserData*)(contact->clientData.createData(this));
            set_str(&data->EMail, mail);
            set_str(&data->ScreenName, name);
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
            set_str(&data->EMail, mail);
            set_str(&data->ScreenName, name);
			setupContact(contact, data);
            Event e(EventContactChanged, contact);
            e.process();
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
                set_str(&data->EMail, mail);
                set_str(&data->ScreenName, name);
				setupContact(contact, data);
                Event e(EventContactChanged, contact);
                e.process();
                return data;
            }
        }
    }
    contact = getContacts()->contact(0, true);
    data = (MSNUserData*)(contact->clientData.createData(this));
    set_str(&data->EMail, mail);
    set_str(&data->ScreenName, name);
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
        if ((res == NULL) || (res->Group != id))
            continue;
        if ((name != NULL) && set_str(&res->ScreenName, name)){
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
        res->Group = id;
        set_str(&res->ScreenName, name);
        return res;
    }
    grp = getContacts()->group(0, this);
    MSNUserData *res = (MSNUserData*)(grp->clientData.createData(this));
    res->Group = id;
    set_str(&res->ScreenName, name);
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

void *MSNClient::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        MSNUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (MSNUserData*)(++it)) != NULL){
            bool bChanged = false;
            if (contact->getIgnore() != ((data->Flags & MSN_BLOCKED) != 0))
                bChanged = true;
            if (contact->getGroup() != (data->Group))
                bChanged = true;
            if (contact->getName() != QString::fromUtf8(data->ScreenName))
                bChanged = true;
            if (!bChanged)
                continue;
            findRequest(data->EMail, LR_CONTACTxCHANGED, true);
            MSNListRequest lr;
            lr.Type = LR_CONTACTxCHANGED;
            lr.Name = data->EMail;
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
            findRequest(data->EMail, LR_CONTACTxCHANGED, true);
            MSNListRequest lr;
            lr.Type = LR_CONTACTxREMOVED;
            lr.Name = data->EMail;
            m_requests.push_back(lr);
            if (data->Flags & MSN_BLOCKED){
                lr.Type = LR_CONTACTxREMOVED_BL;
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
        if ((data == NULL) || (grp->getName() != QString::fromUtf8(data->ScreenName))){
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
            lr.Name = number(data->Group);
            m_requests.push_back(lr);
            processRequests();
        }
    }
    if ((e->type() == EventStartTyping) || (e->type() == EventStopTyping)){
        if (getState() != Connected)
            return NULL;
        Contact *contact = getContacts()->contact((unsigned long)(e->param()));
        if (contact == NULL)
            return NULL;
        ClientDataIterator it(contact->clientData, this);
        MSNUserData *data = (MSNUserData*)(++it);
        if (data == NULL)
            return NULL;
        if (data->sb){
            data->sb->setTyping(e->type() == EventStartTyping);
            return NULL;
        }
        if (e->type() == EventStartTyping){
            data->sb = new SBSocket(this, contact, data);
            data->sb->connect();
            data->sb->setTyping(true);
        }
    }
    if (e->type() == EventMessageCancel){
        Message *msg = (Message*)(e->param());
        for (list<SBSocket*>::iterator it = m_SBsockets.begin(); it != m_SBsockets.end(); ++it){
            if ((*it)->cancelMessage(msg))
                return msg;
        }
    }
    if (e->type() == EventFetchDone){
        fetchData *data = (fetchData*)e->param();
        if (data->req_id != m_fetchId)
            return NULL;
        m_fetchId = 0;
        string h;
        switch (m_state){
        case LoginHost:
            h = getHeader("Location", data->headers);
            if (!h.empty()){
                requestLoginHost(h.c_str());
                break;
            }
            if (data->result == 200){
                h = getHeader("PassportURLs", data->headers);
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
            h = getHeader("Location", data->headers);
            if (!h.empty()){
                requestTWN(h.c_str());
                break;
            }
            if (data->result == 200){
                h = getHeader("Authentication-Info", data->headers);
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
            }else{
                m_socket->error_state("Bad answer code");
            }
            break;
        default:
            log(L_WARN, "Fetch done in bad state");
        }
        return e->param();
    }
    return NULL;
}

void MSNClient::requestLoginHost(const char *url)
{
    m_fetchId = fetch(this, url);
    if (m_fetchId == 0){
        authFailed();
        return;
    }
    m_state = LoginHost;
}

void MSNClient::requestTWN(const char *url)
{
    string auth = "Authorization: Passport1.4 OrgVerb=GET,OrgURL=http%%3A%%2F%%2Fmessenger%%2Emsn%%2Ecom,sign-in=";
    auth += quote(getLogin()).utf8();
    auth += ",pwd=";
    auth += quote(getPassword()).utf8();
    auth += ",";
    auth += m_authChallenge;
    auth += '\x00';
    auth += '\x00';
    m_state = TWN;
    m_fetchId = fetch(this, url, NULL, auth.c_str());
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
                bool bBlock = (data->Flags & MSN_BLOCKED) != 0;
                if (contact->getIgnore() != bBlock){
                    if (contact->getIgnore()){
                        packet = new AddPacket(this, "BL", data->EMail, quote(contact->getName()).utf8());
                        set_str(&data->ScreenName, contact->getName().utf8());
                    }else{
                        packet = new RemPacket(this, "BL", data->EMail);
                    }
                }
                if ((data->Flags & MSN_FORWARD) == 0){
                    packet = new AddPacket(this, "FL", data->EMail, quote(QString::fromUtf8(data->ScreenName)).utf8(), contact->getGroup());
                    data->Group = contact->getGroup();
                }
                Group *group = NULL;
                MSNUserData *grp_data = findGroup(data->Group, NULL, group);
                if (group && (contact->getGroup() != group->id())){
                    packet = new AddPacket(this, "FL", data->EMail, quote(QString::fromUtf8(data->ScreenName)).utf8(), contact->getGroup());
                    packet->send();
                    packet = new RemPacket(this, "FL", data->EMail, data->Group);
                    data->Group = grp_data->Group;
                }
                if (contact->getName() != QString::fromUtf8(data->ScreenName)){
                    packet = new ReaPacket(this, data->EMail, quote(contact->getName()).utf8());
                    set_str(&data->ScreenName, contact->getName().utf8());
                }
            }
            break;
        case LR_CONTACTxREMOVED:
            packet = new RemPacket(this, "FL", (*it).Name.c_str());
            break;
        case LR_CONTACTxREMOVED_BL:
            packet = new RemPacket(this, "BL", (*it).Name.c_str());
            break;
        case LR_GROUPxCHANGED:
            grp = getContacts()->group(atol((*it).Name.c_str()));
            if (grp){
                ClientDataIterator it(grp->clientData, this);
                data = (MSNUserData*)(++it);
                if (data){
                    packet = new RegPacket(this, data->Group, quote(grp->getName()).utf8());
                }else{
                    packet = new AdgPacket(this, grp->id(), quote(grp->getName()).utf8());
                    data = (MSNUserData*)(grp->clientData.createData(this));
                }
                set_str(&data->ScreenName, grp->getName().utf8());
            }
            break;
        case LR_GROUPxREMOVED:
            packet = new RmgPacket(this, atol((*it).Name.c_str()));
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
    return strcmp(((MSNUserData*)d1)->EMail, ((MSNUserData*)d2)->EMail) == 0;
}

static void addIcon(string *s, const char *icon)
{
    if (s == NULL)
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

void MSNClient::contactInfo(void *_data, unsigned long &status, unsigned&, const char *&statusIcon, string *icons)
{
    MSNUserData *data = (MSNUserData*)_data;
    unsigned cmp_status = data->Status;
    const CommandDef *def;
    for (def = protocol()->statusList(); def->text; def++){
        if (def->id == cmp_status)
            break;
    }
    if ((cmp_status == STATUS_BRB) || (cmp_status == STATUS_PHONE) || (cmp_status == STATUS_LUNCH))
        cmp_status = STATUS_AWAY;
    if ((cmp_status > status) || (statusIcon == NULL)){
        status = cmp_status;
        if (icons && statusIcon){
            if (!icons->empty())
                *icons += ",";
            *icons += statusIcon;
        }
        statusIcon = def->icon;
    }else{
        if (icons){
            if (!icons->empty())
                *icons += ",";
            *icons += def->icon;
        }
    }
    if (icons && data->typing_time){
        addIcon(icons, "typing");
    }
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
    res += QString::fromUtf8(data->EMail);
    res += "</b>";
    if (data->Status == STATUS_OFFLINE){
        if (data->StatusTime){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatTime(data->StatusTime);
        }
    }else{
        if (data->OnlineTime){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatTime(data->OnlineTime);
        }
        if (data->Status != STATUS_ONLINE){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatTime(data->StatusTime);
        }
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
    m_socket	= new ClientSocket(this);
    m_packet_id = 0;
    m_messageSize = 0;
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
        m_data->sb = NULL;
        if (m_data->typing_time){
            m_data->typing_time = 0;
            Event e(EventContactStatus, m_contact);
            e.process();
        }
    }
    for (list<Message*>::iterator itm = m_queue.begin(); itm != m_queue.end(); ++itm){
        Message *msg = (*itm);
        msg->setError(I18N_NOOP("Contact go offline"));
        Event e(EventMessageSent, msg);
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
    unsigned port = 0;
    int n = ip.find(':');
    if (n > 0){
        port = atol(ip.substr(n + 1).c_str());
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
    string args = m_client->data.owner.EMail;
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
        send("CAL", m_data->EMail);
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
            Event e(EventMessageSent, msg);
            e.process();
            delete msg;
            m_queue.erase(m_queue.begin());
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

static unsigned fromHex(const char *p)
{
    unsigned res = 0;
    for (; *p; p++){
        char r = 0;
        char c = *p;
        if ((c >= '0') && (c <= '9')){
            r = c - '0';
        }else if ((c >= 'A') && (c <= 'F')){
            r = c - 'A' + 10;
        }else if ((c >= 'a') && (c <= 'f')){
            r = c - 'a' + 10;
        }
        res = (res << 4) + r;
    }
    return res;
}

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
            if (it != v.end()){
                color = fromHex((*it).second.c_str());
                bColor = true;
            }
            continue;
        }
        if (key == "TypingUser"){
            typing = trim(line.c_str());
            continue;
        }
    }
    if (content_type == "text/plain"){
        if (m_data->typing_time){
            m_data->typing_time = 0;
            Event e(EventContactStatus, m_contact);
            e.process();
        }
        QString msg_text = QString::fromUtf8(m_message.c_str());
        msg_text = msg_text.replace(QRegExp("\\r"), "");
        Message msg(MessageGeneric);
        msg.setFlags(MESSAGE_RECEIVED);
        if (bColor){
            msg.setBackground(0xFFFFFF);
            msg.setForeground(color);
        }
        msg.setFont(font.c_str());
        msg.setText(msg_text);
        msg.setContact(m_contact->id());
        msg.setClient(m_client->dataName(m_data).c_str());
        Event e(EventMessageReceived, &msg);
        e.process();
        return;
    }
    if (content_type == "text/x-msmsgscontrol"){
        if (typing == m_data->EMail){
            time_t now;
            time(&now);
            bool bEvent = (m_data->typing_time == 0);
            m_data->typing_time = now;
            if (bEvent){
                Event e(EventContactStatus, m_contact);
                e.process();
            }
        }
    }
}

void SBSocket::timer(unsigned now)
{
    if (m_data->typing_time){
        if (now >= m_data->typing_time + TYPING_TIME){
            m_data->typing_time = 0;
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
        message += m_client->data.owner.EMail;
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

void SBSocket::process(bool bTyping)
{
    if (bTyping)
        sendTyping();
    if (m_msgText.isEmpty() && !m_queue.empty()){
        Message *msg = m_queue.front();
        m_msgText = msg->getPlainText();
        if (m_msgText.isEmpty()){
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

#ifndef WIN32
#include "msnclient.moc"
#endif
