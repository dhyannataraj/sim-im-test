/***************************************************************************
                          icqstatus.cpp  -  description
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

#include "icq.h"
#include "icqconfig.h"
#include "icqinfo.h"
#include "homeinfo.h"
#include "workinfo.h"
#include "moreinfo.h"
#include "aboutinfo.h"
#include "interestsinfo.h"
#include "pastinfo.h"
#include "icqpicture.h"
#include "icqsearch.h"
#include "icqsecure.h"
#include "icqmessage.h"
#include "core.h"

#include "simapi.h"
#include "buffer.h"
#include "socket.h"

#include <qtimer.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qimage.h>
#include <qpixmap.h>

#include <time.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

/*
typedef struct ICQUserData
{
    unsigned long	Status;
    unsigned long	StatusTime;
    unsigned long	OnlineTime;
    void			*IP;
    void			*RealIP;
    unsigned long	Port;
    unsigned long	DCcookie;
    unsigned long	Caps;
    char			*AutoReply;
    unsigned long	Uin;
    unsigned long	IcqID;
    unsigned long	bChecked;
    unsigned long	GrpId;
    unsigned long	IgnoreId;
    unsigned long	VisibleId;
    unsigned long	InvisibleId;
    unsigned long	WaitAuth;
    unsigned long	PhoneState;
    unsigned long	PhoneBookTime;
    unsigned long	PhoneStatusTime;
    unsigned long	InfoUpdateTime;
    unsigned long	Mode;
    unsigned long	Version;
    unsigned long	Build;
    char			*Nick;
    char			*FirstName;
    char			*LastName;
    char			*EMail;
	unsigned long	HiddenEmail;
    char			*City;
    char			*State;
    char			*HomePhone;
    char			*HomeFax;
    char			*Address;
    char			*PrivateCellular;
    char			*Zip;
	unsigned long	Country;
	unsigned long	TimeZone;
    unsigned long	Age;
    unsigned long	Gender;
    char			*Homepage;
    unsigned long	BirthYear;
    unsigned long	BirthMonth;
    unsigned long	BirthDay;
    unsigned long	Language;
	char			*EMails;
    char			*WorkCity;
    char			*WorkState;
    char			*WorkPhone;
    char			*WorkFax;
    char			*WorkAddress;
    char			*WorkZip;;
    unsigned long	WorkCountry;
    char			*WorkName;
    char			*WorkDepartment;
    char			*WorkPosition;
    unsigned long	Occupation;
    char			*WorkHomepage;
	char			*About;
	char			*Interests;
	char			*Backgrounds;
	char			*Affilations;
	unsigned long	InfoFetchTime;
} ICQUserData;
*/
static DataDef _icqUserData[] =
    {
        { "", DATA_UTF, 1, 0 },					// Alias
        { "", DATA_UTF, 1, 0 },					// Cellular
        { "", DATA_ULONG, 1, ICQ_STATUS_OFFLINE },		// Status
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// OnlineTime
        { "IP", DATA_IP, 1, 0 },
        { "RealIP", DATA_IP, 1, 0 },
        { "Port", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// DCcookie
        { "Caps", DATA_ULONG, 1, 0 },
        { "", DATA_STRING, 1, 0 },				// AutoReply
        { "Uin", DATA_ULONG, 1, 0 },
        { "ID", DATA_ULONG, 1, 0 },
        { "", DATA_BOOL, 1, 1 },				// bChecked
        { "GroupID", DATA_ULONG, 1, 0 },
        { "Ignore", DATA_ULONG, 1, 0 },
        { "Visible", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// ContactVisibleId
        { "Invsible", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// ContactInvisibleId
        { "WaitAuth", DATA_BOOL, 1, 0 },
        { "WantAuth", DATA_BOOL, 1, 0 },
        { "WebAware", DATA_BOOL, 1, 1 },
        { "InfoUpdateTime", DATA_ULONG, 1, 0 },
        { "PluginInfoTime", DATA_ULONG, 1, 0 },
        { "PluginStatusTime", DATA_ULONG, 1, 0 },
        { "InfoFetchTime", DATA_ULONG, 1, 0 },
        { "PluginInfoFetchTime", DATA_ULONG, 1, 0 },
        { "PluginStatusFetchTime", DATA_ULONG, 1, 0 },
        { "Mode", DATA_ULONG, 1, 0 },
        { "Version", DATA_ULONG, 1, 0 },
        { "Build", DATA_ULONG, 1, 0 },
        { "Nick", DATA_STRING, 1, 0 },
        { "FirstName", DATA_STRING, 1, 0 },
        { "LastName", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "City", DATA_STRING, 1, 0 },
        { "State", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "Address", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "Zip", DATA_STRING, 1, 0 },
        { "Country", DATA_ULONG, 1, 0 },
        { "TimeZone", DATA_ULONG, 1, 0 },
        { "Age", DATA_ULONG, 1, 0 },
        { "Gender", DATA_ULONG, 1, 0 },
        { "Homepage", DATA_STRING, 1, 0 },
        { "BirthYear", DATA_ULONG, 1, 0 },
        { "BirthMonth", DATA_ULONG, 1, 0 },
        { "BirthDay", DATA_ULONG, 1, 0 },
        { "Language", DATA_ULONG, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "WorkCity", DATA_STRING, 1, 0 },
        { "WorkState", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "WorkAddress", DATA_STRING, 1, 0 },
        { "WorkZip", DATA_STRING, 1, 0 },
        { "WorkCountry", DATA_ULONG, 1, 0 },
        { "WorkName", DATA_STRING, 1, 0 },
        { "WorkDepartment", DATA_STRING, 1, 0 },
        { "WorkPosition", DATA_STRING, 1, 0 },
        { "Occupation", DATA_ULONG, 1, 0 },
        { "WorkHomepage", DATA_STRING, 1, 0 },
        { "About", DATA_STRING, 1, 0 },
        { "Interests", DATA_STRING, 1, 0 },
        { "Backgrounds", DATA_STRING, 1, 0 },
        { "Affilations", DATA_STRING, 1, 0 },
        { "FollowMe", DATA_ULONG, 1, 0 },
        { "SharedFiles", DATA_BOOL, 1, 0 },		// Shared files
        { "ICQPhone", DATA_ULONG, 1, 0 },		// ICQPhone
        { "Encoding", DATA_STRING, 1, 0 },
        { "Picture", DATA_UTF, 1, 0 },
        { "PictureWidth", DATA_ULONG, 1, 0 },
        { "PictureHeight", DATA_ULONG, 1, 0 },
        { "PhoneBook", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },				// bTyping
        { "", DATA_BOOL, 1, 0 },				// bBadClient
        { "", DATA_SOCKET, 1, 0 },				// Direct
        { NULL, 0, 0, 0 }
    };

const DataDef *ICQProtocol::icqUserData = _icqUserData;

/*
typedef struct ICQClientData
{
    unsigned	ContactsTime;
    unsigned	ContactsLength;
    unsigned	ContactsInvisible;
    unsigned	WebAware;
    unsigned	HideIP;
	unsigned	IgnoreAuth;
	unsigned	DirectMode;
	char		*ListRequests;
	ICQUserData	owner;
} ICQClientData;
*/

static DataDef icqClientData[] =
    {
        { "Server", DATA_STRING, 1, (unsigned)"login.icq.com" },
        { "Port", DATA_ULONG, 1, 5190 },
        { "ContactTime", DATA_ULONG, 1, 0 },
        { "ContactLength", DATA_ULONG, 1, 0 },
        { "ContactInvisible", DATA_ULONG, 1, 0 },
        { "HideIP", DATA_BOOL, 1, 0 },
        { "IgnoreAuth", DATA_BOOL, 1, 0 },
        { "DirectMode", DATA_ULONG, 1, 0 },
        { "IdleTime", DATA_ULONG, 1, 0 },
        { "ListRequests", DATA_STRING, 1, 0 },
        { "Picture", DATA_UTF, 1, 0 },
        { "RandomChatGroup", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },			// RandomChatGroupCurrent
        { "", DATA_STRUCT, sizeof(ICQUserData) / sizeof(unsigned), (unsigned)_icqUserData },
        { NULL, 0, 0, 0 }
    };

ENCODING encodingTbl[] =
    {
        { I18N_NOOP("Unicode"), "UTF-8", 106, true },

        { I18N_NOOP("Arabic"), "ISO 8859-6", 82, false },
        { I18N_NOOP("Arabic"), "CP 1256", 2256, true },

        { I18N_NOOP("Baltic"), "ISO 8859-13", 109, false },
        { I18N_NOOP("Baltic"), "CP 1257", 2257, true },

        { I18N_NOOP("Central European"), "ISO 8859-2", 5, false },
        { I18N_NOOP("Esperanto"), "ISO 8859-3", 6, false },
        { I18N_NOOP("Central European"), "CP 1250", 2250, true },

        { I18N_NOOP("Chinese "), "GBK", 2025, false },
        { I18N_NOOP("Chinese Simplified"), "gbk2312", 2312, false },
        { I18N_NOOP("Chinese Traditional"), "Big5", 2026, true },

        { I18N_NOOP("Cyrillic"), "ISO 8859-5", 8, false },
        { I18N_NOOP("Cyrillic"), "KOI8-R", 2084, false },
        { I18N_NOOP("Ukrainian"), "KOI8-U", 2088, false },
        { I18N_NOOP("Cyrillic"), "CP 1251", 2251, true },

        { I18N_NOOP("Greek"), "ISO 8859-7", 10, false },
        { I18N_NOOP("Greek"), "CP 1253", 2253, true },

        { I18N_NOOP("Hebrew"), "ISO 8859-8-I", 85, false },
        { I18N_NOOP("Hebrew"), "CP 1255", 2255, true },

        { I18N_NOOP("Japanese"), "JIS7", 16, false },
        { I18N_NOOP("Japanese"), "eucJP", 18, false },
        { I18N_NOOP("Japanese"), "Shift-JIS", 17, true },

        { I18N_NOOP("Korean"), "eucKR", 38, true },

        { I18N_NOOP("Western European"), "ISO 8859-1", 4, false },
        { I18N_NOOP("Western European"), "ISO 8859-15", 111, false },
        { I18N_NOOP("Western European"), "CP 1252", 2252, true },

        { I18N_NOOP("Tamil"), "TSCII", 2028, true },

        { I18N_NOOP("Thai"), "TIS-620", 2259, true },

        { I18N_NOOP("Turkish"), "ISO 8859-9", 12, false },
        { I18N_NOOP("Turkish"), "CP 1254", 2254, true },

        { NULL, NULL, 0, false }
    };


ICQClient::ICQClient(ICQProtocol *protocol, const char *cfg)
        : TCPClient(protocol, cfg)
{
    load_data(icqClientData, &data, cfg);
    m_bRosters = false;
    m_listRequest = NULL;
    data.owner.DCcookie = rand();
    char buff[64];
    snprintf(buff, sizeof(buff), "ICQ.%lu", data.owner.Uin);
    m_listener = new ICQListener(this);
    m_bBirthday = false;
    m_bServerReady = false;
    m_infoTimer = new QTimer(this);
    connect(m_infoTimer, SIGNAL(timeout()), this, SLOT(infoRequestFail()));
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(processSendQueue()));
    if (getListRequests()){
        string requests = getListRequests();
        while (requests.length()){
            string req = getToken(requests, ';');
            string n = getToken(req, ',');
            ListRequest lr;
            memset(&lr, 0, sizeof(lr));
            lr.type = atol(n.c_str());
            lr.uin = atol(req.c_str());
            listRequests.push_back(lr);
        }
    }
    disconnected();
}

ICQClient::~ICQClient()
{
    delete m_listener;
    free_data(icqClientData, &data);
    if (m_socket)
        delete m_socket;
}

const DataDef *ICQProtocol::userDataDef()
{
    return icqUserData;
}

bool ICQClient::compareData(void *d1, void *d2)
{
    ICQUserData *data1 = (ICQUserData*)d1;
    ICQUserData *data2 = (ICQUserData*)d2;
    return data1->Uin == data2->Uin;
}

string ICQClient::getConfig()
{
    string listRequest;
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (listRequest.length())
            listRequest += ';';
        listRequest += number((*it).type);
        listRequest += ',';
        listRequest += number((*it).uin);
    }
    setListRequests(listRequest.c_str());
    string res = Client::getConfig();
    if (res.length())
        res += "\n";
    return res += save_data(icqClientData, &data);
}

string ICQClient::name()
{
    string res = "ICQ.";
    res += number(data.owner.Uin);
    return res;
}

QWidget	*ICQClient::setupWnd()
{
    return new ICQConfig(NULL, this, true);
}

void ICQClient::setUin(unsigned long uin)
{
    data.owner.Uin = uin;
}

void ICQClient::connect_ready()
{
    log(L_DEBUG, "Connect ready");
    m_socket->readBuffer.init(6);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
    TCPClient::connect_ready();
}

void ICQClient::setStatus(unsigned status, bool bCommon)
{
    if (status != STATUS_OFFLINE){
        switch (status){
        case STATUS_NA:
        case STATUS_AWAY:
            if (getIdleTime() == 0){
                time_t now;
                time(&now);
                setIdleTime(now);
            }
            break;
        default:
            setIdleTime(0);
        }
    }
    TCPClient::setStatus(status, bCommon);
}

void ICQClient::setStatus(unsigned status)
{
    if (status == STATUS_OFFLINE){
        flap(ICQ_CHNxCLOSE);
        return;
    }
    if (status != m_status){
        m_status = status;
        sendStatus();
        Event e(EventClientChanged, this);
        e.process();
    }
}

void ICQClient::setInvisible(bool bState)
{
    if (bState != getInvisible()){
        TCPClient::setInvisible(bState);
        if (getState() == Connected)
            setInvisible();
        Event e(EventClientChanged, this);
        e.process();
    }
}

void ICQClient::disconnected()
{
    m_infoTimer->stop();
    m_sendTimer->stop();
    clearServerRequests();
    clearListServerRequest();
    clearSMSQueue();
    clearMsgQueue();
    buddies.clear();
    Contact *contact;
    ContactList::ContactIterator it;
    arRequests.clear();
    while ((contact = ++it) != NULL){
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (data->Status != ICQ_STATUS_OFFLINE){
                setOffline(data);
                Event e(EventStatusChanged, contact);
                e.process();
            }
        }
    }
    m_bRosters = false;
    m_nMsgSequence = 0;
    m_bServerReady = false;
    m_bIdleTime = false;
    m_cookie.init(0);
    m_advCounter = 0;
    m_nUpdates = 0;
    m_nSendTimeout = 1;
    m_nSequence = rand() & 0x7FFF;
}

