/***************************************************************************
                          icqclient.cpp  -  description
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
#include "aimconfig.h"
#include "icqinfo.h"
#include "homeinfo.h"
#include "workinfo.h"
#include "moreinfo.h"
#include "aboutinfo.h"
#include "interestsinfo.h"
#include "pastinfo.h"
#include "icqpicture.h"
#include "aiminfo.h"
#include "icqsearch.h"
#include "aimsearch.h"
#include "icqsecure.h"
#include "icqmessage.h"
#include "securedlg.h"
#include "core.h"
#include "msgedit.h"
#include "ballonmsg.h"
#include "encodingdlg.h"
#include "warndlg.h"

#include "simapi.h"
#include "buffer.h"
#include "socket.h"

#include <qtimer.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qfile.h>

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <ctype.h>
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
        { "", DATA_ULONG, 1, DATA(1) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "", DATA_UTF, 1, 0 },					// Alias
        { "", DATA_UTF, 1, 0 },					// Cellular
        { "", DATA_ULONG, 1, DATA(0xFFFF) },		// Status
        { "", DATA_ULONG, 1, 0 },				// Class
        { "StatusTime", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// OnlineTime
        { "WarningLevel", DATA_ULONG, 1, 0 },
        { "IP", DATA_IP, 1, 0 },
        { "RealIP", DATA_IP, 1, 0 },
        { "Port", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// DCcookie
        { "Caps", DATA_ULONG, 1, 0 },
        { "", DATA_STRING, 1, 0 },				// AutoReply
        { "Uin", DATA_ULONG, 1, 0 },
        { "Screen", DATA_STRING, 1, 0 },
        { "ID", DATA_ULONG, 1, 0 },
        { "", DATA_BOOL, 1, DATA(1) },				// bChecked
        { "GroupID", DATA_ULONG, 1, 0 },
        { "Ignore", DATA_ULONG, 1, 0 },
        { "Visible", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// ContactVisibleId
        { "Invsible", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },				// ContactInvisibleId
        { "WaitAuth", DATA_BOOL, 1, 0 },
        { "WantAuth", DATA_BOOL, 1, 0 },
        { "WebAware", DATA_BOOL, 1, DATA(1) },
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
        { "MiddleName", DATA_STRING, 1, 0 },
        { "Maiden", DATA_STRING, 1, 0 },
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
        { "ProfileFetch", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },				// bTyping
        { "", DATA_BOOL, 1, 0 },				// bBadClient
        { "", DATA_OBJECT, 1, 0 },				// Direct
        { "", DATA_OBJECT, 1, 0 },				// DirectPluginInfo
        { "", DATA_OBJECT, 1, 0 },				// DirectPluginStatus
        { "", DATA_BOOL, 1, 0 },				// bNoDirect
        { "", DATA_BOOL, 1, 0 },				// bInviisble
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
        { "Server", DATA_STRING, 1, 0 },
        { "ServerPort", DATA_ULONG, 1, DATA(5190) },
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
        { "SendFormat", DATA_ULONG, 1, 0 },
        { "AutoUpdate", DATA_BOOL, 1, DATA(1) },
        { "AutoReplyUpdate", DATA_BOOL, 1, DATA(1) },
        { "TypingNotification", DATA_BOOL, 1, DATA(1) },
        { "AcceptInDND", DATA_BOOL, 1, 0 },
        { "AcceptInOccupied", DATA_BOOL, 1, 0 },
        { "MinPort", DATA_ULONG, 1, DATA(1024) },
        { "MaxPort", DATA_ULONG, 1, DATA(0xFFFE) },
        { "WarnAnonimously", DATA_BOOL, 1, 0 },
        { "", DATA_STRUCT, sizeof(ICQUserData) / sizeof(Data), DATA(_icqUserData) },
        { NULL, 0, 0, 0 }
    };

static ENCODING _encodingTbl[] =
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

const ENCODING *ICQClient::encodings = _encodingTbl;

ICQClient::ICQClient(Protocol *protocol, const char *cfg, bool bAIM)
        : TCPClient(protocol, cfg), EventReceiver(HighPriority - 1)
{
    m_bAIM = bAIM;

    m_listener = NULL;
    load_data(icqClientData, &data, cfg);
    if (data.owner.Uin.value != 0)
        m_bAIM = false;
    if (data.owner.Screen.ptr && *data.owner.Screen.ptr)
        m_bAIM = true;
    if (!m_bAIM && (data.owner.Encoding.ptr == NULL)){
        const char *default_enc = static_cast<ICQPlugin*>(protocol->plugin())->getDefaultEncoding();
        if (default_enc && *default_enc){
            set_str(&data.owner.Encoding.ptr, default_enc);
        }else{
            QTextCodec *codec = _getCodec(NULL);
            if (codec && (QString(codec->name()).lower().find("utf") >= 0)){
                const char *_def_enc = I18N_NOOP("Dear translator! type this default encdoing for your language");
                QString def_enc = i18n(_def_enc);
                if (def_enc == _def_enc){
                    EncodingDlg dlg(NULL, this);
                    dlg.exec();
                }else{
                    set_str(&data.owner.Encoding.ptr, def_enc.latin1());
                }
            }
        }
    }
    m_bRosters = false;
    m_listRequest = NULL;
    data.owner.DCcookie.value = rand();
    char buff[64];
    snprintf(buff, sizeof(buff), "ICQ.%lu", data.owner.Uin);
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
            lr.type   = atol(n.c_str());
            lr.screen = req;
            listRequests.push_back(lr);
        }
    }
    disconnected();
    m_infoRequestId = 0;
}

ICQClient::~ICQClient()
{
    setStatus(STATUS_OFFLINE, false);
    if (m_listener)
        delete m_listener;
    free_data(icqClientData, &data);
    if (m_socket)
        delete m_socket;
    for (list<Message*>::iterator it = m_processMsg.begin(); it != m_processMsg.end(); ++it){
        Message *msg = *it;
        msg->setError(I18N_NOOP("Process message failed"));
        Event e(EventRealSendMessage, msg);
        e.process();
        delete msg;
    }
    while (!m_sockets.empty())
        delete m_sockets.front();
    m_processMsg.clear();

    freeData();
}

const DataDef *ICQProtocol::userDataDef()
{
    return _icqUserData;
}

const DataDef *AIMProtocol::userDataDef()
{
    return _icqUserData;
}

bool ICQClient::compareData(void *d1, void *d2)
{
    ICQUserData *data1 = (ICQUserData*)d1;
    ICQUserData *data2 = (ICQUserData*)d2;
    if (data1->Uin.value)
        return data1->Uin.value == data2->Uin.value;
    if (data2->Uin.value)
        return false;
    return strcmp(data1->Screen.ptr, data2->Screen.ptr) == 0;
}

string ICQClient::getConfig()
{
    string listRequest;
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (listRequest.length())
            listRequest += ';';
        listRequest += number((*it).type);
        listRequest += ',';
        listRequest += (*it).screen;
    }
    setListRequests(listRequest.c_str());
    string res = Client::getConfig();
    if (res.length())
        res += "\n";
    return res += save_data(icqClientData, &data);
}

string ICQClient::name()
{
    string res;
    if (m_bAIM){
        res = "AIM.";
        if (data.owner.Screen.ptr)
            res += data.owner.Screen.ptr;
        return res;
    }
    res = "ICQ.";
    res += number(data.owner.Uin.value);
    return res;
}

QWidget	*ICQClient::setupWnd()
{
    if (m_bAIM)
        return new AIMConfig(NULL, this, true);
    return new ICQConfig(NULL, this, true);
}

static const char aim_server[] = "login.oscar.aol.com";
static const char icq_server[] = "login.icq.com";

const char *ICQClient::getServer() const
{
    if (data.Server.ptr && *data.Server.ptr)
        return data.Server.ptr;
    return m_bAIM ? aim_server : icq_server;
}

void ICQClient::setServer(const char *server)
{
    if (server && !strcmp(server, m_bAIM ? aim_server : icq_server))
        server = NULL;
    set_str(&data.Server.ptr, server);
}

void ICQClient::setUin(unsigned long uin)
{
    data.owner.Uin.value = uin;
}

void ICQClient::setScreen(const char *screen)
{
    set_str(&data.owner.Screen.ptr, screen);
}

unsigned long ICQClient::getUin()
{
    return data.owner.Uin.value;
}

bool ICQClient::isMyData(clientData *&_data, Contact *&contact)
{
    if (_data->Sign.value != ICQ_SIGN)
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    if (m_bAIM){
        if (data->Screen.ptr && this->data.owner.Screen.ptr &&
                (QString(data->Screen.ptr).lower() == QString(this->data.owner.Screen.ptr).lower()))
            return false;
    }else{
        if (data->Uin.value == this->data.owner.Uin.value)
            return false;
    }
    ICQUserData *my_data = findContact(screen(data).c_str(), NULL, false, contact);
    if (my_data){
        data = my_data;
        string s;
        s = contact->getName().local8Bit();
    }else{
        contact = NULL;
    }
    return true;
}

bool ICQClient::createData(clientData *&_data, Contact *contact)
{
    ICQUserData *data = (ICQUserData*)_data;
    ICQUserData *new_data = (ICQUserData*)(contact->clientData.createData(this));
    new_data->Uin = data->Uin;
    set_str(&new_data->Screen.ptr, data->Screen.ptr);
    _data = (clientData*)new_data;
    return true;
}

OscarSocket::OscarSocket()
{
    m_nSequence    = (unsigned short)(rand() & 0x7FFF);
    m_nMsgSequence = 0;
}

OscarSocket::~OscarSocket()
{
}

void OscarSocket::connect_ready()
{
    socket()->readBuffer.init(6);
    socket()->readBuffer.packetStart();
    m_bHeader = true;
}

void ICQClient::connect_ready()
{
    if (m_listener == NULL){
        m_listener = new ICQListener(this);
        m_listener->bind(getMinPort(), getMaxPort(), NULL);
    }
    OscarSocket::connect_ready();
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
    if (m_bAIM){
        if (status == STATUS_ONLINE){
            if (m_status != STATUS_ONLINE){
                m_status = STATUS_ONLINE;
                setAwayMessage(NULL);
                Event e(EventClientChanged, this);
                e.process();
            }
        }else{
            m_status = STATUS_AWAY;

            ar_request req;
            req.bDirect = true;
            arRequests.push_back(req);

            ARRequest ar;
            ar.contact  = NULL;
            ar.param    = &arRequests.back();
            ar.receiver = this;
            ar.status   = status;
            Event eAR(EventARRequest, &ar);
            eAR.process();
            Event e(EventClientChanged, this);
            e.process();
        }
        return;
    }
    if (status != m_status){
        m_status = status;
        sendStatus();
        Event e(EventClientChanged, this);
        e.process();
    }
}

ClientSocket *ICQClient::socket()
{
    return m_socket;
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
            if ((data->Status.value != ICQ_STATUS_OFFLINE) || data->bInvisible.value){
                setOffline(data);
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(dataName(data).c_str());
                m.setStatus(STATUS_OFFLINE);
                m.setFlags(MESSAGE_RECEIVED);
                Event e(EventMessageReceived, &m);
                e.process();
            }
        }
    }
    for (list<Message*>::iterator itm = m_acceptMsg.begin(); itm != m_acceptMsg.end(); ++itm){
        Event e(EventMessageDeleted, *itm);
        e.process();
        delete *itm;
    }
    m_acceptMsg.clear();
    m_bRosters = false;
    m_nMsgSequence = 0;
    m_bServerReady = false;
    m_bIdleTime = false;
    m_cookie.init(0);
    m_advCounter = 0;
    m_nUpdates = 0;
    m_nSendTimeout = 1;
    m_info_req.clear();
    while (!m_services.empty()){
        ServiceSocket *s = m_services.front();
        delete s;
    }

    if (m_listener){

        delete m_listener;

        m_listener = NULL;

    }
}

const char *icq_error_codes[] = {I18N_NOOP("Unknown error"),
                                 I18N_NOOP("Invalid SNAC header"),
                                 I18N_NOOP("Server rate limit exceeded"),
                                 I18N_NOOP("Client rate limit exceeded"),
                                 I18N_NOOP("Recipient is not logged in"),
                                 I18N_NOOP("Requested service unavailable"),
                                 I18N_NOOP("Requested service not defined"),
                                 I18N_NOOP("We sent an obsolete SNAC"),
                                 I18N_NOOP("Not supported by server"),
                                 I18N_NOOP("Not supported by client"),
                                 I18N_NOOP("Refused by client"),
                                 I18N_NOOP("Reply too big"),
                                 I18N_NOOP("Responses lost"),
                                 I18N_NOOP("Request denied"),
                                 I18N_NOOP("Incorrect SNAC format"),
                                 I18N_NOOP("Insufficient rights"),
                                 I18N_NOOP("Recipient blocked"),
                                 I18N_NOOP("Sender too evil"),
                                 I18N_NOOP("Receiver too evil"),
                                 I18N_NOOP("User temporarily unavailable"),
                                 I18N_NOOP("No match"),
                                 I18N_NOOP("List overflow"),
                                 I18N_NOOP("Request ambiguous"),
                                 I18N_NOOP("Server queue full"),
                                 I18N_NOOP("Not while on AOL")};

const char* ICQClient::error_message(unsigned short error)
{
    if ((error < 1) || (error > 18)) {
        error = 0;
    }
    return icq_error_codes[error];
}

void OscarSocket::packet_ready()
{
    if (m_bHeader){
        char c;
        socket()->readBuffer >> c;
        if (c != 0x2A){
            log(L_ERROR, "Server send bad packet start code: %02X", c);
            socket()->error_state(I18N_NOOP("Protocol error"));
            return;
        }
        socket()->readBuffer >> m_nChannel;
        unsigned short sequence, size;
        socket()->readBuffer >> sequence >> size;
        m_bHeader = false;
        if (size){
            socket()->readBuffer.add(size);
            return;
        }
    }
    packet();
}

void ICQClient::packet_ready()
{
    OscarSocket::packet_ready();
}

void ICQClient::packet()
{
    ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->OscarPacket);
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
            if ((flags & 0x8000)) {	// some unknown data before real snac data
                // just read the length and forget it ;-)
                unsigned short unknown_length = 0;
                m_socket->readBuffer >> unknown_length;
                m_socket->readBuffer.incReadPos(unknown_length);
            }
            // now just take a look at the type because 0x0001 == error
            // in all families
            if (type == 0x0001) {
                unsigned short err_code;
                m_socket->readBuffer >> err_code;
                log(L_DEBUG,"Error! family: %u reason: %s",fam,error_message(err_code));
                // now decrease for icqicmb & icqvarious
                m_socket->readBuffer.decReadPos(sizeof(unsigned short));
            }
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

void OscarSocket::flap(char channel)
{
    m_nSequence++;
    socket()->writeBuffer.packetStart();
    socket()->writeBuffer
    << (char)0x2A
    << channel
    << m_nSequence
    << 0;
}

void OscarSocket::snac(unsigned short fam, unsigned short type, bool msgId, bool bType)
{
    flap(ICQ_CHNxDATA);
    socket()->writeBuffer
    << fam
    << type
    << 0x0000
    << (msgId ? ++m_nMsgSequence : 0x0000)
    << (bType ? type : (unsigned short)0);
}

void OscarSocket::sendPacket()
{
    Buffer &writeBuffer = socket()->writeBuffer;
    char *packet = writeBuffer.data(writeBuffer.packetStartPos());
    unsigned size = writeBuffer.size() - writeBuffer.packetStartPos() - 6;
    packet[4] = (char)((size >> 8) & 0xFF);
    packet[5] = (char)(size & 0xFF);
    log_packet(socket()->writeBuffer, true, ICQPlugin::icq_plugin->OscarPacket);
    socket()->write();
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
    for (j = 0; j < 8; j++){
        if (p[j] == 0) break;
        char c = (char)(p[j] ^ xor_table[j]);
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
    if (data.owner.WebAware.bValue)
        status |= ICQ_STATUS_FxWEBxPRESENCE;
    if (getHideIP()){
        status |= ICQ_STATUS_FxHIDExIP | ICQ_STATUS_FxDIRECTxAUTH;
    }else{
        switch (getDirectMode()){
        case 1:
            status |= ICQ_STATUS_FxDIRECTxLISTED;
            break;
        case 2:
            status |= ICQ_STATUS_FxDIRECTxAUTH;
            break;
        }
    }
    if (m_bBirthday)
        status |= ICQ_STATUS_FxBIRTHDAY;
    if (getInvisible())
        status |= ICQ_STATUS_FxPRIVATE;
    return status;
}

ICQUserData *ICQClient::findContact(const char *screen, const char *alias, bool bCreate, Contact *&contact, Group *grp)
{
    string s;
    for (const char *p = screen; *p; p++)
        s += (char)tolower(*p);

    ContactList::ContactIterator it;
    ICQUserData *data;
    unsigned long uin = atol(screen);

    while ((contact = ++it) != NULL){
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (uin && (data->Uin.value != uin))
                continue;
            if ((uin == 0) &&
                    ((data->Screen.ptr == NULL) || (s != data->Screen.ptr)))
                continue;
            bool bChanged = false;
            if (alias){
                if (*alias){
                    QString name = QString::fromUtf8(alias);
                    bChanged = contact->setName(name);
                }
                set_str(&data->Alias.ptr, alias);
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
                if (uin && (data->Uin.value != uin))
                    continue;
                if ((uin == 0) &&
                        ((data->Screen.ptr == NULL) || (s != data->Screen.ptr)))
                    continue;
                data = (ICQUserData*)(contact->clientData.createData(this));
                data->Uin.value = uin;
                if (uin == 0)
                    set_str(&data->Screen.ptr, s.c_str());
                bool bChanged = false;
                if (alias){
                    if (*alias){
                        QString name = QString::fromUtf8(alias);
                        bChanged = contact->setName(name);
                    }
                    set_str(&data->Alias.ptr, alias);
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
                data->Uin.value = uin;
                if (uin == 0)
                    set_str(&data->Screen.ptr, screen);
                set_str(&data->Alias.ptr, alias);
                Event e(EventContactChanged, contact);
                e.process();
                return data;
            }
        }
    }
    contact = getContacts()->contact(0, true);
    data = (ICQUserData*)(contact->clientData.createData(this));
    data->Uin.value = uin;
    if (uin == 0)
        set_str(&data->Screen.ptr, s.c_str());
    QString name;
    if (alias && *alias){
        name = QString::fromUtf8(alias);
    }else if (uin){
        name = QString::number(uin);
    }else{
        name = screen;
    }
    set_str(&data->Alias.ptr, alias);
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
        if (data && (data->IcqID.value == id)){
            if (alias)
                set_str(&data->Alias.ptr, alias);
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
            data->IcqID.value = id;
            set_str(&data->Alias.ptr, alias);
            return data;
        }
    }
    grp = getContacts()->group(0, true);
    grp->setName(QString::fromUtf8(alias));
    data = (ICQUserData*)(grp->clientData.createData(this));
    data->IcqID.value = id;
    set_str(&data->Alias.ptr, alias);
    Event e(EventGroupChanged, grp);
    e.process();
    return data;
}

void ICQClient::setOffline(ICQUserData *data)
{
    string name = dataName(data);
    for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ){
        Message *msg = *it;
        if (msg->client() && (name == msg->client())){
            Event e(EventMessageDeleted, msg);
            e.process();
            delete msg;
            m_acceptMsg.erase(it);
            it = m_acceptMsg.begin();
        }
        ++it;
    }
    if (data->Direct.ptr){
        delete (QObject*)data->Direct.ptr;
        data->Direct.ptr = NULL;
    }
    if (data->DirectPluginInfo.ptr){
        delete (QObject*)data->DirectPluginInfo.ptr;
        data->DirectPluginInfo.ptr = NULL;
    }
    if (data->DirectPluginStatus.ptr){
        delete (QObject*)data->DirectPluginStatus.ptr;
        data->DirectPluginStatus.ptr = NULL;
    }
    data->bNoDirect.bValue = false;
    data->Status.value = ICQ_STATUS_OFFLINE;
    data->Class.value  = 0;
    data->bTyping.bValue = false;
    data->bBadClient.bValue = false;
    data->bInvisible.bValue = false;
    time_t now;
    time(&now);
    data->StatusTime.value  = now;
    set_str(&data->AutoReply.ptr, NULL);
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

void ICQClient::contactInfo(void *_data, unsigned long &curStatus, unsigned &style, const char *&statusIcon, string *icons)
{
    ICQUserData *data = (ICQUserData*)(_data);
    unsigned status = STATUS_ONLINE;
    unsigned s = data->Status.value;
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
    if (data->Uin.value){
        if ((iconStatus == STATUS_ONLINE) && (s & ICQ_STATUS_FxPRIVATE)){
            dicon = "ICQ_invisible";
        }else{
            const CommandDef *def = ICQProtocol::_statusList();
            for (; def->text; def++){
                if (def->id == iconStatus){
                    dicon = def->icon;
                    break;
                }
            }
        }
    }else{
        if (status == STATUS_OFFLINE){
            dicon = "AIM_offline";
        }else{
            status = STATUS_ONLINE;
            dicon = "AIM_online";
            if (data->Class.value & CLASS_AWAY){
                status = STATUS_AWAY;
                dicon = "AIM_away";
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
                addIcon(icons, iconSave.c_str(), statusIcon);
        }
        statusIcon = dicon;
    }else{
        if (statusIcon){
            addIcon(icons, dicon, statusIcon);
        }else{
            statusIcon = dicon;
        }
    }
    if ((status == STATUS_OFFLINE) && data->bInvisible.bValue){
        status = STATUS_INVISIBLE;
        if (status > curStatus)
            curStatus = status;
    }
    if (icons){
        if ((iconStatus != STATUS_ONLINE) && (iconStatus != STATUS_OFFLINE) && (s & ICQ_STATUS_FxPRIVATE))
            addIcon(icons, "ICQ_invisible", statusIcon);
        if (data->bInvisible.bValue)
            addIcon(icons, "ICQ_invisible", statusIcon);
        if (data->Status.value & ICQ_STATUS_FxBIRTHDAY)
            addIcon(icons, "birthday", statusIcon);
        if (data->FollowMe.value == 1)
            addIcon(icons, "phone", statusIcon);
        if (data->FollowMe.value == 2)
            addIcon(icons, "nophone", statusIcon);
        if (status != STATUS_OFFLINE){
            if (data->SharedFiles.bValue)
                addIcon(icons, "sharedfiles", statusIcon);
            if (data->ICQPhone.value == 1)
                addIcon(icons, "icqphone", statusIcon);
            if (data->ICQPhone.value == 2)
                addIcon(icons, "icqphonebusy", statusIcon);
        }
        if (data->bTyping.bValue)
            addIcon(icons, "typing", statusIcon);
        if (data->Direct.ptr && ((DirectClient*)(data->Direct.ptr))->isSecure())
            addIcon(icons, "encrypted", statusIcon);
    }
    if (data->InvisibleId.value)
        style |= CONTACT_STRIKEOUT;
    if (data->VisibleId.value)
        style |= CONTACT_ITALIC;
    if (data->WaitAuth.bValue)
        style |= CONTACT_UNDERLINE;
}

void ICQClient::ping()
{
    if (getState() == Connected){
        bool bBirthday = false;
        if (!m_bAIM){
            int year  = data.owner.BirthYear.value;
            int month = data.owner.BirthMonth.value;
            int day   = data.owner.BirthDay.value;
            if (day && month && year){
                time_t now;
                time(&now);
                struct tm *tm = localtime(&now);
                // removed +1 on tm_day because the notification was one
                // day before the real birthday
                if (((tm->tm_mon + 1) == month) && ((tm->tm_mday) == day))
                    bBirthday = true;
            }
        }
        if (bBirthday != m_bBirthday){
            m_bBirthday = bBirthday;
            setStatus(m_status);
        }else{
            flap(ICQ_CHNxPING);
            sendPacket();
        }
        m_nSendTimeout = m_nSendTimeout / 2;
        if (m_nSendTimeout < 1)
            m_nSendTimeout = 1;
        checkListRequest();
        QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
    }
}

void ICQClient::setupContact(Contact *contact, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    QString phones;
    if (data->HomePhone.ptr){
        phones += toUnicode(trimPhone(data->HomePhone.ptr).c_str(), data);
        phones += ",Home Phone,";
        phones += number(PHONE).c_str();
    }
    if (data->HomeFax.ptr){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->HomeFax.ptr).c_str(), data);
        phones += ",Home Fax,";
        phones += number(FAX).c_str();
    }
    if (data->WorkPhone.ptr){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->WorkPhone.ptr).c_str(), data);
        phones += ",Work Phone,";
        phones += number(PHONE).c_str();
    }
    if (data->WorkFax.ptr){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->WorkFax.ptr).c_str(), data);
        phones += ",Work Fax,";
        phones += number(FAX).c_str();
    }
    if (data->PrivateCellular.ptr){
        if (phones.length())
            phones += ";";
        phones += toUnicode(trimPhone(data->PrivateCellular.ptr).c_str(), data);
        phones += ",Private Cellular,";
        phones += number(CELLULAR).c_str();
    }
    if (data->PhoneBook.ptr){
        if (phones.length())
            phones += ";";
        phones += toUnicode(data->PhoneBook.ptr, data);
    }
    string n = name();
    if (contact != getContacts()->owner()){
        contact->setPhones(phones, n.c_str());
        QString mails;
        if (data->EMail.ptr)
            mails += toUnicode(trim(data->EMail.ptr).c_str(), data);
        if (data->EMails.ptr){
            string emails = data->EMails.ptr;
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
    QString firstName = toUnicode(data->FirstName.ptr, data);
    if (firstName.length())
        contact->setFirstName(firstName, n.c_str());
    QString lastName = toUnicode(data->LastName.ptr, data);
    if (lastName.length())
        contact->setLastName(lastName, n.c_str());
    if (contact->getName().isEmpty())
        contact->setName(QString::number(data->Uin.value));
    QString nick = toUnicode(data->Nick.ptr, data);
    if (nick.isEmpty())
        nick = QString::fromUtf8(data->Alias.ptr);
    if (!nick.isEmpty()){
        QString name = QString::number(data->Uin.value);
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
        encoding = data.owner.Encoding.ptr;
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
            QString res = static_cast<ICQClient*>(it.client())->toUnicode(serverText, (ICQUserData*)data);
            return res.replace(QRegExp("\r"), "");
        }
    }
    QTextCodec *codec = _getCodec(NULL);
    QString res = codec->toUnicode(serverText, strlen(serverText));
    return res.replace(QRegExp("\r"), "");
}

QString ICQClient::toUnicode(const char *str, ICQUserData *client_data)
{
    if ((str == NULL) || (*str == 0))
        return QString();
    if (client_data != NULL)
    {
        if (client_data->Uin.value == 0)
            return QString::fromUtf8(str);
        else
        {
            QTextCodec *codec = getCodec(client_data->Encoding.ptr);
            return codec->toUnicode(str, strlen(str));
        }
    }
    else
    {
        QTextCodec *codec = getCodec(NULL);
        return codec->toUnicode(str, strlen(str));
    }
}

string ICQClient::fromUnicode(const QString &str, ICQUserData *client_data)
{
    string res;
    if (str.isEmpty())
        return res;
    QString s = str;
    s.replace(QRegExp("\r"), "");
    s.replace(QRegExp("\n"), "\r\n");
    QTextCodec *codec = getCodec(client_data ? client_data->Encoding.ptr : NULL);
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
    if (status == STATUS_INVISIBLE){
        res += "<img src=\"icon:ICQ_invisible\">";
        res += i18n("Possibly invisible");
    }else{
        res += "<img src=\"icon:";
        res += statusIcon;
        res += "\">";
        if (!strcmp(statusIcon, "ICQ_invisible")){
            res += " ";
            res += i18n("Invisible");
        }else  if (data->Uin.value){
            for (const CommandDef *cmd = ICQProtocol::_statusList(); cmd->text; cmd++){
                if (!strcmp(cmd->icon, statusIcon)){
                    res += " ";
                    statusText += i18n(cmd->text);
                    res += statusText;
                    break;
                }
            }
        }else{
            switch (status){
            case STATUS_OFFLINE:
                res += i18n("Offline");
                break;
            case STATUS_ONLINE:
                res += i18n("Online");
                break;
            default:
                res += i18n("Away");
            }
        }
    }
    res += "<br>";
    if (data->Uin.value){
        res += "UIN: <b>";
        res += number(data->Uin.value).c_str();
        res += "</b>";
    }else{
        res += "<b>";
        res += data->Screen.ptr;
        res += "</b>";
    }
    if (data->WarningLevel.value){
        res += "<br>";
        res += i18n("Warning level");
        res += ": <b>";
        res += QString::number(warnLevel((unsigned short)(data->WarningLevel.value)));
        res += "% </b></br>";
    }
    if (data->Status.value == ICQ_STATUS_OFFLINE){
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
        if (data->Status.value & (ICQ_STATUS_AWAY | ICQ_STATUS_NA)){
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
    if ((data->RealIP.ptr) && ((data->IP.ptr == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        res += "<br>";
        res += formatAddr(data->RealIP, data->Port.value);
    }
    string client_name = clientName(data);
    if (client_name.length()){
        res += "<br>";
        res += quoteString(client_name.c_str());
    }
    if (data->PictureWidth.value && data->PictureHeight.value){
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
    if (data->AutoReply.ptr && *data->AutoReply.ptr){
        res += "<br><br>";
        res += quoteString(toUnicode(data->AutoReply.ptr, data));
    }
    return res;
}

unsigned ICQClient::warnLevel(unsigned short level)
{
    level = (unsigned short)((level + 5) / 10);
    if (level > 100)
        level = 100;
    return level;
}

bool ICQClient::hasCap(ICQUserData *data, int n)
{
    return (data->Caps.value & (1 << n)) != 0;
}

static string verString(unsigned ver)
{
    string res;
    if (ver == 0) return res;
    unsigned char v[4];
    v[0] = (unsigned char)((ver >> 24) & 0xFF);
    v[1] = (unsigned char)((ver >> 16) & 0xFF);
    v[2] = (unsigned char)((ver >> 8) & 0xFF);
    v[3] = (unsigned char)(ver & 0xFF);
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
    if (data->Version.value)
    {
        snprintf(b, sizeof(b), "v%lu ", data->Version.value);
        res = b;
    }
    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN))
    {
        res += "Trillian";
        return res;
    }

    if (hasCap(data, CAP_SIMOLD))
    {
        int hiVersion = (data->Build.value >> 6) - 1;
        unsigned loVersion = data->Build.value & 0x1F;
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
        unsigned ver1 = (data->Build.value >> 24) & 0xFF;
        unsigned ver2 = (data->Build.value >> 16) & 0xFF;
        unsigned ver3 = (data->Build.value >> 8) & 0xFF;
        if (ver3){
            snprintf(b, sizeof(b), "SIM %u.%u.%u", ver1, ver2, ver3);
        }else{
            snprintf(b, sizeof(b), "SIM %u.%u", ver1, ver2);
        }
        res += b;
        if (data->Build.value & 0x80)
            res += "/win32";

        if (data->Build.value & 0x40)

            res += "/MacOS X";
        return res;
    }

    if (hasCap(data, CAP_LICQ))
    {
        unsigned ver1 = (data->Build.value >> 24) & 0xFF;
        unsigned ver2 = (data->Build.value >> 16) & 0xFF;
        unsigned ver3 = (data->Build.value >> 8) & 0xFF;
        if (ver3){
            snprintf(b, sizeof(b), "Licq %u.%u.%u", ver1, ver2, ver3);
        }else{
            snprintf(b, sizeof(b), "Licq %u.%u", ver1, ver2);
        }
        res += b;
        if (data->Build.value & 0xFF)
            res += "/SSL";
        return res;
    }
    if (hasCap(data, CAP_TYPING))
    {
        switch (data->Version.value){
        case 10:
            res += "ICQ 2003b";
            break;
        case 9:
            res += "ICQ Lite";
            break;
        default:
            res += "ICQ2go";
        }
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
    if (hasCap(data, CAP_AIM_BUDDYCON)){
        res += "gaim";
        return res;
    }
    if ((data->InfoUpdateTime.value & 0xFF7F0000L) == 0x7D000000L){
        unsigned ver = data->InfoUpdateTime.value & 0xFFFF;
        if (ver % 10){
            snprintf(b, sizeof(b), "Licq %u.%u.%u", ver / 1000, (ver / 10) % 100, ver % 10);
        }else{
            snprintf(b, sizeof(b), "Licq %u.%u", ver / 1000, (ver / 10) % 100);
        }
        res += b;
        if (data->InfoUpdateTime.value & 0x00800000L)
            res += "/SSL";
        return res;
    }
    switch (data->InfoUpdateTime.value){
    case 0xFFFFFFFFL:
        if ((data->PluginStatusTime.value == 0xFFFFFFFFL) && (data->PluginInfoTime.value == 0xFFFFFFFFL)){
            res += "GAIM";
            return res;
        }
        res += "MIRANDA";
        res += verString(data->PluginInfoTime.value & 0xFFFFFF);
        if (data->PluginInfoTime.value & 0x80000000)
            res += " alpha";
        return res;
    case 0xFFFFFF8FL:
        res += "StrICQ";
        res += verString(data->PluginInfoTime.value & 0xFFFFFF);
        return res;
    case 0xFFFFFF42L:
        res += "mICQ";
        return res;
    case 0xFFFFFFBEL:
        res += "alicq";
        res += verString(data->PluginInfoTime.value & 0xFFFF);
        return res;
    case 0xFFFFFF7FL:
        res += "&RQ";
        res += verString(data->PluginInfoTime.value & 0xFFFF);
        return res;
    case 0xFFFFFFABL:
        res += "YSM";
        res += verString(data->PluginInfoTime.value & 0xFFFF);
        return res;
    case 0x04031980L:
        snprintf(b, sizeof(b), "vICQ 0.43.%lu.%lu", data->PluginInfoTime.value & 0xffff, data->PluginInfoTime.value & (0x7fff0000) >> 16);
        res += b;
        return res;
    case 0x3AA773EEL:
        if ((data->PluginStatusTime.value == 0x3AA66380L) && (data->PluginInfoTime.value == 0x3A877A42L))
        {
            res += "libicq2000";
            return res;
        }
        break;
    }

    if (data->InfoUpdateTime.value &&
            (data->InfoUpdateTime.value == data->PluginStatusTime.value) &&
            (data->PluginStatusTime.value == data->PluginInfoTime.value) && (data->Caps.value == 0)){
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
    if (hasCap(data, CAP_SRV_RELAY) && hasCap(data, CAP_DIRECT)){
        res += "ICQ 2001b";
        return res;
    }
    if ((data->Version.value == 7) && hasCap(data, CAP_RTF)){
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

static CommandDef aimWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "AIM_online",
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

static CommandDef aimConfigWnd[] =
    {
        {
            MAIN_INFO,
            "",
            "AIM_online",
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

CommandDef *ICQClient::infoWindows(Contact*, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    CommandDef *def = data->Uin.value ? icqWnd : aimWnd;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    if (data->Uin.value){
        name += QString::number(data->Uin.value);
    }else{
        name += data->Screen.ptr;
    }
    def->text_wrk = strdup(name.utf8());
    return def;
}

CommandDef *ICQClient::configWindows()
{
    CommandDef *def = icqConfigWnd;
    QString name = i18n(protocol()->description()->text);
    name += " ";
    if (m_bAIM){
        name += QString::fromUtf8(data.owner.Screen.ptr);
        def = aimConfigWnd;
    }else{
        name += QString::number(data.owner.Uin.value);
    }
    def->text_wrk = strdup(name.utf8());
    return def;
}

QWidget *ICQClient::infoWindow(QWidget *parent, Contact*, void *_data, unsigned id)
{
    ICQUserData *data = (ICQUserData*)_data;
    switch (id){
    case MAIN_INFO:
        if (data->Uin.value)
            return new ICQInfo(parent, data, this);
        return new AIMInfo(parent, data, this);
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
        if (m_bAIM)
            return new AIMInfo(parent, NULL, this);
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
        if (m_bAIM)
            return new AIMConfig(parent, this, false);
        return new ICQConfig(parent, this, false);
    case SECURITY:
        return new ICQSecure(parent, this);
    }
    return NULL;
}

QWidget *ICQClient::searchWindow()
{
    if (m_bAIM)
        return new AIMSearch(this);
    return new ICQSearch(this);
}

void ICQClient::updateInfo(Contact *contact, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    if (getState() != Connected){
        Client::updateInfo(contact, _data);
        return;
    }
    if (data == NULL)
        data = &this->data.owner;
    if (data->Uin.value){
        addFullInfoRequest(data->Uin.value, false);
        addPluginInfoRequest(data->Uin.value, PLUGIN_QUERYxINFO);
        addPluginInfoRequest(data->Uin.value, PLUGIN_QUERYxSTATUS);
        addPluginInfoRequest(data->Uin.value, PLUGIN_AR);
    }else{
        fetchProfile(data);
    }
}

void *ICQClient::processEvent(Event *e)
{
    if (e->type() == EventAddContact){
        addContact *ac = (addContact*)(e->param());
        if (ac->proto && !strcmp(protocol()->description()->text, ac->proto)){
            Group *grp = getContacts()->group(ac->group);
            Contact *contact;
            findContact(ac->addr, ac->nick, true, contact, grp);
            return contact;
        }
        return NULL;
    }
    if (e->type() == EventDeleteContact){
        char *addr = (char*)(e->param());
        ContactList::ContactIterator it;
        Contact *contact;
        while ((contact = ++it) != NULL){
            ICQUserData *data;
            ClientDataIterator itc(contact->clientData, this);
            while ((data = (ICQUserData*)(++itc)) != NULL){
                if (!strcmp(data->Screen.ptr, addr)){
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
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (data->RealIP.ptr)
                return (void*)(data->RealIP.ptr);
            if (data->IP.ptr)
                return (void*)(data->IP.ptr);
        }
        return NULL;
    }
    if (e->type() == EventMessageAccept){
        messageAccept *ma = (messageAccept*)(e->param());
        for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
            if ((*it)->id() == ma->msg->id()){
                Message *msg = *it;
                m_acceptMsg.erase(it);
                accept(msg, ma->dir, ma->overwrite);
                return msg;
            }
        }
        return NULL;
    }
    if (e->type() == EventMessageDecline){
        messageDecline *md = (messageDecline*)(e->param());
        for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
            if ((*it)->id() == md->msg->id()){
                Message *msg = *it;
                m_acceptMsg.erase(it);
                decline(msg, md->reason);
                return msg;
            }
        }
        return NULL;
    }
    if (e->type() == EventMessageRetry){
        MsgSend *m = (MsgSend*)(e->param());
        QStringList btns;
        if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendOccupied){
            btns.append(i18n("Send &urgent"));
        }else if (m->msg->getRetryCode() != static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendDND){
            return NULL;
        }
        btns.append(i18n("Send to &list"));
        btns.append(i18n("&Cancel"));
        QString err;
        const char *err_str = m->msg->getError();
        if (err_str && *err_str)
            err = i18n(err_str);
        Command cmd;
        cmd->id		= CmdSend;
        cmd->param	= m->edit;
        Event eWidget(EventCommandWidget, cmd);
        QWidget *msgWidget = (QWidget*)(eWidget.process());
        if (msgWidget == NULL)
            msgWidget = m->edit;
        BalloonMsg *msg = new BalloonMsg(m, err, btns, msgWidget, NULL, false);
        connect(msg, SIGNAL(action(int, void*)), this, SLOT(retry(int, void*)));
        msg->show();
        return e->param();
    }
    if (e->type() == EventTemplateExpanded){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        list<ar_request>::iterator it;
        for (it = arRequests.begin(); it != arRequests.end(); ++it)
            if (&(*it) == t->param)
                break;
        if (it == arRequests.end())
            return NULL;
        if (m_bAIM){
            if ((getState() == Connected) && (m_status == STATUS_AWAY)){
                if ((*it).bDirect){
                    setAwayMessage(t->tmpl.utf8());
                }else{
                    sendCapability(t->tmpl.utf8());
                    sendICMB(1, 11);
                    sendICMB(0, 11);
                    processListRequest();
                    fetchProfiles();
                }
            }
            return e->param();
        }
        ar_request ar = (*it);
        if (ar.bDirect){
            Contact *contact;
            ICQUserData *data = findContact(ar.screen.c_str(), NULL, false, contact);
            if (data && data->Direct.ptr){
                QString answer;
                if (data->Version.value >= 10){
                    answer = t->tmpl.utf8();
                }else{
                    answer = fromUnicode(t->tmpl, data).c_str();
                }
                ((DirectClient*)(data->Direct.ptr))->sendAck((unsigned short)(ar.id.id_l), ar.type, ar.flags, answer);
            }
        }else{
            Buffer copy;
            string response;
            response = t->tmpl.utf8();
            sendAutoReply(ar.screen.c_str(), ar.id, plugins[PLUGIN_NULL],
                          ar.id1, ar.id2, ar.type, (char)(ar.ack), 0, response.c_str(), 0, copy);
        }
        arRequests.erase(it);
        return e->param();
    }
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (getState() == Connected){
            if (!m_bAIM)
                addBuddy(contact);
            if (contact == getContacts()->owner()){
                time_t now;
                time(&now);
                if (getContacts()->owner()->getPhones() != QString::fromUtf8(data.owner.PhoneBook.ptr)){
                    set_str(&data.owner.PhoneBook.ptr, getContacts()->owner()->getPhones().utf8());
                    data.owner.PluginInfoTime.value = now;
                    sendPluginInfoUpdate(PLUGIN_PHONEBOOK);
                }
                if (getPicture() != QString::fromUtf8(data.owner.Picture.ptr)){
                    set_str(&data.owner.Picture.ptr, getPicture().utf8());
                    data.owner.PluginInfoTime.value = now;
                    sendPluginInfoUpdate(PLUGIN_PICTURE);
                }
                if (getContacts()->owner()->getPhoneStatus() != data.owner.FollowMe.value){
                    data.owner.FollowMe.value = getContacts()->owner()->getPhoneStatus();
                    data.owner.PluginStatusTime.value = now;
                    sendPluginStatusUpdate(PLUGIN_FOLLOWME, data.owner.FollowMe.value);
                }
                return NULL;
            }
            ICQUserData *data;
            ClientDataIterator it(contact->clientData, this);
            while ((data = (ICQUserData*)(++it)) != NULL){
                if (data->Uin.value || data->ProfileFetch.bValue)
                    continue;
                fetchProfile(data);
            }
        }
        addContactRequest(contact);
    }
    if (e->type() == EventContactDeleted){
        Contact *contact =(Contact*)(e->param());
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (data->IcqID.value == 0)
                continue;
            list<ListRequest>::iterator it;
            for (it = listRequests.begin(); it != listRequests.end(); it++){
                if ((*it).type != LIST_USER_CHANGED)
                    continue;
                if ((*it).screen == screen(data))
                    break;
            }
            if (it != listRequests.end())
                listRequests.erase(it);
            ListRequest lr;
            lr.type = LIST_USER_DELETED;
            lr.screen = screen(data);
            lr.icq_id = (unsigned short)(data->IcqID.value);
            lr.grp_id = (unsigned short)(data->GrpId.value);
            lr.visible_id   = (unsigned short)(data->ContactVisibleId.value);
            lr.invisible_id = (unsigned short)(data->ContactInvisibleId.value);
            lr.ignore_id    = (unsigned short)(data->IgnoreId.value);
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
                lr.type   = LIST_GROUP_DELETED;
                lr.icq_id = (unsigned short)(data->IcqID.value);
                listRequests.push_back(lr);
                processListRequest();
            }
        }
    }
    if (e->type() == EventMessageCancel){
        Message *msg = (Message*)(e->param());
        list<Message*>::iterator it;
        for (it = m_processMsg.begin(); it != m_processMsg.end(); ++it)
            if (*it == msg)
                break;
        if (it != m_processMsg.end()){
            m_processMsg.erase(it);
            delete msg;
            return msg;
        }
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
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact){
                ICQUserData *data;
                ClientDataIterator it(contact->clientData, this);
                while ((data = (ICQUserData*)(++it)) != NULL){
                    if (data->Direct.ptr && ((DirectClient*)(data->Direct.ptr))->cancelMessage(msg))
                        return msg;
                }
            }
            if (m_send.msg == msg){
                m_send.msg = NULL;
                m_send.screen = "";
                send(true);
                return msg;
            }
            list<SendMsg>::iterator it;
            for (it = sendQueue.begin(); it != sendQueue.end(); ++it){
                if ((*it).msg == msg){
                    sendQueue.erase(it);
                    delete msg;
                    return msg;
                }
            }
            for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                if ((*it).msg == msg){
                    replyQueue.erase(it);
                    delete msg;
                    return msg;
                }
            }
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->menu_id == MenuEncoding){
            ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
            if (cmd->id == CmdChangeEncoding){
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
                for (enc = encodings; enc->language; enc++){
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
                if (data->Encoding.ptr == NULL)
                    cmds[nEncoding].flags = COMMAND_CHECKED;
                nEncoding++;
                main.sort();
                for (it = main.begin(); it != main.end(); ++it){
                    QString str = *it;
                    int n = str.find('(');
                    str = str.mid(n + 1);
                    n = str.find(')');
                    str = str.left(n);
                    if (data->Encoding.ptr && !strcmp(data->Encoding.ptr, str.latin1()))
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
                    if (data->Encoding.ptr && !strcmp(data->Encoding.ptr, str.latin1()))
                        cmds[nEncoding].flags = COMMAND_CHECKED;
                    cmds[nEncoding].id = nEncoding;
                    cmds[nEncoding].text = "_";
                    cmds[nEncoding].text_wrk = strdup((*it).utf8());
                    nEncoding++;
                }
                return e->param();
            }
            if (cmd->id == CmdAllEncodings){
                cmd->flags &= ~COMMAND_CHECKED;
                if (plugin->getShowAllEncodings())
                    cmd->flags |= COMMAND_CHECKED;
                return e->param();
            }
        }
        if (cmd->id == CmdCheckInvisibleAll){
            cmd->flags &= ~COMMAND_CHECKED;
            if ((getState() != Connected) || m_bAIM)
                return NULL;
            Contact *contact;
            ContactList::ContactIterator it;
            bool bICQ = false;
            while ((contact = ++it) != NULL){
                ICQUserData *data;
                ClientDataIterator itc(contact->clientData, this);
                while ((data = (ICQUserData*)(++itc)) != NULL){
                    if (data->Uin.value == 0)
                        continue;
                    if (data->Status.value != ICQ_STATUS_OFFLINE)
                        continue;
                    bICQ = true;
                    if (data->bInvisible.bValue){
                        cmd->popup_id = MenuCheckInvisible;
                        return e->param();
                    }
                }
            }
            if (bICQ){
                cmd->popup_id = 0;
                return e->param();
            }
        }
        if ((cmd->bar_id == ToolBarContainer) || (cmd->bar_id == BarHistory)){
            if (cmd->id == CmdChangeEncoding){
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
            if (cmd->id == CmdVisibleList){
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
                    if (data->VisibleId.value)
                        cmd->flags |= COMMAND_CHECKED;
                }
                return bOK ? e->param() : NULL;
            }
            if (cmd->id == CmdInvisibleList){
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
                    if (data->InvisibleId.value)
                        cmd->flags |= COMMAND_CHECKED;
                }
                return bOK ? e->param() : NULL;
            }
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        ICQPlugin *plugin = static_cast<ICQPlugin*>(protocol()->plugin());
        if (cmd->menu_id == MenuEncoding){
            if (cmd->id == CmdAllEncodings){
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
                for (enc = encodings; enc->language; enc++){
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
                if (set_str(&data->Encoding.ptr, codec))
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
        if ((cmd->id == CmdCheckInvisible) ||
                (cmd->id == CmdCheckInvisibleAll)){
            if (getState() == Connected){
                ContactList::ContactIterator it;
                Contact *contact;
                while ((contact = ++it) != NULL){
                    if (contact->getIgnore())
                        continue;
                    ClientDataIterator itd(contact->clientData, this);
                    ICQUserData *data;
                    while ((data = (ICQUserData*)(++itd)) != NULL){
                        if (data->Uin.value == 0)
                            continue;
                        if (data->Status.value != ICQ_STATUS_OFFLINE)
                            continue;
                        if ((cmd->id == CmdCheckInvisible) && (data->bInvisible.bValue == 0))
                            continue;
                        Message *m = new Message(MessageCheckInvisible);
                        m->setContact(contact->id());
                        m->setClient(dataName(data).c_str());
                        m->setFlags(MESSAGE_NOHISTORY);
                        if (!send(m, data))
                            delete m;
                    }
                }
            }
        }
        if (cmd->menu_id == MenuContactGroup){
            if (cmd->id == CmdVisibleList){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                ICQUserData *data;
                ClientDataIterator it(contact->clientData, this);
                bool bChanged = false;
                while ((data = (ICQUserData*)(++it)) != NULL){
                    data->VisibleId.value = (cmd->flags & COMMAND_CHECKED) ? getListId() : 0;
                    bChanged = true;
                }
                if (bChanged){
                    Event eContact(EventContactChanged, contact);
                    eContact.process();
                }
                return e->param();
            }
            if (cmd->id == CmdInvisibleList){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                ICQUserData *data;
                ClientDataIterator it(contact->clientData, this);
                bool bChanged = false;
                while ((data = (ICQUserData*)(++it)) != NULL){
                    data->InvisibleId.value = (cmd->flags & COMMAND_CHECKED) ? getListId() : 0;
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
        if ((proto != "icq") && (proto != "aim"))
            return NULL;
        url = url.substr(proto.length() + 1);
        while (url[0] == '/')
            url = url.substr(1);
        QString s = unquoteString(QString(url.c_str()));
        QString screen = getToken(s, ',');
        if (!screen.isEmpty()){
            Contact *contact;
            findContact(screen.latin1(), s.utf8(), true, contact);
            Command cmd;
            cmd->id		 = MessageGeneric;
            cmd->menu_id = MenuMessage;
            cmd->param	 = (void*)(contact->id());
            Event eCmd(EventCommandExec, cmd);
            eCmd.process();
            return e->param();
        }
    }
    if (e->type() == EventOpenMessage){
        Message *msg = (Message*)(e->param());
        if ((msg->type() != MessageOpenSecure) &&
                (msg->type() != MessageCloseSecure) &&
                (msg->type() != MessageCheckInvisible) &&
                (msg->type() != MessageWarning))
            return NULL;
        const char *client = msg->client();
        if (client && (*client == 0))
            client = NULL;
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            return NULL;
        ICQUserData *data = NULL;
        ClientDataIterator it(contact->clientData, this);
        if (client){
            while ((data = (ICQUserData*)(++it)) != NULL){
                if (dataName(data) == client)
                    break;
            }
        }else{
            while ((data = (ICQUserData*)(++it)) != NULL)
                break;
        }
        if (data == NULL)
            return NULL;
        if (msg->type() == MessageCheckInvisible){
            Message *m = new Message(MessageCheckInvisible);
            m->setContact(msg->contact());
            m->setClient(msg->client());
            if (!send(m, data)){
                delete m;
                return NULL;
            }
            return e->param();
        }
        if (msg->type() == MessageOpenSecure){
            SecureDlg *dlg = NULL;
            QWidgetList  *list = QApplication::topLevelWidgets();
            QWidgetListIt it(*list);
            QWidget * w;
            while ((w=it.current()) != NULL) {
                ++it;
                if (!w->inherits("SecureDlg"))
                    continue;
                dlg = static_cast<SecureDlg*>(w);
                if ((dlg->m_client == this) &&
                        (dlg->m_contact == contact->id()) &&
                        (dlg->m_data == data))
                    break;
                dlg = NULL;
            }
            delete list;
            if (dlg == NULL)
                dlg = new SecureDlg(this, contact->id(), data);
            raiseWindow(dlg);
            return e->param();
        }
        if (msg->type() == MessageWarning){
            if (data && (m_bAIM || (data->Uin.value == 0))){
                WarnDlg *dlg = new WarnDlg(NULL, data, this);
                raiseWindow(dlg);
                return e->param();
            }
            return NULL;
        }
        if (data->Direct.ptr && ((DirectClient*)(data->Direct.ptr))->isSecure()){
            Message *m = new Message(MessageCloseSecure);
            m->setContact(msg->contact());
            m->setClient(msg->client());
            m->setFlags(MESSAGE_NOHISTORY);
            if (!((DirectClient*)(data->Direct.ptr))->sendMessage(m))
                delete m;
            return e->param();
        }
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
        smsQueue.push_back(s);
        processSMSQueue();
        return true;
    case MessageAuthRequest:
        if (data && data->WaitAuth.bValue)
            return sendAuthRequest(msg, data);
        return false;
    case MessageAuthGranted:
        if (data && data->WantAuth.bValue)
            return sendAuthGranted(msg, data);
        return false;
    case MessageAuthRefused:
        if (data && data->WantAuth.bValue)
            return sendAuthRefused(msg, data);
        return false;
    case MessageCheckInvisible:
        return data &&
               ((data->Status.value & 0xFFFF) == ICQ_STATUS_OFFLINE) &&
               sendThruServer(msg, data);
    case MessageFile:
        if (data && ((data->Status.value & 0xFFFF) != ICQ_STATUS_OFFLINE)){
            if (data->Uin.value){
                if (data->Direct.ptr == NULL){
                    data->Direct.ptr = (char*)(new DirectClient(data, this, PLUGIN_NULL));
                    ((DirectClient*)(data->Direct.ptr))->connect();
                }
                return ((DirectClient*)(data->Direct.ptr))->sendMessage(msg);
            }
            if (!hasCap(data, CAP_AIM_SENDFILE))
                return false;
            m_processMsg.push_back(msg);
            AIMFileTransfer *ft = new AIMFileTransfer(static_cast<FileMessage*>(msg), data, this);
            ft->listen();
            return true;
        }
        return false;
    case MessageTypingStart:
        if ((data == NULL) || !getTypingNotification())
            return false;
        if (getInvisible()){
            if (data->VisibleId.value == 0)
                return false;
        }else{
            if (data->InvisibleId.value)
                return false;
        }
        if (!hasCap(data, CAP_TYPING) && !hasCap(data, CAP_AIM_BUDDYCON))
            return false;
        sendMTN(screen(data).c_str(), ICQ_MTN_START);
        delete msg;
        return true;
    case MessageTypingStop:
        if (data == NULL)
            return false;
        sendMTN(screen(data).c_str(), ICQ_MTN_FINISH);
        delete msg;
        return true;
#ifdef USE_OPENSSL
    case MessageOpenSecure:
        if (data == NULL)
            return false;
        if (data && data->Direct.ptr && ((DirectClient*)(data->Direct.ptr))->isSecure())
            return false;
        if (data->Direct.ptr == NULL){
            data->Direct.ptr = (char*)(new DirectClient(data, this, PLUGIN_NULL));
            ((DirectClient*)(data->Direct.ptr))->connect();
        }
        return ((DirectClient*)(data->Direct.ptr))->sendMessage(msg);
    case MessageCloseSecure:
        if (data && data->Direct.ptr && ((DirectClient*)(data->Direct.ptr))->isSecure())
            return ((DirectClient*)(data->Direct.ptr))->sendMessage(msg);
        return false;
#endif
    case MessageWarning:
        return sendThruServer(msg, data);
    case MessageContacts:
        if ((data == NULL) || ((data->Uin.value == 0) && !hasCap(data, CAP_AIM_BUDDYLIST)))
            return false;
    }
    if (data == NULL)
        return false;
    if (data->Uin.value){
        bool bCreateDirect = false;
        if ((data->Direct.ptr == NULL) &&
                !data->bNoDirect.bValue &&
                (data->Status.value != ICQ_STATUS_OFFLINE) &&
                (get_ip(data->IP) == get_ip(this->data.owner.IP)))
            bCreateDirect = true;
        if (!bCreateDirect &&
                (msg->type() == MessageGeneric) &&
                (data->Status.value != ICQ_STATUS_OFFLINE) &&
                get_ip(data->IP) &&
                (msg->getPlainText().length() >= MAX_TYPE2_MESSAGE_SIZE))
            bCreateDirect = true;
        if ((getInvisible() && (data->VisibleId.value == 0)) ||
                (!getInvisible() && data->InvisibleId.value))
            bCreateDirect = false;
        if (bCreateDirect){
            data->Direct.ptr = (char*)(new DirectClient(data, this, PLUGIN_NULL));
            ((DirectClient*)(data->Direct.ptr))->connect();
        }
        if (data->Direct.ptr)
            return ((DirectClient*)(data->Direct.ptr))->sendMessage(msg);
    }
    return sendThruServer(msg, data);
}

bool ICQClient::canSend(unsigned type, void *_data)
{
    if (_data && (((clientData*)_data)->Sign.value != ICQ_SIGN))
        return false;
    if (getState() != Connected)
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    switch (type){
    case MessageSMS:
        return true;
    case MessageGeneric:
    case MessageUrl:
        return (data != NULL);
    case MessageContacts:
        return (data != NULL) && (data->Uin.value || hasCap(data, CAP_AIM_BUDDYLIST));
    case MessageAuthRequest:
        return data && (data->WaitAuth.bValue);
    case MessageAuthGranted:
        return data && (data->WantAuth.bValue);
    case MessageCheckInvisible:
        return data && data->Uin.value && !m_bAIM && ((data->Status.value & 0xFFFF) == ICQ_STATUS_OFFLINE);
    case MessageFile:
        return data &&
               ((data->Status.value & 0xFFFF) != ICQ_STATUS_OFFLINE) &&
               (data->Uin.value || hasCap(data, CAP_AIM_SENDFILE));
    case MessageWarning:
        return data && (m_bAIM || (data->Uin.value == 0));
#ifdef USE_OPENSSL
    case MessageOpenSecure:
        if ((data == NULL) || ((data->Status.value & 0xFFFF) == ICQ_STATUS_OFFLINE))
            return false;
        if (hasCap(data, CAP_LICQ) ||
                hasCap(data, CAP_SIM) ||
                hasCap(data, CAP_SIMOLD) ||
                ((data->InfoUpdateTime.value & 0xFF7F0000L) == 0x7D000000L)){
            if (data->Direct.ptr)
                return !((DirectClient*)(data->Direct.ptr))->isSecure();
            return get_ip(data->IP) || get_ip(data->RealIP);
        }
        return false;
    case MessageCloseSecure:
        return data && data->Direct.ptr && ((DirectClient*)(data->Direct.ptr))->isSecure();
#endif
    }
    return false;
}

string ICQClient::dataName(void *data)
{
    return dataName(screen((ICQUserData*)data).c_str());
}

string ICQClient::dataName(const char *screen)
{
    string res = name();
    res += ".";
    res += screen;
    return res;
}

string ICQClient::screen(ICQUserData *data)
{
    if (data->Uin.value == 0)
        return data->Screen.ptr ? data->Screen.ptr : "";
    return number(data->Uin.value);
}

bool ICQClient::messageReceived(Message *msg, const char *screen)
{
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    if (msg->contact() == 0){
        Contact *contact;
        ICQUserData *data = findContact(screen, NULL, false, contact);
        if (data == NULL){
            data = findContact(screen, NULL, true, contact);
            if (data == NULL){
                delete msg;
                return true;
            }
            contact->setTemporary(CONTACT_TEMP);
            Event e(EventContactChanged, contact);
            e.process();
        }
        msg->setClient(dataName(data).c_str());
        msg->setContact(contact->id());
        if (data->bTyping.bValue){
            data->bTyping.bValue = false;
            Event e(EventContactStatus, contact);
            e.process();
        }
    }
    bool bAccept = false;
    switch (msg->type()){
    case MessageICQFile:
    case MessageFile:
        bAccept = true;
        break;
    }
    if (bAccept)
        m_acceptMsg.push_back(msg);
    Event e(EventMessageReceived, msg);
    if (e.process()){
        if (bAccept){
            for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
                if ((*it) == msg){
                    m_acceptMsg.erase(it);
                    break;
                }
            }
        }
    }else{
        if (!bAccept)
            delete msg;
    }
    return !bAccept;
}

QString ICQClient::ownerName()
{
    return contactName(&data.owner);
}

QString ICQClient::contactName(void *clientData)
{
    QString res;
    ICQUserData *data = (ICQUserData*)clientData;
    res = data->Uin.value ? "ICQ: " : "AIM: ";
    if (data->Nick.ptr && *data->Nick.ptr){
        res += toUnicode(data->Nick.ptr, data);
        res += " (";
    }
    res += data->Uin.value ? QString::number(data->Uin.value) : QString(data->Screen.ptr);
    if (data->Nick.ptr && *data->Nick.ptr)
        res += ")";
    return res;
}

bool ICQClient::isSupportPlugins(ICQUserData *data)
{
    if (data->Version.value < 7)
        return false;
    switch (data->InfoUpdateTime.value){
    case 0xFFFFFF42:
    case 0xFFFFFFFF:
    case 0xFFFFFF7F:
    case 0xFFFFFFBE:
    case 0x3B75AC09:
    case 0x3AA773EE:
    case 0x3BC1252C:
    case 0x3B176B57:
    case 0x3BA76E2E:
    case 0x3C7D8CBC:
    case 0x3CFE0688:
    case 0x3BFF8C98:
        return false;
    }
    if ((data->InfoUpdateTime.value & 0xFF7F0000L) == 0x7D000000L)
        return false;
    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN))
        return false;
    return true;
}

void ICQClient::addPluginInfoRequest(unsigned long uin, unsigned plugin_index)
{
    Contact *contact;
    ICQUserData *data = findContact(number(uin).c_str(), NULL, false, contact);
    if (data && !data->bNoDirect.bValue &&
            (get_ip(data->IP) == get_ip(this->data.owner.IP)) &&
            ((getInvisible() && data->VisibleId.value) ||
             (!getInvisible() && (data->InvisibleId.value == 0)))){
        switch (plugin_index){
        case PLUGIN_AR:
            if ((data->Direct.ptr == NULL) && !getHideIP()){
                data->Direct.ptr = (char*)(new DirectClient(data, this, PLUGIN_NULL));
                ((DirectClient*)(data->Direct.ptr))->connect();
            }
            if (data->Direct.ptr){
                ((DirectClient*)(data->Direct.ptr))->addPluginInfoRequest(plugin_index);
                return;
            }
            break;
        case PLUGIN_QUERYxINFO:
        case PLUGIN_PHONEBOOK:
        case PLUGIN_PICTURE:
            if (!isSupportPlugins(data))
                return;
            if ((data->DirectPluginInfo.ptr == NULL) && !getHideIP()){
                data->DirectPluginInfo.ptr = (char*)(new DirectClient(data, this, PLUGIN_INFOxMANAGER));
                ((DirectClient*)(data->DirectPluginInfo.ptr))->connect();
            }
            if (data->DirectPluginInfo.ptr){
                ((DirectClient*)(data->DirectPluginInfo.ptr))->addPluginInfoRequest(plugin_index);
                return;
            }
            break;
        case PLUGIN_QUERYxSTATUS:
        case PLUGIN_FILESERVER:
        case PLUGIN_FOLLOWME:
        case PLUGIN_ICQPHONE:
            if (!isSupportPlugins(data))
                return;
            if ((data->DirectPluginStatus.ptr == NULL) && !getHideIP()){
                data->DirectPluginStatus.ptr = (char*)(new DirectClient(data, this, PLUGIN_STATUSxMANAGER));
                ((DirectClient*)(data->DirectPluginStatus.ptr))->connect();
            }
            if (data->DirectPluginStatus.ptr){
                ((DirectClient*)(data->DirectPluginStatus.ptr))->addPluginInfoRequest(plugin_index);
                return;
            }
            break;
        }
    }
    list<SendMsg>::iterator it;
    for (it = sendQueue.begin(); it != sendQueue.end(); ++it){
        SendMsg &s = *it;
        if (((unsigned)atol(s.screen.c_str()) == uin) && (s.flags == plugin_index) && (s.msg == NULL))
            break;
    }
    if (it != sendQueue.end())
        return;
    SendMsg s;
    s.screen = number(uin);
    s.flags  = plugin_index;
    sendQueue.push_back(s);
    send(true);
}

void ICQClient::randomChatInfo(unsigned long uin)
{
    addPluginInfoRequest(uin, PLUGIN_RANDOMxCHAT);
}

unsigned short ICQClient::msgStatus()
{
    return (unsigned short)(fullStatus(getStatus()) & 0xFF);
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
    f += number(data->Uin.value);
    f = user_file(f.c_str());
    return QFile::decodeName(f.c_str());
}

void ICQClient::retry(int n, void *p)
{
    MsgSend *m = (MsgSend*)p;
    if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendDND){
        if (n == 0){
            m->edit->m_flags = MESSAGE_LIST;
        }else{
            return;
        }
    }else if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendOccupied){
        switch (n){
        case 0:
            m->edit->m_flags = MESSAGE_URGENT;
            break;
        case 1:
            m->edit->m_flags = MESSAGE_LIST;
            break;
        default:
            return;
        }
    }else{
        return;
    }
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m->edit;
    Event e(EventCommandExec, cmd);
    e.process();
}

bool ICQClient::isOwnData(const char *screen)
{
    if ((screen == NULL) || (data.owner.Screen.ptr == NULL))
        return false;
    QString s1(screen);
    QString s2(data.owner.Screen.ptr);
    return (s1.lower() == s2.lower());
}

QString ICQClient::addCRLF(const QString &str)
{
    QString res = str;
    return res.replace(QRegExp("\r?\n"), "\r\n");
}

#ifndef WIN32
#include "icqclient.moc"
#endif