void ICQClient::packet_ready()
{
    if (m_bHeader){
        char c;
        m_socket->readBuffer >> c;
        if (c != 0x2A){
            log(L_ERROR, "Server send bad packet start code: %02X", c);
            m_socket->error_state(I18N_NOOP("Protocol error"));
            return;
        }
        m_socket->readBuffer >> m_nChannel;
        unsigned short sequence, size;
        m_socket->readBuffer >> sequence >> size;
        if (size){
            m_socket->readBuffer.add(size);
            m_bHeader = false;
            return;
        }
        log(L_DEBUG, "Header ready %u (%u)", size, sequence);
    }
    ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->ICQPacket);
    switch (m_nChannel){
    case ICQ_CHNxNEW:
        chn_login();
        break;
    case ICQ_CHNxCLOSE:
        chn_close();
        break;
    case ICQ_CHNxDATA:{
            unsigned short fam, type;
            unsigned short flags, seq, cmd;
            m_socket->readBuffer >> fam >> type >> flags >> seq >> cmd;
            switch (fam){
            case ICQ_SNACxFAM_SERVICE:
                snac_service(type, seq);
                break;
            case ICQ_SNACxFAM_LOCATION:
                snac_location(type, seq);
                break;
            case ICQ_SNACxFAM_BUDDY:
                snac_buddy(type, seq);
                break;
            case ICQ_SNACxFAM_MESSAGE:
                snac_icmb(type, seq);
                break;
            case ICQ_SNACxFAM_BOS:
                snac_bos(type, seq);
                break;
            case ICQ_SNACxFAM_PING:
                snac_ping(type, seq);
                break;
            case ICQ_SNACxFAM_LISTS:
                snac_lists(type, seq);
                break;
            case ICQ_SNACxFAM_VARIOUS:
                snac_various(type, seq);
                break;
            case ICQ_SNACxFAM_LOGIN:
                snac_login(type, seq);
                break;
            default:
                log(L_WARN, "Unknown family %02X", fam);
            }
            break;
        }
    default:
        log(L_ERROR, "Unknown channel %u", m_nChannel & 0xFF);
    }
    m_socket->readBuffer.init(6);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
}

void ICQClient::flap(char channel)
{
    m_nSequence++;
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer
    << (char)0x2A
    << channel
    << m_nSequence
    << 0;
}

void ICQClient::snac(unsigned short fam, unsigned short type, bool msgId)
{
    flap(ICQ_CHNxDATA);
    m_socket->writeBuffer
    << fam
    << type
    << 0x0000
    << (msgId ? ++m_nMsgSequence : 0x0000)
    << type;
}

void ICQClient::sendPacket()
{
    Buffer &writeBuffer = m_socket->writeBuffer;
    char *packet = writeBuffer.data(writeBuffer.packetStartPos());
    *((unsigned short*)(packet + 4)) = htons(writeBuffer.size() - writeBuffer.packetStartPos() - 6);
    ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->ICQPacket);
    m_socket->write();
}

string ICQClient::cryptPassword()
{
    string pswd = fromUnicode(getPassword(), &data.owner);
    const char *p = pswd.c_str();
    string res;
    unsigned char xor_table[] =
        {
            0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
            0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c
        };
    int j;
    for (j = 0; j < 16; j++){
        if (p[j] == 0) break;
        char c = (p[j] ^ xor_table[j]);
        if (c == 0){
            res += "\\";
            c = '0';
        }else if (c == '\\'){
            res += "\\";
        }
        res += c;
    }
    return res;
}

unsigned ICQClient::getStatus()
{
    return m_status;
}

unsigned long ICQClient::fullStatus(unsigned s)
{
    unsigned long status = 0;
    switch (s){
    case STATUS_ONLINE:
        status = ICQ_STATUS_ONLINE;
        break;
    case STATUS_AWAY:
        status = ICQ_STATUS_AWAY;
        break;
    case STATUS_NA:
        status = ICQ_STATUS_NA | ICQ_STATUS_AWAY;
        break;
    case STATUS_OCCUPIED:
        status = ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
        break;
    case STATUS_DND:
        status = ICQ_STATUS_DND | ICQ_STATUS_OCCUPIED | ICQ_STATUS_AWAY;
        break;
    case STATUS_FFC:
        status = ICQ_STATUS_FFC;
        break;
    }
    if (data.owner.WebAware)
        status |= ICQ_STATUS_FxWEBxPRESENCE;
    if (getHideIP())
        status |= ICQ_STATUS_FxHIDExIP;
    if (m_bBirthday)
        status |= ICQ_STATUS_FxBIRTHDAY;
    if (getInvisible())
        status |= ICQ_STATUS_FxPRIVATE;
    switch (getDirectMode()){
    case 1:
        status |= ICQ_STATUS_FxDIRECTxLISTED;
        break;
    case 2:
        status |= ICQ_STATUS_FxDIRECTxAUTH;
        break;
    }
    return status;
}

ICQUserData *ICQClient::findContact(unsigned long uin, const char *alias, bool bCreate, Contact *&contact, Group *grp)
{
    ContactList::ContactIterator it;
    ICQUserData *data;

    while ((contact = ++it) != NULL){
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (data->Uin != uin)
                continue;
            bool bChanged = false;
            if (alias){
                if (*alias){
                    QString name = QString::fromUtf8(alias);
                    bChanged = contact->setName(name);
                }
                set_str(&data->Alias, alias);
            }
            if (grp){
                if (contact->getGroup() != grp->id()){
                    contact->setGroup(grp->id());
                    bChanged = true;
                }
            }
            if (bChanged){
                Event e(EventContactChanged, contact);
                e.process();
            }
            return data;
        }
    }
    if (!bCreate)
        return NULL;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (client == this)
            continue;
        if (client->protocol() != protocol())
            continue;
        ICQClient *c = static_cast<ICQClient*>(client);
        it.reset();
        while ((contact = ++it) != NULL){
            ClientDataIterator it(contact->clientData, c);
            while ((data = (ICQUserData*)(++it)) != NULL){
                if (data->Uin != uin)
                    continue;
                data = (ICQUserData*)(contact->clientData.createData(this));
                data->Uin = uin;
                bool bChanged = false;
                if (alias){
                    if (*alias){
                        QString name = QString::fromUtf8(alias);
                        bChanged = contact->setName(name);
                    }
                    set_str(&data->Alias, alias);
                }
                if (grp){
                    if (grp->id() != contact->getGroup()){
                        contact->setGroup(grp->id());
                        bChanged = true;
                    }
                }
                if (bChanged){
                    Event e(EventContactChanged, contact);
                    e.process();
                }
                return data;
            }
        }
    }
    if (alias && *alias){
        QString name = QString::fromUtf8(alias).lower();
        it.reset();
        while ((contact = ++it) != NULL){
            if (contact->getName().lower() == name){
                ICQUserData *data = (ICQUserData*)(contact->clientData.createData(this));
                data->Uin = uin;
                set_str(&data->Alias, alias);
                Event e(EventContactChanged, contact);
                e.process();
                return data;
            }
        }
    }
    contact = getContacts()->contact(0, true);
    data = (ICQUserData*)(contact->clientData.createData(this));
    data->Uin = uin;
    QString name;
    if (alias && *alias){
        name = QString::fromUtf8(alias);
    }else{
        name = QString::number(uin);
    }
    set_str(&data->Alias, alias);
    contact->setName(name);
    if (grp)
        contact->setGroup(grp->id());
    Event e(EventContactChanged, contact);
    e.process();
    return data;
}

ICQUserData *ICQClient::findGroup(unsigned id, const char *alias, Group *&grp)
{
    ContactList::GroupIterator it;
    ICQUserData *data;
    while ((grp = ++it) != NULL){
        data = (ICQUserData*)(grp->clientData.getData(this));
        if (data && (data->IcqID == id)){
            if (alias)
                set_str(&data->Alias, alias);
            return data;
        }
    }
    if (alias == NULL)
        return NULL;
    it.reset();
    QString name = QString::fromUtf8(alias);
    while ((grp = ++it) != NULL){
        if (grp->getName() == name){
            data = (ICQUserData*)(grp->clientData.createData(this));
            data->IcqID = id;
            set_str(&data->Alias, alias);
            return data;
        }
    }
    grp = getContacts()->group(0, true);
    grp->setName(QString::fromUtf8(alias));
    data = (ICQUserData*)(grp->clientData.createData(this));
    data->IcqID = id;
    set_str(&data->Alias, alias);
    return data;
}

void ICQClient::setOffline(ICQUserData *data)
{
    if (data->Direct){
        delete (SocketNotify*)(data->Direct);
        data->Direct = NULL;
    }
    data->Status = ICQ_STATUS_OFFLINE;
    data->bTyping = false;
    data->bBadClient = false;
    time_t now;
    time(&now);
    data->StatusTime  = now;
    set_str(&data->AutoReply, NULL);
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

void ICQClient::contactInfo(void *_data, unsigned long &curStatus, unsigned &style, const char *&statusIcon, string *icons)
{
    ICQUserData *data = (ICQUserData*)(_data);
    unsigned status = STATUS_ONLINE;
    unsigned s = data->Status;
    if (s == ICQ_STATUS_OFFLINE){
        status = STATUS_OFFLINE;
    }else if (s & ICQ_STATUS_DND){
        status = STATUS_DND;
    }else if (s & ICQ_STATUS_OCCUPIED){
        status = STATUS_OCCUPIED;
    }else if (s & ICQ_STATUS_NA){
        status = STATUS_NA;
    }else if (s & ICQ_STATUS_AWAY){
        status = STATUS_AWAY;
    }else if (s & ICQ_STATUS_FFC){
        status = STATUS_FFC;
    }
    unsigned iconStatus = status;
    const char *dicon = NULL;
    if ((iconStatus == STATUS_ONLINE) && (s & ICQ_STATUS_FxPRIVATE)){
        dicon = "ICQ_invisible";
    }else{
        const CommandDef *def = protocol()->statusList();
        for (; def->text; def++){
            if (def->id == iconStatus){
                dicon = def->icon;
                break;
            }
        }
    }
    if (dicon == NULL)
        return;
    if (status == STATUS_OCCUPIED)
        status = STATUS_DND;
    if (status == STATUS_FFC)
        status = STATUS_ONLINE;
    if (status > curStatus){
        curStatus = status;
        if (statusIcon && icons){
            string iconSave = *icons;
            *icons = statusIcon;
            if (iconSave.length())
                addIcon(icons, iconSave.c_str());
        }
        statusIcon = dicon;
    }else{
        if (statusIcon){
            addIcon(icons, dicon);
        }else{
            statusIcon = dicon;
        }
    }
    if ((iconStatus != STATUS_ONLINE) && (iconStatus != STATUS_OFFLINE) && (s & ICQ_STATUS_FxPRIVATE))
        addIcon(icons, "ICQ_invisible");
    if (icons){
        if (data->Status & ICQ_STATUS_FxBIRTHDAY)
            addIcon(icons, "birthday");
        if (data->FollowMe == 1)
            addIcon(icons, "phone");
        if (data->FollowMe == 2)
            addIcon(icons, "nophone");
        if (status != STATUS_OFFLINE){
            if (data->SharedFiles)
                addIcon(icons, "sharedfiles");
            if (data->ICQPhone == 1)
                addIcon(icons, "icqphone");
            if (data->ICQPhone == 2)
                addIcon(icons, "icqphonebusy");
        }
        if (data->bTyping)
            addIcon(icons, "typing");
    }
    if (data->InvisibleId)
        style |= CONTACT_STRIKEOUT;
    if (data->VisibleId)
        style |= CONTACT_ITALIC;
    if (data->WaitAuth)
        style |= CONTACT_UNDERLINE;
}

void ICQClient::ping()
{
    if (getState() == Connected){
        bool bBirthday = false;
        int year  = data.owner.BirthYear;
        int month = data.owner.BirthMonth;
        int day   = data.owner.BirthDay;
        if (day && month && year){
            time_t now;
            time(&now);
            struct tm *tm = localtime(&now);
            if (((tm->tm_mon + 1) == month) && ((tm->tm_mday + 1) == day))
                bBirthday = true;
        }
        if (bBirthday != m_bBirthday){
            setStatus(m_status);
        }else{
            flap(ICQ_CHNxPING);
            sendPacket();
        }
        m_nSendTimeout = m_nSendTimeout / 2;
        if (m_nSendTimeout < 1)
            m_nSendTimeout = 1;
        QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
    }
}

void ICQClient::setupContact(Contact *contact, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    QString phones;
    if (data->HomePhone){
        phones += toUnicode(trimPhone(data->HomePhone).c_str(), data);
        phones += ",Home Phone,";
        phones += number(PHONE).c_str();
    }
    if (data->HomeFax){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->HomeFax).c_str(), data);
        phones += ",Home Fax,";
        phones += number(FAX).c_str();
    }
    if (data->WorkPhone){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->WorkPhone).c_str(), data);
        phones += ",Work Phone,";
        phones += number(PHONE).c_str();
    }
    if (data->WorkFax){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->WorkFax).c_str(), data);
        phones += ",Work Fax,";
        phones += number(FAX).c_str();
    }
    if (data->PrivateCellular){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->PrivateCellular).c_str(), data);
        phones += ",Private Cellular,";
        phones += number(CELLULAR).c_str();
    }
    if (data->PhoneBook){
        if (phones.length())
            phones += ";";
        phones += toUnicode(data->PhoneBook, data);
    }
    string n = name();
    if (contact != getContacts()->owner()){
        contact->setPhones(phones, n.c_str());
        QString mails;
        if (data->EMail)
            mails += toUnicode(trim(data->EMail).c_str(), data);
        if (data->EMails){
            string emails = data->EMails;
            while (emails.length()){
                string mailItem = getToken(emails, ';', false);
                string mail = trim(getToken(mailItem, '/').c_str());
                if (mail.length()){
                    if (mails.length())
                        mails += ";";
                    mails += toUnicode(mail.c_str(), data);
                }
            }
        }
        contact->setEMails(mails, n.c_str());
    }
    QString firstName = toUnicode(data->FirstName, data);
    if (firstName.length())
        contact->setFirstName(firstName, n.c_str());
    QString lastName = toUnicode(data->LastName, data);
    if (lastName.length())
        contact->setLastName(lastName, n.c_str());
    if (contact->getName().isEmpty())
        contact->setName(QString::number(data->Uin));
    QString nick = toUnicode(data->Nick, data);
    if (nick.isEmpty())
        nick = QString::fromUtf8(data->Alias);
    if (!nick.isEmpty()){
        QString name = QString::number(data->Uin);
        if (name == contact->getName())
            contact->setName(nick);
    }
}

string ICQClient::trimPhone(const char *from)
{
    string res;
    if (from == NULL)
        return res;
    res = from;
    char *p = strstr((char*)res.c_str(), "SMS");
    if (p)
        *p = 0;
    return trim(res.c_str());
}

QTextCodec *ICQClient::getCodec(const char *encoding)
{
    if ((encoding == NULL) || (*encoding == 0))
        encoding = data.owner.Encoding;
    return _getCodec(encoding);
}

QTextCodec *ICQClient::_getCodec(const char *encoding)
{
    QTextCodec *codec = NULL;
    if (encoding)
        codec = QTextCodec::codecForName(encoding);
    if (codec == NULL){
        codec = QTextCodec::codecForLocale();
        const ENCODING *e;
        for (e = encodingTbl; e->language; e++){
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
    }
    return codec;
}

QString ICQClient::toUnicode(const char *serverText, const char *clientName, unsigned contactId)
{
    Contact *c = getContacts()->contact(contactId);
    if (c){
        void *data;
        ClientDataIterator it(c->clientData);
        while ((data = ++it) != NULL){
            if (it.client()->dataName(data) != clientName)
                continue;
            return static_cast<ICQClient*>(it.client())->toUnicode(serverText, (ICQUserData*)data);
        }
    }
    QTextCodec *codec = _getCodec(NULL);
    return codec->toUnicode(serverText, strlen(serverText));
}

QString ICQClient::toUnicode(const char *str, ICQUserData *client_data)
{
    if ((str == NULL) || (*str == 0))
        return QString();
    QTextCodec *codec = getCodec(client_data ? client_data->Encoding : NULL);
    return codec->toUnicode(str, strlen(str));
}

string ICQClient::fromUnicode(const QString &str, ICQUserData *client_data)
{
    string res;
    if (str.isEmpty())
        return res;
    QString s = str;
    s.replace(QRegExp("\r"), "");
    s.replace(QRegExp("\n"), "\r\n");
    QTextCodec *codec = getCodec(client_data ? client_data->Encoding : NULL);
    QCString cstr = codec->fromUnicode(s);
    res = (const char*)cstr;
    return res;
}

QString ICQClient::contactTip(void *_data)
{
    ICQUserData *data = (ICQUserData*)(_data);
    QString res;
    QString statusText;
    unsigned long status = STATUS_OFFLINE;
    unsigned style  = 0;
    const char *statusIcon = NULL;
    contactInfo(data, status, style, statusIcon);
    res += "<img src=\"icon:";
    res += statusIcon;
    res += "\">";
    for (const CommandDef *cmd = protocol()->statusList(); cmd->text; cmd++){
        if (!strcmp(cmd->icon, statusIcon)){
            res += " ";
            statusText += i18n(cmd->text);
            res += statusText;
            break;
        }
    }
    res += "<br>";
    res += "UIN: <b>";
    res += number(data->Uin).c_str();
    res += "</b>";
    if (data->Status == ICQ_STATUS_OFFLINE){
        if (data->StatusTime){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatTime(data->Status);
        }
    }else{
        if (data->OnlineTime){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatTime(data->OnlineTime);
        }
        if (data->Status & (ICQ_STATUS_AWAY | ICQ_STATUS_NA)){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatTime(data->StatusTime);
        }
    }
    if (data->IP){
        res += "<br>";
        res += formatAddr(data->IP, data->Port);
    }
    if ((data->RealIP) && ((data->IP == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        res += "<br>";
        res += formatAddr(data->RealIP, data->Port);
    }
    string client_name = clientName(data);
    if (client_name.length()){
        res += "<br>";
        res += quoteString(client_name.c_str());
    }
    if (data->PictureWidth && data->PictureHeight){
        QImage img(pictureFile(data));
        if (!img.isNull()){
            QPixmap pict;
            pict.convertFromImage(img);
            int w = pict.width();
            int h = pict.height();
            if (h > w){
                if (h > 60){
                    w = w * 60 / h;
                    h = 60;
                }
            }else{
                if (w > 60){
                    h = h * 60 / w;
                    w = 60;
                }
            }
            QMimeSourceFactory::defaultFactory()->setPixmap("pict://icq", pict);
            res += "<br><img src=\"pict://icq\" width=\"";
            res += number(w).c_str();
            res += "\" height=\"";
            res += number(h).c_str();
            res += "\">";
        }
    }
    if (data->AutoReply && *data->AutoReply){
        res += "<br><br>";
        res += quoteString(toUnicode(data->AutoReply, data));
    }
    return res;
}

bool ICQClient::hasCap(ICQUserData *data, int n)
{
    return (data->Caps & (1 << n)) != 0;
}

static string verString(unsigned ver)
{
    string res;
    if (ver == 0) return res;
    unsigned char v[4];
    v[0] = (ver >> 24) & 0xFF;
    v[1] = (ver >> 16) & 0xFF;
    v[2] = (ver >> 8) & 0xFF;
    v[3] = ver & 0xFF;
    if ((v[0] & 0x80) || (v[1] & 0x80) || (v[2] & 0x80) || (v[3] & 0x80))
        return res;
    char b[32];
    snprintf(b, sizeof(b), " %u.%u", v[0], v[1]);
    res = b;
    if (v[2] || v[3]){
        snprintf(b, sizeof(b), ".%u", v[2]);
        res += b;
    }
    if (v[3]){
        snprintf(b, sizeof(b), ".%u", v[3]);
        res += b;
    }
    return res;
}

string ICQClient::clientName(ICQUserData *data)
{
    string res;
    char b[32];
    if (data->Version)
    {
        snprintf(b, sizeof(b), "v%lu ", data->Version);
        res = b;
    }
    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN))
    {
        res += "Trillian";
        return res;
    }

    if (hasCap(data, CAP_SIMOLD))
    {
        int hiVersion = (data->Build >> 6) - 1;
        unsigned loVersion = data->Build & 0x1F;
        if ((hiVersion < 0) || ((hiVersion == 0) && (loVersion == 0))){
            res += "Kopete";
            return res;
        }
        snprintf(b, sizeof(b), "SIM %u.%u", (unsigned)hiVersion, loVersion);
        res += b;
        return res;
    }

    if (hasCap(data, CAP_SIM))
    {
        unsigned ver1 = (data->Build >> 24) & 0xFF;
        unsigned ver2 = (data->Build >> 16) & 0xFF;
        unsigned ver3 = (data->Build >> 8) & 0xFF;
        if (ver3){
            snprintf(b, sizeof(b), "SIM %u.%u.%u", ver1, ver2, ver3);
        }else{
            snprintf(b, sizeof(b), "SIM %u.%u", ver1, ver2);
        }
        res += b;
        if (data->Build & 0x80)
            res += "/win32";
        return res;
    }

    if (hasCap(data, CAP_LICQ))
    {
        unsigned ver1 = (data->Build >> 24) & 0xFF;
        unsigned ver2 = (data->Build >> 16) & 0xFF;
        unsigned ver3 = (data->Build >> 8) & 0xFF;
        if (ver3){
            snprintf(b, sizeof(b), "Licq %u.%u.%u", ver1, ver2, ver3);
        }else{
            snprintf(b, sizeof(b), "Licq %u.%u", ver1, ver2);
        }
        res += b;
        if (data->Build & 0xFF)
            res += "/SSL";
        return res;
    }
    if (hasCap(data, CAP_TYPING))
    {
        res += (data->Version == 9) ? "ICQ Lite" : "ICQ2go";
        return res;
    }
    if (hasCap(data, CAP_MACICQ)){
        res += "ICQ for Mac";
        return res;
    }
    if (hasCap(data, CAP_AIM_CHAT)){
        res += "AIM";
        return res;
    }
    if ((data->InfoUpdateTime & 0xFF7F0000L) == 0x7D000000L){
        unsigned ver = data->InfoUpdateTime & 0xFFFF;
        if (ver % 10){
            snprintf(b, sizeof(b), "Licq %u.%u.%u", ver / 1000, (ver / 10) % 100, ver % 10);
        }else{
            snprintf(b, sizeof(b), "Licq %u.%u", ver / 1000, (ver / 10) % 100);
        }
        res += b;
        if (data->InfoUpdateTime & 0x00800000L)
            res += "/SSL";
        return res;
    }
    switch (data->InfoUpdateTime){
    case 0xFFFFFFFFL:
        if ((data->PluginStatusTime == 0xFFFFFFFFL) && (data->PluginInfoTime == 0xFFFFFFFFL)){
            res += "GAIM";
            return res;
        }
        res += "MIRANDA";
        res += verString(data->PluginInfoTime & 0xFFFFFF);
        if (data->PluginInfoTime & 0x80000000)
            res += " alpha";
        return res;
    case 0xFFFFFF8FL:
        res += "StrICQ";
        res += verString(data->PluginInfoTime & 0xFFFFFF);
        return res;
    case 0xFFFFFF42L:
        res += "mICQ";
        return res;
    case 0xFFFFFFBEL:
        res += "alicq";
        res += verString(data->PluginInfoTime & 0xFFFF);
        return res;
    case 0xFFFFFF7FL:
        res += "&RQ";
        res += verString(data->PluginInfoTime & 0xFFFF);
        return res;
    case 0xFFFFFFABL:
        res += "YSM";
        res += verString(data->PluginInfoTime & 0xFFFF);
        return res;
    case 0x04031980L:
        snprintf(b, sizeof(b), "vICQ 0.43.%lu.%lu", data->PluginInfoTime & 0xffff, data->PluginInfoTime & (0x7fff0000) >> 16);
        res += b;
        return res;
    case 0x3AA773EEL:
        if ((data->PluginStatusTime == 0x3AA66380L) && (data->PluginInfoTime == 0x3A877A42L))
        {
            res += "libicq2000";
            return res;
        }
        break;
    }

    if (data->InfoUpdateTime &&
            (data->InfoUpdateTime == data->PluginStatusTime) &&
            (data->PluginStatusTime == data->PluginInfoTime) && (data->Caps == 0)){
        res += "vICQ";
        return res;
    }
    if (hasCap(data, CAP_SRV_RELAY) && hasCap(data, CAP_UTF)){
        res += "ICQ 2002";
        return res;
    }
    if ((hasCap(data, CAP_STR_2001) || hasCap(data, CAP_SRV_RELAY)) && hasCap(data, CAP_IS_2001)){
        res += "ICQ 2001";
        return res;
    }
    if ((data->Version == 7) && hasCap(data, CAP_RTF)){
        res += "GnomeICU";
        return res;
    }
    return res;
}

const unsigned MAIN_INFO = 1;
const unsigned HOME_INFO = 2;
const unsigned WORK_INFO = 3;
const unsigned MORE_INFO = 4;
const unsigned ABOUT_INFO = 5;
const unsigned INTERESTS_INFO = 6;
const unsigned PAST_INFO = 7;
const unsigned PICTURE_INFO = 8;
const unsigned NETWORK   = 9;
const unsigned SECURITY  = 10;

static CommandDef icqWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "ICQ_online",
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
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
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
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
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
            MORE_INFO,
            I18N_NOOP("More info"),
            "more",
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
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
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
            INTERESTS_INFO,
            I18N_NOOP("Interests"),
            "interest",
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
            PAST_INFO,
            I18N_NOOP("Group/Past"),
            "past",
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
            PICTURE_INFO,
            I18N_NOOP("Picture"),
            "pict",
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

static CommandDef icqConfigWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "ICQ_online",
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
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
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
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
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
            MORE_INFO,
            I18N_NOOP("More info"),
            "more",
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
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
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
            INTERESTS_INFO,
            I18N_NOOP("Interests"),
            "interest",
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
            PAST_INFO,
            I18N_NOOP("Group/Past"),
            "past",
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
            PICTURE_INFO,
            I18N_NOOP("Picture"),
            "pict",
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
            SECURITY,
            I18N_NOOP("Security"),
            "security",
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

CommandDef *ICQClient::infoWindows(Contact*, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::number(data->Uin);
    icqWnd[0].text_wrk = strdup(name.utf8());
    return icqWnd;
}

CommandDef *ICQClient::configWindows()
{
    QString name = i18n(protocol()->description()->text);
    name += " ";
    name += QString::number(data.owner.Uin);
    icqConfigWnd[0].text_wrk = strdup(name.utf8());
    return icqConfigWnd;
}

QWidget *ICQClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    ICQUserData *data = (ICQUserData*)_data;
    switch (id){
    case MAIN_INFO:
        return new ICQInfo(parent, data, this);
    case HOME_INFO:
        return new HomeInfo(parent, data, this);
    case WORK_INFO:
        return new WorkInfo(parent, data, this);
    case MORE_INFO:
        return new MoreInfo(parent, data, this);
    case ABOUT_INFO:
        return new AboutInfo(parent, data, this);
    case INTERESTS_INFO:
        return new InterestsInfo(parent, data, this);
    case PAST_INFO:
        return new PastInfo(parent, data, this);
    case PICTURE_INFO:
        return new ICQPicture(parent, data, this);
    }
    return NULL;
}

QWidget *ICQClient::configWindow(QWidget *parent, unsigned id)
{
    switch (id){
    case MAIN_INFO:
        return new ICQInfo(parent, NULL, this);
    case HOME_INFO:
        return new HomeInfo(parent, NULL, this);
    case WORK_INFO:
        return new WorkInfo(parent, NULL, this);
    case MORE_INFO:
        return new MoreInfo(parent, NULL, this);
    case ABOUT_INFO:
        return new AboutInfo(parent, NULL, this);
    case INTERESTS_INFO:
        return new InterestsInfo(parent, NULL, this);
    case PAST_INFO:
        return new PastInfo(parent, NULL, this);
    case PICTURE_INFO:
        return new ICQPicture(parent, NULL, this);
    case NETWORK:
        return new ICQConfig(parent, this, false);
    case SECURITY:
        return new ICQSecure(parent, this);
    }
    return NULL;
}

QWidget *ICQClient::searchWindow()
{
    return new ICQSearch(this);
}

void ICQClient::updateInfo(Contact *contact, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    if (getState() != Connected){
        Client::updateInfo(contact, _data);
        return;
    }
    addFullInfoRequest(data->Uin, false);
}

void *ICQClient::processEvent(Event *e)
{
    if (e->type() == EventTemplateExpanded){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        list<ar_request>::iterator it;
        for (it = arRequests.begin(); it != arRequests.end(); ++it)
            if (&(*it) == t->param)
                break;
        if (it == arRequests.end())
            return NULL;
        ar_request ar = (*it);
        Buffer copy;
        string response;
        response = t->tmpl.utf8();
        sendAutoReply(ar.uin, ar.timestamp1, ar.timestamp2, plugins[PLUGIN_NULL],
                      ar.id1, ar.id2, ar.type, 3, 256, response.c_str(), 0, copy);
        arRequests.erase(it);
        return e->param();
    }
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (getState() == Connected){
            addBuddy(contact);
            if (contact == getContacts()->owner()){
                time_t now;
                time(&now);
                if (getContacts()->owner()->getPhones() != QString::fromUtf8(data.owner.PhoneBook)){
                    set_str(&data.owner.PhoneBook, getContacts()->owner()->getPhones().utf8());
                    data.owner.PluginInfoTime = now;
                    sendPluginInfoUpdate(PLUGIN_PHONEBOOK);
                }
                if (getPicture() != QString::fromUtf8(data.owner.Picture)){
                    set_str(&data.owner.Picture, getPicture().utf8());
                    data.owner.PluginInfoTime = now;
                    sendPluginInfoUpdate(PLUGIN_PICTURE);
                }
                if (getContacts()->owner()->getPhoneStatus() != data.owner.FollowMe){
                    data.owner.FollowMe = getContacts()->owner()->getPhoneStatus();
                    data.owner.PluginStatusTime = now;
                    sendPluginStatusUpdate(PLUGIN_FOLLOWME, data.owner.FollowMe);
                }
                return NULL;
            }
        }
        addContactRequest(contact);
    }
    if (e->type() == EventContactDeleted){
        Contact *contact =(Contact*)(e->param());
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (data->IcqID == 0)
                continue;
            list<ListRequest>::iterator it;
            for (it = listRequests.begin(); it != listRequests.end(); it++){
                if ((*it).type != LIST_USER_CHANGED)
                    continue;
                if ((*it).uin == data->Uin)
                    break;
            }
            if (it != listRequests.end())
                listRequests.erase(it);
            ListRequest lr;
            memset(&lr, 0, sizeof(lr));
            lr.type = LIST_USER_DELETED;
            lr.uin = data->Uin;
            lr.icq_id = data->IcqID;
            lr.grp_id = data->GrpId;
            lr.visible_id = data->ContactVisibleId;
            lr.invisible_id = data->ContactInvisibleId;
            lr.ignore_id = data->IgnoreId;
            listRequests.push_back(lr);
            processListRequest();
        }
    }
    if (e->type() == EventGroupChanged){
        Group *group = (Group*)(e->param());
        if (group->id())
            addGroupRequest(group);
    }
    if (e->type() == EventGroupDeleted){
        Group *group =(Group*)(e->param());
        if (group->id()){
            ICQUserData *data = (ICQUserData*)(group->clientData.getData(this));
            if (data){
                ListRequest lr;
                memset(&lr, 0, sizeof(lr));
                lr.type = LIST_GROUP_DELETED;
                lr.icq_id = data->IcqID;
                listRequests.push_back(lr);
                processListRequest();
            }
        }
    }
    if (e->type() == EventMessageCancel){
        Message *msg = (Message*)(e->param());
        if (msg->type() == MessageSMS){
            for (list<SendMsg>::iterator it = smsQueue.begin(); it != smsQueue.end(); ++it){
                if ((*it).msg == msg){
                    if (it == smsQueue.begin()){
                        (*it).text = "";
                    }else{
                        smsQueue.erase(it);
                    }
                    return msg;
                }
            }
        }else{
            if (m_send.msg == msg){
                m_send.msg = NULL;
                m_send.uin = 0;
                send(true);
                return msg;
            }
            for (list<SendMsg>::iterator it = sendQueue.begin(); it != sendQueue.end(); ++it){
                if ((*it).msg == msg){
                    sendQueue.erase(it);
                    return msg;
                }
            }
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
        if (cmd->menu_id == plugin->MenuEncoding){
            if (cmd->id == plugin->CmdChangeEncoding){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                ClientDataIterator itClient(contact->clientData, this);
                ICQUserData *data = (ICQUserData*)(++itClient);
                if (data == NULL)
                    return NULL;
                unsigned nEncoding = 3;
                QStringList main;
                QStringList nomain;
                QStringList::Iterator it;
                const ENCODING *enc;
                for (enc = encodingTbl; enc->language; enc++){
                    if (enc->bMain){
                        main.append(i18n(enc->language) + " (" + enc->codec + ")");
                        nEncoding++;
                        continue;
                    }
                    if (!plugin->getShowAllEncodings())
                        continue;
                    nomain.append(i18n(enc->language) + " (" + enc->codec + ")");
                    nEncoding++;
                }
                CommandDef *cmds = new CommandDef[nEncoding];
                memset(cmds, 0, sizeof(CommandDef) * nEncoding);
                cmd->param = cmds;
                cmd->flags |= COMMAND_RECURSIVE;
                nEncoding = 0;
                cmds[nEncoding].id = 1;
                cmds[nEncoding].text = I18N_NOOP("System");
                if (data->Encoding == NULL)
                    cmds[nEncoding].flags = COMMAND_CHECKED;
                nEncoding++;
                main.sort();
                for (it = main.begin(); it != main.end(); ++it){
                    QString str = *it;
                    int n = str.find('(');
                    str = str.mid(n + 1);
                    n = str.find(')');
                    str = str.left(n);
                    if (data->Encoding && !strcmp(data->Encoding, str.latin1()))
                        cmds[nEncoding].flags = COMMAND_CHECKED;
                    cmds[nEncoding].id = nEncoding + 1;
                    cmds[nEncoding].text = "_";
                    cmds[nEncoding].text_wrk = strdup((*it).utf8());
                    nEncoding++;
                }
                if (!plugin->getShowAllEncodings())
                    return e->param();
                cmds[nEncoding++].text = "_";
                nomain.sort();
                for (it = nomain.begin(); it != nomain.end(); ++it){
                    QString str = *it;
                    int n = str.find('(');
                    str = str.mid(n + 1);
                    n = str.find(')');
                    str = str.left(n);
                    if (data->Encoding && !strcmp(data->Encoding, str.latin1()))
                        cmds[nEncoding].flags = COMMAND_CHECKED;
                    cmds[nEncoding].id = nEncoding;
                    cmds[nEncoding].text = "_";
                    cmds[nEncoding].text_wrk = strdup((*it).utf8());
                    nEncoding++;
                }
                return e->param();
            }
            if (cmd->id == plugin->CmdAllEncodings){
                cmd->flags &= ~COMMAND_CHECKED;
                if (plugin->getShowAllEncodings())
                    cmd->flags |= COMMAND_CHECKED;
                return e->param();
            }
        }
        if ((cmd->bar_id == ToolBarContainer) || (cmd->bar_id == BarHistory)){
            if (cmd->id == plugin->CmdChangeEncoding){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL){
                    cmd->flags |= BTN_HIDE;
                    return e->param();
                }
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client == this){
                        cmd->flags |= BTN_HIDE;
                        break;
                    }
                    if (client->protocol() == protocol())
                        break;
                }
                ClientDataIterator it(contact->clientData, this);
                if ((++it) != NULL){
                    cmd->flags &= ~BTN_HIDE;
                    return e->param();
                }
                return NULL;
            }
        }
        if (cmd->menu_id == MenuContactGroup){
            if (cmd->id == plugin->CmdVisibleList){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client == this){
                        cmd->flags &= ~COMMAND_CHECKED;
                        break;
                    }
                    if (client->protocol() == protocol())
                        break;
                }
                ICQUserData *data;
                bool bOK = false;
                ClientDataIterator it(contact->clientData, this);
                while ((data = (ICQUserData*)(++it)) != NULL){
                    bOK = true;
                    if (data->VisibleId)
                        cmd->flags |= COMMAND_CHECKED;
                }
                return bOK ? e->param() : NULL;
            }
            if (cmd->id == plugin->CmdInvisibleList){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client == this){
                        cmd->flags &= ~COMMAND_CHECKED;
                        break;
                    }
                    if (client->protocol() == protocol())
                        break;
                }
                ICQUserData *data;
                bool bOK = false;
                ClientDataIterator it(contact->clientData, this);
                while ((data = (ICQUserData*)(++it)) != NULL){
                    bOK = true;
                    if (data->InvisibleId)
                        cmd->flags |= COMMAND_CHECKED;
                }
                return bOK ? e->param() : NULL;
            }
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
        if (cmd->menu_id == plugin->MenuEncoding){
            if (cmd->id == plugin->CmdAllEncodings){
                plugin->setShowAllEncodings(!plugin->getShowAllEncodings());
                return e->param();
            }
            Contact *contact = getContacts()->contact((unsigned)(cmd->param));
            if (contact == NULL)
                return NULL;
            QCString codecStr;
            const char *codec = NULL;
            if (cmd->id == 1){
                codec = "";
            }else{
                QStringList main;
                QStringList nomain;
                QStringList::Iterator it;
                const ENCODING *enc;
                for (enc = encodingTbl; enc->language; enc++){
                    if (enc->bMain){
                        main.append(i18n(enc->language) + " (" + enc->codec + ")");
                        continue;
                    }
                    if (!plugin->getShowAllEncodings())
                        continue;
                    nomain.append(i18n(enc->language) + " (" + enc->codec + ")");
                }
                QString str;
                main.sort();
                int n = cmd->id - 1;
                for (it = main.begin(); it != main.end(); ++it){
                    if (--n == 0){
                        str = *it;
                        break;
                    }
                }
                if (n >= 0){
                    nomain.sort();
                    for (it = nomain.begin(); it != nomain.end(); ++it){
                        if (--n == 0){
                            str = *it;
                            break;
                        }
                    }
                }
                if (!str.isEmpty()){
                    int n = str.find('(');
                    str = str.mid(n + 1);
                    n = str.find(')');
                    codecStr = str.left(n).latin1();
                    codec = codecStr;
                }
            }
            if (codec == NULL)
                return NULL;
            ICQUserData *data;
            ClientDataIterator it(contact->clientData, this);
            bool bChanged = false;
            while ((data = (ICQUserData*)(++it)) != NULL){
                if (set_str(&data->Encoding, codec))
                    bChanged = true;
            }
            if (bChanged){
                Event eContact(EventContactChanged, contact);
                eContact.process();
                Event eh(EventHistoryConfig, (void*)(contact->id()));
                eh.process();
            }
            return NULL;
        }
        if (cmd->menu_id == MenuContactGroup){
            if (cmd->id == plugin->CmdVisibleList){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                ICQUserData *data;
                ClientDataIterator it(contact->clientData, this);
                bool bChanged = false;
                while ((data = (ICQUserData*)(++it)) != NULL){
                    data->VisibleId = (cmd->flags & COMMAND_CHECKED) ? getListId() : 0;
                    bChanged = true;
                }
                if (bChanged){
                    Event eContact(EventContactChanged, contact);
                    eContact.process();
                }
                return e->param();
            }
            if (cmd->id == plugin->CmdInvisibleList){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                ICQUserData *data;
                ClientDataIterator it(contact->clientData, this);
                bool bChanged = false;
                while ((data = (ICQUserData*)(++it)) != NULL){
                    data->InvisibleId = (cmd->flags & COMMAND_CHECKED) ? getListId() : 0;
                    bChanged = true;
                }
                if (bChanged){
                    Event eContact(EventContactChanged, contact);
                    eContact.process();
                }
                return e->param();
            }
        }
    }
    if (e->type() == EventGoURL){
        string url = (const char*)(e->param());
        string proto;
        int n = url.find(':');
        if (n < 0)
            return NULL;
        proto = url.substr(0, n);
        if (proto != "icq")
            return NULL;
        url = url.substr(proto.length() + 1);
        while (url[0] == '/')
            url = url.substr(1);
        string s = unquoteText(url.c_str());
        unsigned long uin = atol(getToken(s, ',').c_str());
        if (uin){
            Contact *contact;
            findContact(uin, s.c_str(), true, contact);
            Command cmd;
            cmd->id		 = MessageGeneric;
            cmd->menu_id = MenuMessage;
            cmd->param	 = (void*)(contact->id());
            Event eCmd(EventCommandExec, cmd);
            eCmd.process();
        }
    }
    if ((e->type() == EventStartTyping) || (e->type() == EventStopTyping)){
        if (getState() != Connected)
            return NULL;
        Contact *contact = getContacts()->contact((unsigned long)(e->param()));
        if (contact == NULL)
            return NULL;
        ClientDataIterator it(contact->clientData, this);
        ICQUserData *data = (ICQUserData*)(++it);
        if (data == NULL)
            return NULL;
        if (getInvisible()){
            if (data->VisibleId == 0)
                return NULL;
        }else{
            if (data->InvisibleId)
                return NULL;
        }
        if (!hasCap(data, CAP_TYPING))
            return NULL;
        sendMTN(data->Uin, (e->type() == EventStartTyping) ? ICQ_MTN_START : ICQ_MTN_FINISH);
        return e->param();
    }
    return NULL;
}

bool ICQClient::send(Message *msg, void *_data)
{
    if (getState() != Connected)
        return false;
	ICQUserData *data = (ICQUserData*)_data;
    SendMsg s;
    switch (msg->type()){
    case MessageSMS:
        s.msg    = static_cast<SMSMessage*>(msg);
        s.text   = s.msg->getPlainText();
        s.flags  = SEND_1STPART;
        if (s.msg->getFlags() & MESSAGE_TRANSLIT)
            s.text = toTranslit(s.text);
        smsQueue.push_back(s);
        processSMSQueue();
        return true;
    case MessageAuthRequest:
		if (data && data->WaitAuth)
			return sendAuthRequest(msg, data);
		break;
    case MessageAuthGranted:
		if (data && data->WantAuth)
			return sendAuthGranted(msg, data);
		break;
    case MessageAuthRefused:
		if (data && data->WantAuth)
			return sendAuthRefused(msg, data);
		break;
    }
    if (sendThruServer(msg, data))
        return true;
    return false;
}

bool ICQClient::canSend(unsigned type, void *_data)
{
    if (getState() != Connected)
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    switch (type){
    case MessageSMS:
        return true;
    case MessageGeneric:
    case MessageURL:
    case MessageContact:
        return (data != NULL);
    case MessageAuthRequest:
        return data && (data->WaitAuth);
    case MessageAuthGranted:
        return data && (data->WantAuth);
    }
    return false;
}

string ICQClient::dataName(void *data)
{
    return dataName(((ICQUserData*)data)->Uin);
}

string ICQClient::dataName(unsigned long uin)
{
    string res = name();
    res += ".";
    res += number(uin);
    return res;
}

void ICQClient::messageReceived(Message *msg, unsigned long uin)
{
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    if (msg->contact() == 0){
        Contact *contact;
        void *data = findContact(uin, NULL, false, contact);
        if (data == NULL){
            data = findContact(uin, NULL, true, contact);
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
    if (!e.process())
        delete msg;
}

rtf_charset ICQClient::rtf_charsets[] =
    {
        { "CP 1251", 204 },
        { "KOI8-R", 204 },
        { "KOI8-U", 204 },
        { "ISO 8859-6-I", 180 },
        { "ISO 8859-7", 161 },
        { "ISO 8859-8-I", 177 },
        { "ISO 8859-9", 162 },
        { "CP 1254", 162 },
        { "eucJP", 128 },
        { "sjis7", 128 },
        { "jis7", 128 },
        { "CP 1250", 238 },
        { "ISO 8859-2", 238 },
        { "", 0 }
    };

QString ICQClient::ownerName()
{
    return contactName(&data.owner);
}

QString ICQClient::contactName(void *clientData)
{
    QString res = Client::contactName(clientData);
    res += ": ";
    ICQUserData *data = (ICQUserData*)clientData;
    if (data->Nick && *data->Nick){
        res += toUnicode(data->Nick, data);
        res += " (";
        res += QString::number(data->Uin);
        res += ")";
    }else{
        res += QString::number(data->Uin);
    }
    return res;
}

void ICQClient::addPluginInfoRequest(unsigned long uin, unsigned plugin_index)
{
    list<SendMsg>::iterator it;
    for (it = sendQueue.begin(); it != sendQueue.end(); ++it){
        SendMsg &s = *it;
        if ((s.uin == uin) && (s.flags == plugin_index) && (s.msg == NULL))
            break;
    }
    if (it != sendQueue.end())
        return;
    SendMsg s;
    s.uin   = uin;
    s.flags = plugin_index;
    sendQueue.push_back(s);
    send(true);
}

void ICQClient::randomChatInfo(unsigned long uin)
{
    addPluginInfoRequest(uin, PLUGIN_RANDOM_CHAT);
}

#ifdef WIN32
static char PICT_PATH[] = "pictures\\";
#else
static char PICT_PATH[] = "pictures/";
#endif

QString ICQClient::pictureFile(ICQUserData *data)
{
    string f = PICT_PATH;
    f += "icq.";
    f += number(data->Uin);
    f = user_file(f.c_str());
    return QString::fromLocal8Bit(f.c_str());
}

#ifndef WIN32
#include "icqclient.moc"
#endif

