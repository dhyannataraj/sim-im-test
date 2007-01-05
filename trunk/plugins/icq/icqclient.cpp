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

#include <qtimer.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qfile.h>
#include <qfileinfo.h>

#include "buffer.h"
#include "socket.h"
#include "unquot.h"
#include "log.h"
#include "core.h"

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
#include "icqsecure.h"
#include "icqmessage.h"
#include "securedlg.h"
#include "msgedit.h"
#include "ballonmsg.h"
#include "encodingdlg.h"
#include "warndlg.h"

using namespace std;
using namespace SIM;

static DataDef _icqUserData[] =
    {
        { "", DATA_ULONG, 1, DATA(1) },		// Sign
        { "LastSend", DATA_ULONG, 1, 0 },
        { "Alias", DATA_UTF, 1, 0 },					// Alias
        { "Cellular", DATA_UTF, 1, 0 },					// Cellular
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
        { "Caps2", DATA_ULONG, 1, 0 },
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
        { "WaitAuth", DATA_BOOL, 1, 0 },        // Need auth from to add to our contact list
        { "WantAuth", DATA_BOOL, 1, 0 },        // Want's auth from us
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
        { "EMail", DATA_STRING, 1, 0 },
        { "HiddenEMail", DATA_BOOL, 1, 0 },
        { "City", DATA_STRING, 1, 0 },
        { "State", DATA_STRING, 1, 0 },
        { "HomePhone", DATA_STRING, 1, 0 },
        { "HomeFax", DATA_STRING, 1, 0 },
        { "Address", DATA_UTF, 1, 0 },
        { "PrivateCellular", DATA_STRING, 1, 0 },
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
        { "", DATA_ULONG, 1, 0},                // buddyRosterID
        { "buddyID", DATA_ULONG, 1, 0},
        { "buddyHash", DATA_BINARY, 1, 0},
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

const DataDef *ICQProtocol::icqUserData = _icqUserData;

static DataDef icqClientData[] =
    {
        { "Server", DATA_STRING, 1, 0 },
        { "ServerPort", DATA_ULONG, 1, DATA(5190) },
        { "", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { "HideIP", DATA_BOOL, 1, 0 },
        { "IgnoreAuth", DATA_BOOL, 1, 0 },
        { "UseMD5", DATA_BOOL, 1, 0 },
        { "DirectMode", DATA_ULONG, 1, 0 },
        { "IdleTime", DATA_ULONG, 1, 0 },
        { "ListRequests", DATA_STRING, 1, 0 },
        { "Picture", DATA_UTF, 1, 0 },
        { "RandomChatGroup", DATA_ULONG, 1, 0 },
        { "", DATA_ULONG, 1, 0 },			// RandomChatGroupCurrent
        { "SendFormat", DATA_ULONG, 1, DATA(1) }, // use utf-8 whereever possible
        { "DisablePlugins", DATA_BOOL, 1, 0 },
        { "DisableAutoUpdate", DATA_BOOL, 1, 0 },
        { "DisableAutoReplyUpdate", DATA_BOOL, 1, 0 },
        { "DisableTypingNotification", DATA_BOOL, 1, 0 },
        { "AcceptInDND", DATA_BOOL, 1, 0 },
        { "AcceptInOccupied", DATA_BOOL, 1, 0 },
        { "MinPort", DATA_ULONG, 1, DATA(1024) },
        { "MaxPort", DATA_ULONG, 1, DATA(0xFFFE) },
        { "WarnAnonimously", DATA_BOOL, 1, 0 },
        { "ACKMode", DATA_ULONG, 1, DATA(1) },
        { "UseHTTP", DATA_BOOL, 1, DATA(0) },
        { "AutoHTTP", DATA_BOOL, 1, DATA(0) },
        { "KeepAlive", DATA_BOOL, 1, DATA(1) },
        { "", DATA_STRUCT, sizeof(ICQUserData) / sizeof(Data), DATA(_icqUserData) },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

ICQClient::ICQClient(Protocol *protocol, Buffer *cfg, bool bAIM)
        : TCPClient(protocol, cfg, HighPriority - 1)
{
    m_bAIM = bAIM;

    m_listener = NULL;
    load_data(icqClientData, &data, cfg);
    if (data.owner.Uin.toULong() != 0)
        m_bAIM = false;
    if (!data.owner.Screen.str().isEmpty())
        m_bAIM = true;

    m_bVerifying = false;
    m_bNoSend  = true;
    m_bReady   = false;
    m_bRosters = false;
    m_bJoin    = false;
    m_listRequest = NULL;
    data.owner.DCcookie.asULong() = rand();
    m_bBirthday = false;
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(sendTimeout()));
    m_processTimer = new QTimer(this);
    connect(m_processTimer, SIGNAL(timeout()), this, SLOT(processSendQueue()));
    QString requests = getListRequests();
    while (requests.length()){
        QString req = getToken(requests, ';');
        QString n = getToken(req, ',');
        ListRequest lr;
        lr.type   = n.toUInt();
        lr.screen = req;
        listRequests.push_back(lr);
    }
    disconnected();
    m_bFirstTry = false;
    ContactList::ContactIterator it;
    Contact *contact;
    while ((contact = ++it) != NULL){
        ClientDataIterator itd(contact->clientData, this);
        ICQUserData *data;
        while ((data = (ICQUserData*)(++itd)) != NULL)
            data->Alias.str() = contact->getName();
    }
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
 // FIXME: this does not work and could crash !!!!
 //       Event e(EventRealSendMessage, msg);
 //       e.process();
        delete msg;
    }
    while (!m_sockets.empty())
        delete m_sockets.front();
    m_processMsg.clear();

    freeData();
}

void ICQClient::contactsLoaded()
{
    /* outdated
    QTextCodec *codec = getContacts()->getCodec(NULL);
    QString cdc = codec->name();
    if (codec && (cdc.lower().find("utf") >= 0)){
        QString _def_enc = I18N_NOOP("Dear translator! type this default encoding for your language");
        QString def_enc = i18n(_def_enc);
        if (def_enc == _def_enc){
            EncodingDlg dlg(NULL, this);
            dlg.exec();
        }else{
            getContacts()->owner()->setEncoding(def_enc);
        }
    }
    */
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
    if (data1->Uin.toULong())
        return data1->Uin.toULong() == data2->Uin.toULong();
    if (data2->Uin.toULong())
        return false;
    return (data1->Screen.str() == data2->Screen.str());
}

QCString ICQClient::getConfig()
{
    QString listRequest;
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (listRequest.length())
            listRequest += ';';
        listRequest += QString::number((*it).type);
        listRequest += ',';
        listRequest += (*it).screen;
    }
    setListRequests(listRequest);
    QCString res = Client::getConfig();
    if (res.length())
        res += '\n';
    return res += save_data(icqClientData, &data);
}

QString ICQClient::name()
{
    if (m_bAIM)
        return "AIM." + data.owner.Screen.str();
    return "ICQ." + QString::number(data.owner.Uin.toULong());
}

QWidget	*ICQClient::setupWnd()
{
    if (m_bAIM)
        return new AIMConfig(NULL, this, true);
    return new ICQConfig(NULL, this, true);
}

static const char aim_server[] = "login.oscar.aol.com";
static const char icq_server[] = "login.icq.com";

QString ICQClient::getServer() const
{
    if (!data.Server.str().isEmpty())
        return data.Server.str();
    return m_bAIM ? aim_server : icq_server;
}

void ICQClient::setServer(const QString &server)
{
    if (server == (m_bAIM ? aim_server : icq_server))
        data.Server.str() = QString::null;
    else
        data.Server.str() = server;
}

void ICQClient::setUin(unsigned long uin)
{
    data.owner.Uin.asULong() = uin;
}

void ICQClient::setScreen(const QString &screen)
{
    data.owner.Screen.str() = screen;
}

unsigned long ICQClient::getUin()
{
    return data.owner.Uin.toULong();
}

bool ICQClient::isMyData(clientData *&_data, Contact *&contact)
{
    if (_data->Sign.toULong() != ICQ_SIGN)
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    if (m_bAIM){
        if (!data->Screen.str().isEmpty() && !this->data.owner.Screen.str().isEmpty() &&
                (data->Screen.str().lower() == this->data.owner.Screen.str().lower()))
            return false;
    }else{
        if (data->Uin.toULong() == this->data.owner.Uin.toULong())
            return false;
    }
    ICQUserData *my_data = findContact(screen(data), NULL, false, contact);
    if (my_data){
        data = my_data;
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
    new_data->Screen.str() = data->Screen.str();
    _data = (clientData*)new_data;
    return true;
}

OscarSocket::OscarSocket()
{
    m_nFlapSequence = (unsigned short)(rand() & 0x7FFF);
    m_nMsgSequence  = 0;
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
    m_bFirstTry = false;
    if (m_listener == NULL){
        m_listener = new ICQListener(this);
        m_listener->bind(getMinPort(), getMaxPort(), NULL);
    }
    m_bNoSend	= false;
    m_bReady	= true;
    OscarSocket::connect_ready();
    TCPClient::connect_ready();
}

void ICQClient::setNewLevel(RateInfo &r)
{
    QDateTime now = QDateTime::currentDateTime();
    unsigned delta = 0;
    if (now.date() == r.m_lastSend.date())
        delta = r.m_lastSend.time().msecsTo(now.time());
    unsigned res = (((r.m_winSize - 1) * r.m_curLevel) + delta) / r.m_winSize;
    if (res > r.m_maxLevel)
        res = r.m_maxLevel;
    r.m_curLevel = res;
    r.m_lastSend = now;
    log(L_DEBUG, "Level: %04X [%04X %04X]", res, r.m_minLevel, r.m_winSize);
}

RateInfo *ICQClient::rateInfo(unsigned snac)
{
    RATE_MAP::iterator it = m_rate_grp.find(snac);
    if (it == m_rate_grp.end())
        return NULL;
    return &m_rates[(*it).second];
}

unsigned ICQClient::delayTime(unsigned snac)
{
    RateInfo *r = rateInfo(snac);
    if (r == NULL)
        return 0;
    return delayTime(*r);
}

unsigned ICQClient::delayTime(RateInfo &r)
{
    if (r.m_winSize == 0)
        return 0;
    int res = r.m_minLevel * r.m_winSize - r.m_curLevel * (r.m_winSize - 1);
    if (res < 0)
        return 0;
    QDateTime now = QDateTime::currentDateTime();
    unsigned delta = 0;
    if (now.date() == r.m_lastSend.date())
        delta = r.m_lastSend.time().msecsTo(now.time());
    res -= delta;
    return (res > 0) ? res : 0;
}

void ICQClient::setStatus(unsigned status, bool bCommon)
{
    if (status != STATUS_OFFLINE){
        switch (status){
        case STATUS_NA:
        case STATUS_AWAY:
            if (getIdleTime() == 0)
                setIdleTime(time(NULL));
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
                setAwayMessage();
                EventClientChanged(this).process();
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
            EventARRequest(&ar).process();
            EventClientChanged(this).process();
        }
        return;
    }
    if (status != m_status){
        m_status = status;
        sendStatus();
        EventClientChanged(this).process();
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
        EventClientChanged(this).process();
    }
}

void ICQClient::disconnected()
{
    m_rates.clear();
    m_rate_grp.clear();
    m_sendTimer->stop();
    m_processTimer->stop();
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
            if ((data->Status.toULong() != ICQ_STATUS_OFFLINE) || data->bInvisible.toBool()){
                setOffline(data);
                StatusMessage *m = new StatusMessage();
                m->setContact(contact->id());
                m->setClient(dataName(data));
                m->setStatus(STATUS_OFFLINE);
                m->setFlags(MESSAGE_RECEIVED);
                EventMessageReceived e(m);
                if(!e.process())
                    delete m;
            }
        }
    }
    for (list<Message*>::iterator itm = m_acceptMsg.begin(); itm != m_acceptMsg.end(); ++itm){
        EventMessageDeleted(*itm).process();
        delete *itm;
    }
    m_acceptMsg.clear();
    m_bRosters = false;
    m_nMsgSequence = 0;
    m_bIdleTime = false;
    m_bNoSend	= true;
    m_bReady	= false;
    m_cookie.resize(0);
    m_advCounter = 0;
    m_nUpdates = 0;
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
    if ((error < 1) || (error > 0x18)) {
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
    EventLog::log_packet(m_socket->readBuffer, false, plugin->OscarPacket);
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
                log(L_DEBUG,"Error! family: %04X reason: %s",fam,error_message(err_code));
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
                log(L_WARN, "Unknown family %04X", fam);
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
    socket()->writeBuffer.packetStart();
    socket()->writeBuffer
    << (char)0x2A
    << channel
    << 0x00000000L;
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

void OscarSocket::sendPacket(bool bSend)
{
    Buffer &writeBuffer = socket()->writeBuffer;
    char *packet = writeBuffer.data(writeBuffer.packetStartPos());
    unsigned size = writeBuffer.size() - writeBuffer.packetStartPos() - 6;
    packet[4] = (char)((size >> 8) & 0xFF);
    packet[5] = (char)(size & 0xFF);
    if (bSend){
        ++m_nFlapSequence;
        packet[2] = (m_nFlapSequence >> 8);
        packet[3] = m_nFlapSequence;
        EventLog::log_packet(socket()->writeBuffer, true, ICQPlugin::icq_plugin->OscarPacket);
        socket()->write();
    }
}

void ICQClient::sendPacket(bool bSend)
{
    Buffer &writeBuffer = socket()->writeBuffer;
    unsigned char *packet = (unsigned char*)(writeBuffer.data(writeBuffer.readPos()));
    unsigned long snac = 0;
    if (writeBuffer.writePos() >= writeBuffer.readPos() + 10)
        snac = (packet[6] << 24) + (packet[7] << 16) + (packet[8] << 8) + packet[9];
    unsigned delay = delayTime(snac);
    if (m_bNoSend){
        bSend = false;
    }else if (!bSend && (delay == 0)){
        bSend = true;
    }
    RateInfo *r = rateInfo(snac);
    if (r){
        if (m_bNoSend || r->delayed.size())
            bSend = false;
    }else{
        bSend = true;
    }
    if (bSend){
        if (r)
            setNewLevel(*r);
        OscarSocket::sendPacket(true);
        return;
    }
    OscarSocket::sendPacket(false);
    r->delayed.pack(writeBuffer.data(writeBuffer.packetStartPos()), writeBuffer.size() - writeBuffer.packetStartPos());
    writeBuffer.resize(writeBuffer.packetStartPos());
    m_processTimer->stop();
    m_processTimer->start(delay);
}

QCString ICQClient::cryptPassword()
{
    unsigned char xor_table[] =
        {
            0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
            0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c
        };
    QCString pswd = getContacts()->fromUnicode(NULL, getPassword());
    QCString res;
    for (int j = 0; j < 8; j++){
        char c = pswd[j];
        if (c == 0)
            break;
        c = (char)(c ^ xor_table[j]);
        res += c;
    }
    return res;
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
    if (data.owner.WebAware.toBool())
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
    if (getInvisible()){
        status |= ICQ_STATUS_FxPRIVATE | ICQ_STATUS_FxHIDExIP;
        status &= ~(ICQ_STATUS_FxDIRECTxLISTED | ICQ_STATUS_FxDIRECTxAUTH);
    }
    return status;
}

ICQUserData *ICQClient::findContact(unsigned long l, const QString *alias, bool bCreate, Contact *&contact, Group *grp, bool bJoin)
{
    return findContact(QString::number(l), alias, bCreate, contact, grp, bJoin);
}

ICQUserData *ICQClient::findContact(const QString &screen, const QString *alias, bool bCreate, Contact *&contact, Group *grp, bool bJoin)
{
    if (screen.isEmpty())
        return NULL;

    QString s = screen.lower();

    ContactList::ContactIterator it;
    ICQUserData *data;
    unsigned long uin = screen.toULong();

    while ((contact = ++it) != NULL){
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (uin && (data->Uin.toULong() != uin))
                continue;
            if ((uin == 0) && (s != data->Screen.str()))
                continue;
            bool bChanged = false;
            if (alias){
                if (!alias->isEmpty()){
                    bChanged = contact->setName(*alias);
                }
                data->Alias.str() = *alias;
            }
            if (grp){
                if (contact->getGroup() != grp->id()){
                    contact->setGroup(grp->id());
                    bChanged = true;
                }
            }
            if (bChanged){
                EventContact e(contact, EventContact::eChanged);
                e.process();
            }
            return data;
        }
    }
    if (!bCreate)
        return NULL;
    if (bJoin){
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
                    if (uin && (data->Uin.toULong() != uin))
                        continue;
                    if ((uin == 0) && (s != data->Screen.str()))
                        continue;
                    data = (ICQUserData*)(contact->clientData.createData(this));
                    data->Uin.asULong() = uin;
                    if (uin == 0)
                        data->Screen.str() = s;
                    bool bChanged = false;
                    if (alias){
                        if (*alias){
                            bChanged = contact->setName(*alias);
                        }
                        data->Alias.str() = *alias;
                    }
                    if (grp){
                        if (grp->id() != contact->getGroup()){
                            contact->setGroup(grp->id());
                            bChanged = true;
                        }
                    }
                    if (bChanged){
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                        updateInfo(contact, data);
                    }
                    updateInfo(contact, data);
                    return data;
                }
            }
        }
        if (alias && !alias->isEmpty()){
            QString name = alias->lower();
            it.reset();
            while ((contact = ++it) != NULL){
                if (contact->getName().lower() == name){
                    ICQUserData *data = (ICQUserData*)(contact->clientData.createData(this));
                    data->Uin.asULong() = uin;
                    if (uin == 0)
                        data->Screen.str() = screen;
                    data->Alias.str() = alias ? *alias : QString::null;
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                    m_bJoin = true;
                    updateInfo(contact, data);
                    return data;
                }
            }
        }
    }
    contact = getContacts()->contact(0, true);
    data = (ICQUserData*)(contact->clientData.createData(this));
    data->Uin.asULong() = uin;
    if (uin == 0)
        data->Screen.str() = s;
    QString name;
    if (alias){
        name = *alias;
    }else if (uin){
        name = QString::number(uin);
    }else{
        name = screen;
    }
    if(alias)
        data->Alias.str() = *alias;
    contact->setName(name);
    if (grp)
        contact->setGroup(grp->id());
    EventContact e(contact, EventContact::eChanged);
    e.process();
    updateInfo(contact, data);
    return data;
}

ICQUserData *ICQClient::findGroup(unsigned id, const QString *alias, Group *&grp)
{
    ContactList::GroupIterator it;
    ICQUserData *data;
    while ((grp = ++it) != NULL){
        data = (ICQUserData*)(grp->clientData.getData(this));
        if (data && (data->IcqID.toULong() == id)){
            if (alias)
                data->Alias.str() = *alias;
            return data;
        }
    }
    if (alias == NULL)
        return NULL;
    it.reset();
    QString name = *alias;
    while ((grp = ++it) != NULL){
        if (grp->getName() == name){
            data = (ICQUserData*)(grp->clientData.createData(this));
            data->IcqID.asULong() = id;
            data->Alias.str() = *alias;
            return data;
        }
    }
    grp = getContacts()->group(0, true);
    grp->setName(name);
    data = (ICQUserData*)(grp->clientData.createData(this));
    data->IcqID.asULong() = id;
    data->Alias.str() = *alias;
    EventGroup e(grp, EventGroup::eChanged);
    e.process();
    return data;
}

void ICQClient::setOffline(ICQUserData *data)
{
    QString name = dataName(data);
    for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ){
        Message *msg = *it;
        if (msg->client() && (name == msg->client())){
            EventMessageDeleted(msg).process();
            delete msg;
            m_acceptMsg.erase(it);
            it = m_acceptMsg.begin();
        }
        ++it;
    }
    if (data->Direct.object()){
        delete data->Direct.object();
        data->Direct.clear();
    }
    if (data->DirectPluginInfo.object()){
        delete data->DirectPluginInfo.object();
        data->DirectPluginInfo.clear();
    }
    if (data->DirectPluginStatus.object()){
        delete data->DirectPluginStatus.object();
        data->DirectPluginStatus.clear();
    }
    data->bNoDirect.asBool()  = false;
    data->Status.asULong()    = ICQ_STATUS_OFFLINE;
    data->Class.asULong()     = 0;
    data->bTyping.asBool()    = false;
    data->bBadClient.asBool() = false;
    data->bInvisible.asBool() = false;
    data->StatusTime.asULong()= time(NULL);
    data->AutoReply.str() = QString::null;
}

static void addIcon(QString *s, const QString &icon, const QString &statusIcon)
{
    if (s == NULL)
        return;
    if (statusIcon == icon)
        return;
    QString str = *s;
    while (!str.isEmpty()){
        QString item = getToken(str, ',');
        if (item == icon)
            return;
    }
    if (!s->isEmpty())
        *s += ',';
    *s += icon;
}

void ICQClient::contactInfo(void *_data, unsigned long &curStatus, unsigned &style, QString &statusIcon, QString *icons)
{
    ICQUserData *data = (ICQUserData*)(_data);
    unsigned status = STATUS_ONLINE;
    unsigned s = data->Status.toULong();
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
    QString dicon;
    if (data->Uin.toULong()){
        if ((iconStatus == STATUS_ONLINE) && (s & ICQ_STATUS_FxPRIVATE)){
            dicon = "ICQ_invisible";
        }else{
            const CommandDef *def = ICQProtocol::_statusList();
            for (; !def->text.isEmpty(); def++){
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
            if (data->Class.toULong() & CLASS_AWAY){
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
        if (!statusIcon.isEmpty() && icons){
            QString iconSave = *icons;
            *icons = statusIcon;
            if (iconSave.length())
                addIcon(icons, iconSave, statusIcon);
        }
        statusIcon = dicon;
    }else{
        if (!statusIcon.isEmpty()){
            addIcon(icons, dicon, statusIcon);
        }else{
            statusIcon = dicon;
        }
    }
    if ((status == STATUS_OFFLINE) && data->bInvisible.toBool()){
        status = STATUS_INVISIBLE;
        if (status > curStatus)
            curStatus = status;
    }
    if (icons){
        if ((iconStatus != STATUS_ONLINE) && (iconStatus != STATUS_OFFLINE) && (s & ICQ_STATUS_FxPRIVATE))
            addIcon(icons, "ICQ_invisible", statusIcon);
        if (data->bInvisible.toBool())
            addIcon(icons, "ICQ_invisible", statusIcon);
        if (data->Status.toULong() & ICQ_STATUS_FxBIRTHDAY)
            addIcon(icons, "birthday", statusIcon);
        if (data->FollowMe.toULong() == 1)
            addIcon(icons, "phone", statusIcon);
        if (data->FollowMe.toULong() == 2)
            addIcon(icons, "nophone", statusIcon);
        if (status != STATUS_OFFLINE){
            if (data->SharedFiles.toBool())
                addIcon(icons, "sharedfiles", statusIcon);
            if (data->ICQPhone.toULong() == 1)
                addIcon(icons, "icqphone", statusIcon);
            if (data->ICQPhone.toULong() == 2)
                addIcon(icons, "icqphonebusy", statusIcon);
        }
        if (data->bTyping.toBool())
            addIcon(icons, "typing", statusIcon);
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if (dc && dc->isSecure())
            addIcon(icons, "encrypted", statusIcon);
    }
    if (data->InvisibleId.toULong())
        style |= CONTACT_STRIKEOUT;
    if (data->VisibleId.toULong())
        style |= CONTACT_ITALIC;
    if (data->WaitAuth.toBool())
        style |= CONTACT_UNDERLINE;
}

void ICQClient::ping()
{
    if (getState() == Connected){
        bool bBirthday = false;
        if (!m_bAIM){
            int year  = data.owner.BirthYear.toULong();
            int month = data.owner.BirthMonth.toULong();
            int day   = data.owner.BirthDay.toULong();
            if (day && month && year){
				QDate tNow = QDate::currentDate();
				QDate tBirthday(tNow.year(), month, day);
                // we send it two days before we've birthday
				int diff = tNow.daysTo(tBirthday);
				if(diff >= 0 && diff <=2)
	                bBirthday = true;
				else {
					tBirthday = tBirthday.addYears(1);
					diff = tNow.daysTo(tBirthday);
					if(diff >= 0 && diff <=2)
						bBirthday = true;
				}
            }
        }
        if (bBirthday != m_bBirthday){
            m_bBirthday = bBirthday;
            setStatus(m_status);
        }else if (getKeepAlive() || m_bHTTP){
            bool bSend = true;
            for (unsigned i = 0; i < m_rates.size(); i++){
                if (m_rates[i].delayed.size()){
                    bSend = false;
                    break;
                }
            }
            if (bSend){
                flap(ICQ_CHNxPING);
                sendPacket(false);
            }
        }
        processSendQueue();
        checkListRequest();
        checkInfoRequest();
        QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
    }
}

void ICQClient::setupContact(Contact *contact, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    QString phones;
    if (!data->HomePhone.str().isEmpty()){
        phones += trimPhone(data->HomePhone.str());
        phones += ",Home Phone,";
        phones += QString::number(PHONE);
    }
    if (!data->HomeFax.str().isEmpty()){
        if (phones.length())
            phones += ';';
        phones += trimPhone(data->HomeFax.str());
        phones += ",Home Fax,";
        phones += QString::number(FAX);
    }
    if (!data->WorkPhone.str().isEmpty()){
        if (phones.length())
            phones += ';';
        phones += trimPhone(data->WorkPhone.str());
        phones += ",Work Phone,";
        phones += QString::number(PHONE);
    }
    if (!data->WorkFax.str().isEmpty()){
        if (phones.length())
            phones += ';';
        phones += trimPhone(data->WorkFax.str());
        phones += ",Work Fax,";
        phones += QString::number(FAX);
    }
    if (!data->PrivateCellular.str().isEmpty()){
        if (phones.length())
            phones += ';';
        phones += trimPhone(data->PrivateCellular.str());
        phones += ",Private Cellular,";
        phones += QString::number(CELLULAR);
    }
    if (!data->PhoneBook.str()){
        if (phones.length())
            phones += ';';
        phones += data->PhoneBook.str();
    }
    contact->setPhones(phones, name());
    QString mails;
    if (!data->EMail.str().isEmpty())
        mails += data->EMail.str().stripWhiteSpace();
    if (!data->EMail.str().isEmpty()) {
        QString emails = data->EMails.str();
        while (emails.length()){
            QString mailItem = getToken(emails, ';', false);
            QString mail = getToken(mailItem, '/').stripWhiteSpace();
            if (mail.length()){
                if (mails.length())
                    mails += ';';
                mails += mail;
            }
        }
    }
    QString n = name();
    contact->setEMails(mails, n);
    QString firstName = data->FirstName.str();
    if (firstName.length())
        contact->setFirstName(firstName, n);
    QString lastName = data->LastName.str();
    if (lastName.length())
        contact->setLastName(lastName, n);
    if (contact->getName().isEmpty())
        contact->setName(QString::number(data->Uin.toULong()));
    QString nick = data->Nick.str();
    if (nick.isEmpty())
        nick = data->Alias.str();
    if (!nick.isEmpty()){
        QString name = QString::number(data->Uin.toULong());
        if (name == contact->getName())
            contact->setName(nick);
    }
}

QString ICQClient::trimPhone(const QString &from)
{
    QString res;
    if (from.isEmpty())
        return res;
    res = from;
    int idx = res.find("SMS");
    if(idx != -1)
        res = res.left(idx);
    return res.stripWhiteSpace();
}

QString ICQClient::contactTip(void *_data)
{
    ICQUserData *data = (ICQUserData*)(_data);
    QString res;
    QString statusText;
    unsigned long status = STATUS_OFFLINE;
    unsigned style  = 0;
    QString statusIcon;
    contactInfo(data, status, style, statusIcon);
    if (status == STATUS_INVISIBLE){
        res += "<img src=\"icon:ICQ_invisible\">";
        res += i18n("Possibly invisible");
    }else{
        res += "<img src=\"icon:";
        res += statusIcon;
        res += "\">";
        if (statusIcon == "ICQ_invisible"){
            res += ' ';
            res += i18n("Invisible");
        }else  if (data->Uin.toULong()){
            for (const CommandDef *cmd = ICQProtocol::_statusList(); !cmd->text.isEmpty(); cmd++){
                if (cmd->icon == statusIcon){
                    res += ' ';
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
    if (data->Uin.toULong()){
        res += "UIN: <b>";
        res += QString::number(data->Uin.toULong());
        res += "</b>";
    }else{
        res += "<b>";
        res += data->Screen.str();
        res += "</b>";
    }
    if (data->WarningLevel.toULong()){
        res += "<br>";
        res += i18n("Warning level");
        res += ": <b>";
        res += QString::number(warnLevel(data->WarningLevel.toULong()));
        res += "% </b></br>";
    }
    if (data->Status.toULong() == ICQ_STATUS_OFFLINE){
        if (data->StatusTime.toULong()){
            res += "<br><font size=-1>";
            res += i18n("Last online");
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
        }
    }else{
        if (data->OnlineTime.toULong()){
            res += "<br><font size=-1>";
            res += i18n("Online");
            res += ": </font>";
            res += formatDateTime(data->OnlineTime.toULong());
        }
        if (data->Status.toULong() & (ICQ_STATUS_AWAY | ICQ_STATUS_NA)){
            res += "<br><font size=-1>";
            res += statusText;
            res += ": </font>";
            res += formatDateTime(data->StatusTime.toULong());
        }
    }
    if (data->IP.ip()){
        res += "<br>";
        res += formatAddr(data->IP, data->Port.toULong());
    }
    if ((data->RealIP.ip()) && ((data->IP.ip() == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        res += "<br>";
        res += formatAddr(data->RealIP, data->Port.toULong());
    }
    QString client_name = clientName(data);
    if (client_name.length()){
        res += "<br>";
        res += quoteString(client_name);
    }
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
        QString url="pict://icqavatar." + QString::number(data->Uin.toULong());
        QMimeSourceFactory::defaultFactory()->setPixmap(url, pict);
        res += "<br><img src=\"" + url + "\" width=\"";
        res += QString::number(w);
        res += "\" height=\"";
        res += QString::number(h);
        res += "\">";
    }
    if (!data->AutoReply.str().isEmpty()){
        res += "<br><br>";
        res += quoteString(data->AutoReply.str());
    }
    return res;
}

unsigned long ICQClient::warnLevel(unsigned long level)
{
    level = ((level + 5) / 10);
    if (level > 100)
        level = 100;
    return level;
}

bool ICQClient::hasCap(const ICQUserData *data, cap_id_t n)
{
    unsigned long val = n > 31 ? data->Caps2.toULong() : data->Caps.toULong();
    int pos = (int)n % 32;
    return (val & (1 << pos)) != 0;
}

void ICQClient::setCap(ICQUserData *data, cap_id_t n)
{
    unsigned long &val = n > 31 ? data->Caps2.asULong() : data->Caps.asULong();
    int pos = (int)n % 32;
    val |= (1 << pos);
}

static QString verString(unsigned ver)
{
    QString res;
    if (ver == 0)
        return res;
    unsigned char v[4];
    v[0] = (unsigned char)((ver >> 24) & 0xFF);
    v[1] = (unsigned char)((ver >> 16) & 0xFF);
    v[2] = (unsigned char)((ver >>  8) & 0xFF);
    v[3] = (unsigned char)((ver >>  0) & 0xFF);
    if ((v[0] & 0x80) || (v[1] & 0x80) || (v[2] & 0x80) || (v[3] & 0x80))
        return res;

    res.sprintf(" %u.%u", v[0], v[1]);
    if (v[2] || v[3]){
        QString s;
        s.sprintf(".%u", v[2]);
        res += s;
    }
    if (v[3]){
        QString s;
        s.sprintf(".%u", v[3]);
        res += s;
    }
    return res;
}

QString ICQClient::clientName(ICQUserData *data)
{
    QString res;
    if (data->Version.toULong())
        res.sprintf("v%lu ", data->Version.toULong());

    if (hasCap(data, CAP_MIRANDA)) {
        QString r;
        unsigned ver1 = (data->Build.toULong() >> 24) & 0xFF;
        unsigned ver2 = (data->Build.toULong() >> 16) & 0xFF;
        unsigned ver3 = (data->Build.toULong() >>  8) & 0xFF;
        unsigned ver4 = (data->Build.toULong() >>  0) & 0xFF;
        r.sprintf("Miranda %u.%u.%u.%u", ver1, ver2, ver3, ver4);
        return res + r;
    }
    if (hasCap(data, CAP_QIP)) {
        res += "QIP 2005a";
        return res;
    }
    if (hasCap(data, CAP_JIMM)) {
        QString r;
        unsigned maj = (data->Build.toULong() >> 24) & 0xFF;
        unsigned min = (data->Build.toULong() >> 16) & 0xFF;
        unsigned rev = (data->Build.toULong() >>  0) & 0xFFFF;
        if(rev)
            r.sprintf("Jimm %d.%d.%d", maj, min, rev);
        else
            r.sprintf("Jimm %d.%d", maj, min);
        return res + r;
    }
    if (hasCap(data, CAP_ICQ51)) {
        res += "ICQ 5.1";
        return res;
    }
    if (hasCap(data, CAP_ICQ5_1) && hasCap(data, CAP_ICQ5_3) && hasCap(data, CAP_ICQ5_4)) {
        res += "ICQ 5.0";
        return res;
    }
    if (hasCap(data, CAP_ICQ5_1)) {
        log( L_DEBUG, "CAP_ICQ5_1 without all others" );
    }
    if (hasCap(data, CAP_ICQ5_3)) {
        log( L_DEBUG, "CAP_ICQ5_3 without all others" );
    }
    if (hasCap(data, CAP_ICQ5_4)) {
        log( L_DEBUG, "CAP_ICQ5_4 without all others" );
    }
    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN)) {
        res += "Trillian";
        return res;
    }

    if (hasCap(data, CAP_SIMOLD)) {
        QString r;
        unsigned hiVersion = (data->Build.toULong() >> 6) - 1;
        unsigned loVersion = data->Build.toULong() & 0x1F;
        r.sprintf("SIM %u.%u", hiVersion, loVersion);
        return res + r;
    }

    if (hasCap(data, CAP_SIM)) {
        QString r;
        unsigned ver1 = (data->Build.toULong() >> 24) & 0xFF;
        unsigned ver2 = (data->Build.toULong() >> 16) & 0xFF;
        unsigned ver3 = (data->Build.toULong() >> 8) & 0xFF;
        unsigned ver4 = data->Build.toULong() & 0x0F;
        if (ver4){
            r.sprintf("SIM %u.%u.%u.%u", ver1, ver2, ver3, ver4);
        }else if (ver3){
            r.sprintf("SIM %u.%u.%u", ver1, ver2, ver3);
        }else{
            r.sprintf("SIM %u.%u", ver1, ver2);
        }
        res += r;
        if (data->Build.toULong() & 0x80)
            res += "/win32";

        if (data->Build.toULong() & 0x40)
            res += "/MacOS X";
        return res;
    }

    if (hasCap(data, CAP_LICQ)) {
        QString r;
        unsigned ver1 = (data->Build.toULong() >> 24) & 0xFF;
        unsigned ver2 = (data->Build.toULong() >> 16) & 0xFF;
        unsigned ver3 = (data->Build.toULong() >> 8) & 0xFF;
        ver2 %=100;	// see licq source
        r.sprintf("Licq %u.%u.%u", ver1, ver2, ver3);
        res += r;
        if ((data->Build.toULong() & 0xFF) == 1)
            res += "/SSL";
        return res;
    }
    if (hasCap(data, CAP_KOPETE)) {
        // last 4 bytes determine version
        // NOTE change with each Kopete Release!
        // first number, major version
        // second number,  minor version
        // third number, point version 100+
        // fourth number,  point version 0-99
        QString r;
        unsigned ver1 =  (data->Build.toULong() >> 24) & 0xFF;	// major
        unsigned ver2 =  (data->Build.toULong() >> 16) & 0xFF;	// minor
        unsigned ver3 = ((data->Build.toULong() >>  8) & 0xFF) * 100;
        ver3         +=  (data->Build.toULong() >>  0) & 0xFF;
        r.sprintf("Kopete %u.%u.%u", ver1, ver2, ver3);
        res += r;
        return res;
    }
    if (hasCap(data, CAP_XTRAZ)){
        res += "ICQ 4.0 Lite";
        return res;
    }
    if (hasCap(data, CAP_MACICQ)){
        res += "ICQ for Mac";
        return res;
    }
    //  gaim 2.0 development
    if (hasCap(data, CAP_AIM_SENDFILE) &&
        hasCap(data, CAP_AIM_IMIMAGE) &&
        hasCap(data, CAP_AIM_BUDDYCON) &&
        hasCap(data, CAP_UTF) &&
        hasCap(data, CAP_AIM_CHAT)){
        res += "gaim 2.0dev";
        return res;
    }
    if (hasCap(data, CAP_AIM_CHAT)){
        res += "AIM";
        return res;
    }
    if ((data->InfoUpdateTime.toULong() & 0xFF7F0000L) == 0x7D000000L){
        QString r;
        unsigned ver = data->InfoUpdateTime.toULong() & 0xFFFF;
        if (ver % 10){
            r.sprintf("Licq %u.%u.%u", ver / 1000, (ver / 10) % 100, ver % 10);
        }else{
            r.sprintf("Licq %u.%u", ver / 1000, (ver / 10) % 100);
        }
        res += r;
        if (data->InfoUpdateTime.toULong() & 0x00800000L)
            res += "/SSL";
        return res;
    }
    switch (data->InfoUpdateTime.toULong()){
    case 0xFFFFFFFFL:
        if ((data->PluginStatusTime.toULong() == 0xFFFFFFFFL) && (data->PluginInfoTime.toULong() == 0xFFFFFFFFL)){
            res += "GAIM";
            return res;
        }
        res += "MIRANDA";
        res += verString(data->PluginInfoTime.toULong() & 0xFFFFFF);
        if (data->PluginInfoTime.toULong() & 0x80000000)
            res += " alpha";
        return res;
    case 0xFFFFFF8FL:
        res += "StrICQ";
        res += verString(data->PluginInfoTime.toULong() & 0xFFFFFF);
        return res;
    case 0xFFFFFF42L:
        res += "mICQ";
        return res;
    case 0xFFFFFFBEL:
        res += "alicq";
        res += verString(data->PluginInfoTime.toULong() & 0xFFFFFF);
        return res;
    case 0xFFFFFF7FL:
        res += "&RQ";
        res += verString(data->PluginInfoTime.toULong() & 0xFFFFFF);
        return res;
    case 0xFFFFFFABL:
        res += "YSM";
        res += verString(data->PluginInfoTime.toULong() & 0xFFFF);
        return res;
    case 0x04031980L: {
        QString r;
        r.sprintf("vICQ 0.43.%lu.%lu", data->PluginInfoTime.toULong() & 0xffff, data->PluginInfoTime.toULong() & (0x7fff0000) >> 16);
        res += r;
        return res;
    }
    case 0x3AA773EEL:
        if ((data->PluginStatusTime.toULong() == 0x3AA66380L) && (data->PluginInfoTime.toULong() == 0x3A877A42L))
        {
            res += "libicq2000";
            return res;
        }
        break;
    }

    if (hasCap(data, CAP_TYPING)) {
        switch (data->Version.toULong()){
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
    if (data->InfoUpdateTime.toULong() &&
            (data->InfoUpdateTime.toULong() == data->PluginStatusTime.toULong()) &&
            (data->PluginStatusTime.toULong() == data->PluginInfoTime.toULong()) &&
            (data->Caps.toULong() == 0) && (data->Caps2.toULong() == 0)){
        res += "vICQ";
        return res;
    }
    if (hasCap(data, CAP_AIM_BUDDYCON)){
        res += "gaim";
        return res;
    }
    if ((hasCap(data, CAP_STR_2001) || hasCap(data, CAP_SRV_RELAY)) && hasCap(data, CAP_IS_2001)){
        res += "ICQ 2001";
        return res;
    }
    if ((hasCap(data, CAP_STR_2001) || hasCap(data, CAP_SRV_RELAY)) && hasCap(data, CAP_IS_2002)){
        res += "ICQ 2002";
        return res;
    }
    if (hasCap(data, CAP_RTF) && hasCap(data, CAP_UTF) &&
            hasCap(data, CAP_SRV_RELAY) && hasCap(data, CAP_DIRECT)) {
        res += "ICQ 2003a";
        return res;
    }
    if (hasCap(data, CAP_SRV_RELAY) && hasCap(data, CAP_DIRECT)){
        res += "ICQ 2001b";
        return res;
    }
    if ((data->Version.toULong() == 7) && hasCap(data, CAP_RTF)){
        res += "GnomeICU";
        return res;
    }
    // ICQ2go doesn't use CAP_TYPING anymore
    if ((data->Version.toULong() == 7) && hasCap(data, CAP_UTF)){
        res += "ICQ2go";
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
        CommandDef (
            MAIN_INFO,
            " ",
            "ICQ_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            MORE_INFO,
            I18N_NOOP("More info"),
            "more",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            INTERESTS_INFO,
            I18N_NOOP("Interests"),
            "interest",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            PAST_INFO,
            I18N_NOOP("Group/Past"),
            "past",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            PICTURE_INFO,
            I18N_NOOP("Picture"),
            "pict",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (),
    };

static CommandDef aimWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "AIM_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (),
    };

static CommandDef icqConfigWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "ICQ_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            HOME_INFO,
            I18N_NOOP("Home info"),
            "home",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            WORK_INFO,
            I18N_NOOP("Work info"),
            "work",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            MORE_INFO,
            I18N_NOOP("More info"),
            "more",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            INTERESTS_INFO,
            I18N_NOOP("Interests"),
            "interest",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            PAST_INFO,
            I18N_NOOP("Group/Past"),
            "past",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            PICTURE_INFO,
            I18N_NOOP("Picture"),
            "pict",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            NETWORK,
            I18N_NOOP("Network"),
            "network",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            SECURITY,
            I18N_NOOP("Security"),
            "security",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (),
    };

static CommandDef aimConfigWnd[] =
    {
        CommandDef (
            MAIN_INFO,
            " ",
            "AIM_online",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            ABOUT_INFO,
            I18N_NOOP("About info"),
            "info",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (
            NETWORK,
            I18N_NOOP("Network"),
            "network",
            QString::null,
            QString::null,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            QString::null
        ),
        CommandDef (),
    };

CommandDef *ICQClient::infoWindows(Contact*, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    CommandDef *def = data->Uin.toULong() ? icqWnd : aimWnd;
    QString name = i18n(protocol()->description()->text);
    name += ' ';
    if (data->Uin.toULong()){
        name += QString::number(data->Uin.toULong());
    }else{
        name += data->Screen.str();
    }
    def->text_wrk = name;
    return def;
}

CommandDef *ICQClient::configWindows()
{
    CommandDef *def = icqConfigWnd;
    QString name = i18n(protocol()->description()->text);
    name += ' ';
    if (m_bAIM){
        name += data.owner.Screen.str();
        def = aimConfigWnd;
    }else{
        name += QString::number(data.owner.Uin.toULong());
    }
    def->text_wrk = name;
    return def;
}

QWidget *ICQClient::infoWindow(QWidget *parent, Contact *contact, void *_data, unsigned id)
{
    ICQUserData *data = (ICQUserData*)_data;
    switch (id){
    case MAIN_INFO:
        if (data->Uin.toULong())
            return new ICQInfo(parent, data, contact->id(), this);
        return new AIMInfo(parent, data, contact->id(), this);
    case HOME_INFO:
        return new HomeInfo(parent, data, contact->id(), this);
    case WORK_INFO:
        return new WorkInfo(parent, data, contact->id(), this);
    case MORE_INFO:
        return new MoreInfo(parent, data, contact->id(), this);
    case ABOUT_INFO:
        return new AboutInfo(parent, data, contact->id(), this);
    case INTERESTS_INFO:
        return new InterestsInfo(parent, data, contact->id(), this);
    case PAST_INFO:
        return new PastInfo(parent, data, contact->id(), this);
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
            return new AIMInfo(parent, NULL, 0, this);
        return new ICQInfo(parent, NULL, 0, this);
    case HOME_INFO:
        return new HomeInfo(parent, NULL, 0, this);
    case WORK_INFO:
        return new WorkInfo(parent, NULL, 0, this);
    case MORE_INFO:
        return new MoreInfo(parent, NULL, 0, this);
    case ABOUT_INFO:
        return new AboutInfo(parent, NULL, 0, this);
    case INTERESTS_INFO:
        return new InterestsInfo(parent, NULL, 0, this);
    case PAST_INFO:
        return new PastInfo(parent, NULL, 0, this);
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

QWidget *ICQClient::searchWindow(QWidget *parent)
{
    if (getState() != Connected)
        return NULL;
    return new ICQSearch(this, parent);
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
    if (data->Uin.toULong()){
        addFullInfoRequest(data->Uin.toULong());
        addPluginInfoRequest(data->Uin.toULong(), PLUGIN_QUERYxINFO);
        addPluginInfoRequest(data->Uin.toULong(), PLUGIN_QUERYxSTATUS);
        addPluginInfoRequest(data->Uin.toULong(), PLUGIN_AR);
    }else{
        fetchProfile(data);
    }
    requestBuddy(data);
}

void ICQClient::fetchAwayMessage(ICQUserData *data)
{
    addPluginInfoRequest(data->Uin.toULong(), PLUGIN_AR);
}

bool ICQClient::processEvent(Event *e)
{
    TCPClient::processEvent(e);
    switch (e->type()) {
    case eEventAddContact: {
        EventAddContact *ec = static_cast<EventAddContact*>(e);
        EventAddContact::AddContact *ac = ec->addContact();
        if (protocol()->description()->text == ac->proto){
            Group *grp = getContacts()->group(ac->group);
            Contact *contact;
            QString tmp = ac->nick;
            findContact(ac->addr, &tmp, true, contact, grp);
            ec->setContact(contact);
            return true;
        }
        break;
    }
    case eEventDeleteContact: {
        EventDeleteContact *ec = static_cast<EventDeleteContact*>(e);
        QString addr = ec->alias();
        ContactList::ContactIterator it;
        Contact *contact;
        while ((contact = ++it) != NULL){
            ICQUserData *data;
            ClientDataIterator itc(contact->clientData, this);
            while ((data = (ICQUserData*)(++itc)) != NULL){
                if (data->Screen.str() == addr){
                    contact->clientData.freeData(data);
                    ClientDataIterator itc(contact->clientData);
                    if (++itc == NULL)
                        delete contact;
                    return true;
                }
            }
        }
        break;
    }
    case eEventGetContactIP: {
        EventGetContactIP *ei = static_cast<EventGetContactIP*>(e);
        Contact *contact = ei->contact();
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, this);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (data->RealIP.ip()) {
                ei->setIP(data->RealIP.ip());
                return true;
            }
            if (data->IP.ip()) {
                ei->setIP(data->IP.ip());
                return true;
            }
        }
        break;
    }
    case eEventMessageAccept: {
        EventMessageAccept *ema = static_cast<EventMessageAccept*>(e);
        for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
            if ((*it)->id() == ema->msg()->id()){
                Message *msg = *it;
                m_acceptMsg.erase(it);
                accept(msg, ema->dir(), ema->mode());
                return msg;
            }
        }
        break;
    }
    case eEventMessageDecline: {
        EventMessageDecline *emd = static_cast<EventMessageDecline*>(e);
        for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
            if ((*it)->id() == emd->msg()->id()){
                Message *msg = *it;
                m_acceptMsg.erase(it);
                decline(msg, emd->reason());
                return msg;
            }
        }
        break;
    }
    case eEventMessageRetry: {
        EventMessageRetry *emr = static_cast<EventMessageRetry*>(e);
        EventMessageRetry::MsgSend *m = emr->msgRetry();
        QStringList btns;
        if (m->msg->getRetryCode() == static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendOccupied){
            btns.append(i18n("Send &urgent"));
        }else if (m->msg->getRetryCode() != static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendDND){
            return false;
        }
        btns.append(i18n("Send to &list"));
        btns.append(i18n("&Cancel"));
        QString err = i18n(m->msg->getError());
        Command cmd;
        cmd->id		= CmdSend;
        cmd->param	= m->edit;
        EventCommandWidget eWidget(cmd);
        eWidget.process();
        QWidget *msgWidget = eWidget.widget();
        if (msgWidget == NULL)
            msgWidget = m->edit;
        BalloonMsg *msg = new BalloonMsg(m, quoteString(err), btns, msgWidget, NULL, false);
        connect(msg, SIGNAL(action(int, void*)), this, SLOT(retry(int, void*)));
        msg->show();
        return true;
    }
    case eEventTemplateExpanded: {
        EventTemplate *et = static_cast<EventTemplate*>(e);
        EventTemplate::TemplateExpand *t = et->templateExpand();
        list<ar_request>::iterator it;
        for (it = arRequests.begin(); it != arRequests.end(); ++it)
            if (&(*it) == t->param)
                break;
        if (it == arRequests.end())
            return false;
        if (m_bAIM){
            if ((getState() == Connected) && (m_status == STATUS_AWAY)){
                if ((*it).bDirect){
                    setAwayMessage(t->tmpl);
                }else{
                    sendCapability(t->tmpl);
                    sendICMB(1, 11);
                    sendICMB(2,  3);
                    sendICMB(4,  3);
                    processSendQueue();
                    fetchProfiles();
                }
            }
            return true;
        }
        ar_request ar = (*it);
        if (ar.bDirect){
            Contact *contact;
            ICQUserData *data = findContact(ar.screen, NULL, false, contact);
            DirectClient *dc = dynamic_cast<DirectClient*>(data ? data->Direct.object() : 0);
            if (dc){
                QCString answer;
                if (data->Version.toULong() >= 10){
                    answer = t->tmpl.utf8();
                }else{
                    answer = getContacts()->fromUnicode(contact, t->tmpl);
                }
                dc->sendAck((unsigned short)(ar.id.id_l), ar.type, ar.flags, answer);
            }
        }else{
            Buffer copy;
            QCString response;
            response = t->tmpl.utf8();
            sendAutoReply(ar.screen, ar.id, plugins[PLUGIN_NULL],
                          ar.id1, ar.id2, ar.type, (char)(ar.ack), 0, response, 0, copy);
        }
        arRequests.erase(it);
        return true;
    }
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        switch(ec->action()) {
            case EventContact::eDeleted: {
                ICQUserData *data;
                ClientDataIterator it(contact->clientData, this);
                while ((data = (ICQUserData*)(++it)) != NULL){
                    if (data->IcqID.toULong() == 0)
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
                    lr.icq_id = (unsigned short)(data->IcqID.toULong());
                    lr.grp_id = (unsigned short)(data->GrpId.toULong());
                    lr.visible_id   = (unsigned short)(data->ContactVisibleId.toULong());
                    lr.invisible_id = (unsigned short)(data->ContactInvisibleId.toULong());
                    lr.ignore_id    = (unsigned short)(data->IgnoreId.toULong());
                    listRequests.push_back(lr);
                    processSendQueue();
                }
                removeBuddy(contact);
                break;
            }
            case EventContact::eChanged: {
                if (getState() == Connected){
                    if (!m_bAIM)
                        addBuddy(contact);
                    if (contact == getContacts()->owner()){
                        time_t now = time(NULL);
                        if (getContacts()->owner()->getPhones() != data.owner.PhoneBook.str()){
                            data.owner.PhoneBook.str() = getContacts()->owner()->getPhones();
                            data.owner.PluginInfoTime.asULong() = now;
                            sendPluginInfoUpdate(PLUGIN_PHONEBOOK);
                        }
                        if (getPicture() != data.owner.Picture.str()){
                            data.owner.Picture.str() = getPicture();
                            data.owner.PluginInfoTime.asULong() = now;
                            sendPluginInfoUpdate(PLUGIN_PICTURE);
                        }
                        if (getContacts()->owner()->getPhoneStatus() != data.owner.FollowMe.toULong()){
                            data.owner.FollowMe.asULong() = getContacts()->owner()->getPhoneStatus();
                            data.owner.PluginStatusTime.asULong() = now;
                            sendPluginStatusUpdate(PLUGIN_FOLLOWME, data.owner.FollowMe.toULong());
                        }
                        return false;
                    }
                    ICQUserData *data;
                    ClientDataIterator it(contact->clientData, this);
                    while ((data = (ICQUserData*)(++it)) != NULL){
                        if (data->Uin.toULong() || data->ProfileFetch.toBool())
                            continue;
                        fetchProfile(data);
                    }
                }
                addContactRequest(contact);
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventGroup: {
        EventGroup *ev = static_cast<EventGroup*>(e);
        Group *group = ev->group();
        if(!group->id())
            return false;
        switch(ev->action()) {
            case EventGroup::eChanged: 
                addGroupRequest(group);
                break;
            case EventGroup::eDeleted: {
                ICQUserData *data = (ICQUserData*)(group->clientData.getData(this));
                if (data){
                    ListRequest lr;
                    lr.type   = LIST_GROUP_DELETED;
                    lr.icq_id = (unsigned short)(data->IcqID.toULong());
                    listRequests.push_back(lr);
                    processSendQueue();
                }
                break;
            }
            case EventGroup::eAdded:
                return false;
        }
        break;
    }
    case eEventMessageCancel: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
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
                    DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
                    if (dc && dc->cancelMessage(msg))
                        return msg;
                }
            }
            if (m_send.msg == msg){
                m_send.msg = NULL;
                m_send.screen = "";
                m_sendTimer->stop();
                processSendQueue();
                return msg;
            }
            list<SendMsg>::iterator it;
            for (it = sendFgQueue.begin(); it != sendFgQueue.end(); ++it){
                if ((*it).msg == msg){
                    sendFgQueue.erase(it);
                    delete msg;
                    return msg;
                }
            }
            for (it = sendBgQueue.begin(); it != sendBgQueue.end(); ++it){
                if ((*it).msg == msg){
                    sendBgQueue.erase(it);
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
        break;
    }
    case eEventCheckState: {
        EventCheckState *ecs = static_cast<EventCheckState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdPhones){
            if (!m_bAIM)
                return true;
            return false;
        }
        if(cmd->id == CmdFetchAway) {
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            ClientDataIterator it(contact->clientData, this);
            ICQUserData *data;
            while ((data = (ICQUserData*)(++it)) != NULL){
                unsigned long status = STATUS_OFFLINE;
                unsigned style  = 0;
                QString statusIcon;
                contactInfo(data, status, style, statusIcon);
                if(status != STATUS_ONLINE && status != STATUS_OFFLINE) {
                    cmd->flags &= ~BTN_HIDE;
                    return true;
                }
            }
            return false;
        }
        if ((cmd->bar_id == ToolBarContainer) || (cmd->bar_id == BarHistory)){
            if (cmd->id == CmdChangeEncoding){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL){
                    cmd->flags |= BTN_HIDE;
                    return true;
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
                    return true;
                }
                return false;
            }
        }
        if (cmd->menu_id == MenuContactGroup){
            if (cmd->id == CmdVisibleList){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return false;
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
                    if (data->VisibleId.toULong())
                        cmd->flags |= COMMAND_CHECKED;
                }
                return (void*)bOK;
            }
            if (cmd->id == CmdInvisibleList){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return false;
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
                    if (data->InvisibleId.toULong())
                        cmd->flags |= COMMAND_CHECKED;
                }
                return (void*)bOK;
            }
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if(cmd->id == CmdFetchAway) {
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            ClientDataIterator it(contact->clientData, this);
            ICQUserData *data;
            while ((data = (ICQUserData*)(++it)) != NULL){
                unsigned long status = STATUS_OFFLINE;
                unsigned style  = 0;
                QString statusIcon;
                contactInfo(data, status, style, statusIcon);
                if(status != STATUS_ONLINE && status != STATUS_OFFLINE)
                    fetchAwayMessage(data);
            }
            cmd->flags &= ~COMMAND_CHECKED;
            return false;
        }
        if (cmd->menu_id == MenuContactGroup){
            if (cmd->id == CmdVisibleList){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return false;
                ICQUserData *data;
                ClientDataIterator it(contact->clientData);
                while ((data = (ICQUserData*)(++it)) != NULL){
                    data->VisibleId.asULong() = (cmd->flags & COMMAND_CHECKED) ? getListId() : 0;
                    EventContact eContact(contact, EventContact::eChanged);
                    eContact.process();
                }
                return true;
            }
            if (cmd->id == CmdInvisibleList){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return false;
                ICQUserData *data;
                ClientDataIterator it(contact->clientData);
                while ((data = (ICQUserData*)(++it)) != NULL){
                    data->InvisibleId.asULong() = (cmd->flags & COMMAND_CHECKED) ? getListId() : 0;
                    EventContact eContact(contact, EventContact::eChanged);
                    eContact.process();
                }
                return true;
            }
        }
        break;
    }
    case eEventGoURL: {
        EventGoURL *u = static_cast<EventGoURL*>(e);
        QString url = u->url();
        QString proto;
        int n = url.find(':');
        if (n < 0)
            return false;
        proto = url.left(n);
        if ((proto != "icq") && (proto != "aim"))
            return false;
        url = url.mid(proto.length() + 1);
        while (url.startsWith("/"))
            url = url.mid(1);
        QString s = unquoteString(url);
        QString screen = getToken(s, ',');
        if (!screen.isEmpty()){
            Contact *contact;
            findContact(screen, &s, true, contact);
            Command cmd;
            cmd->id		 = MessageGeneric;
            cmd->menu_id = MenuMessage;
            cmd->param	 = (void*)(contact->id());
            EventCommandExec(cmd).process();
            return true;
        }
        break;
    }
    case eEventOpenMessage: {
        if (getState() != Connected)
            return false;
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if ((msg->type() != MessageOpenSecure) &&
            (msg->type() != MessageCloseSecure) &&
            (msg->type() != MessageWarning))
            return false;
        QString client = msg->client();
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            return false;
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
            return false;
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
            return true;
        } else
        if (msg->type() == MessageWarning){
            if (data && (m_bAIM || (data->Uin.toULong() == 0))){
                WarnDlg *dlg = new WarnDlg(NULL, data, this);
                raiseWindow(dlg);
                return true;
            }
            return false;
        }
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if (dc && dc->isSecure()){
            Message *m = new Message(MessageCloseSecure);
            m->setContact(msg->contact());
            m->setClient(msg->client());
            m->setFlags(MESSAGE_NOHISTORY);
            if (!dc->sendMessage(m))
                delete m;
            return(void*)1;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

bool ICQClient::send(Message *msg, void *_data)
{
    if (getState() != Connected)
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    SendMsg s;
    switch (msg->type()){
    case MessageSMS:
        if (m_bAIM)
            return false;
        s.msg    = static_cast<SMSMessage*>(msg);
        s.text   = s.msg->getPlainText();
        s.flags  = SEND_1STPART;
        smsQueue.push_back(s);
        processSMSQueue();
        return true;
    case MessageAuthRequest:
        if (data && data->WaitAuth.toBool())
            return sendAuthRequest(msg, data);
        return false;
    case MessageAuthGranted:
        if (data && data->WantAuth.toBool())
            return sendAuthGranted(msg, data);
        return false;
    case MessageAuthRefused:
        if (data && data->WantAuth.toBool())
            return sendAuthRefused(msg, data);
        return false;
    case MessageFile:
        if (data && ((data->Status.toULong() & 0xFFFF) != ICQ_STATUS_OFFLINE)){
            if (data->Uin.toULong()){
                DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
                if (!dc){
                    if (data->bNoDirect.toBool())
                        return sendThruServer(msg, data);
                    dc = new DirectClient(data, this, PLUGIN_NULL);
                    data->Direct.setObject(dc);
                    dc->connect();
                }
                return dc->sendMessage(msg);
            }
            if (!hasCap(data, CAP_AIM_SENDFILE))
                return false;
            m_processMsg.push_back(msg);
            AIMFileTransfer *ft = new AIMFileTransfer(static_cast<FileMessage*>(msg), data, this);
            ft->listen();
            return true;
        }
        return false;
    case MessageTypingStop:
    case MessageTypingStart:
        if ((data == NULL) || getDisableTypingNotification())
            return false;
        if((data->Status.toULong() & 0xFFFF) == ICQ_STATUS_OFFLINE)
            return false;
        if (getInvisible()){
            if (data->VisibleId.toULong() == 0)
                return false;
        }else{
            if (data->InvisibleId.toULong())
                return false;
        }
        if (!hasCap(data, CAP_TYPING) && !hasCap(data, CAP_AIM_BUDDYCON))
            return false;
        sendMTN(screen(data), msg->type() == MessageTypingStart ? ICQ_MTN_START : ICQ_MTN_FINISH);
        delete msg;
        return true;
#ifdef USE_OPENSSL
    case MessageOpenSecure: {
        if (data == NULL)
            return false;
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if (dc && dc->isSecure())
            return false;
        if (!dc){
            dc = new DirectClient(data, this, PLUGIN_NULL);
            data->Direct.setObject(dc);
            dc->connect();
        }
        return dc->sendMessage(msg);
    }
    case MessageCloseSecure: {
        if (data == NULL)
            return false;
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if (dc && dc->isSecure())
            return dc->sendMessage(msg);
        return false;
    }
#endif
    case MessageWarning:
        return sendThruServer(msg, data);
    case MessageContacts:
        if ((data == NULL) || ((data->Uin.toULong() == 0) && !hasCap(data, CAP_AIM_BUDDYLIST)))
            return false;
    }
    if (data == NULL)
        return false;
    if (data->Uin.toULong()){
        bool bCreateDirect = false;
        DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
        if ((dc == NULL) &&
                !data->bNoDirect.toBool() &&
                (data->Status.toULong() != ICQ_STATUS_OFFLINE) &&
                (get_ip(data->IP) == get_ip(this->data.owner.IP)))
            bCreateDirect = true;
        if (!bCreateDirect &&
                (msg->type() == MessageGeneric) &&
                (data->Status.toULong() != ICQ_STATUS_OFFLINE) &&
                (get_ip(data->IP)) &&
                (msg->getPlainText().length() >= MAX_TYPE2_MESSAGE_SIZE))
            bCreateDirect = true;
        if ((getInvisible() && (data->VisibleId.toULong() == 0)) ||
                (!getInvisible() && data->InvisibleId.toULong()))
            bCreateDirect = false;
        if (bCreateDirect){
            dc = new DirectClient(data, this, PLUGIN_NULL);
            data->Direct.setObject(dc);
            dc->connect();
        }
        if (dc)
            return dc->sendMessage(msg);
    }
    return sendThruServer(msg, data);
}

bool ICQClient::canSend(unsigned type, void *_data)
{
    if (_data && (((clientData*)_data)->Sign.toULong() != ICQ_SIGN))
        return false;
    if (getState() != Connected)
        return false;
    ICQUserData *data = (ICQUserData*)_data;
    switch (type){
    case MessageSMS:
        return !m_bAIM;
    case MessageGeneric:
    case MessageUrl:
        return (data != NULL);
    case MessageContacts:
        return (data != NULL) && (data->Uin.toULong() || hasCap(data, CAP_AIM_BUDDYLIST));
    case MessageAuthRequest:
        return data && (data->WaitAuth.toBool());
    case MessageAuthGranted:
        return data && (data->WantAuth.toBool());
    case MessageFile:
        return data &&
               ((data->Status.toULong() & 0xFFFF) != ICQ_STATUS_OFFLINE) &&
               (data->Uin.toULong() || hasCap(data, CAP_AIM_SENDFILE));
    case MessageWarning:
        return data && (data->Uin.toULong() == 0);
#ifdef USE_OPENSSL
    case MessageOpenSecure:
        if ((data == NULL) || ((data->Status.toULong() & 0xFFFF) == ICQ_STATUS_OFFLINE))
            return false;
        if (hasCap(data, CAP_LICQ) ||
                hasCap(data, CAP_SIM) ||
                hasCap(data, CAP_SIMOLD) ||
                ((data->InfoUpdateTime.toULong() & 0xFF7F0000L) == 0x7D000000L)){
            DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
            if (dc)
                return !(dc->isSecure());
            return get_ip(data->IP) || get_ip(data->RealIP);
        }
        return false;
    case MessageCloseSecure: {
            if(!data)
                return false;
            DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
            return dc && dc->isSecure();
        }
#endif
    }
    return false;
}

QString ICQClient::dataName(void *data)
{
    return dataName(screen((ICQUserData*)data));
}

QString ICQClient::dataName(const QString &screen)
{
    return name() + '.' + screen;
}

QString ICQClient::screen(const ICQUserData *data)
{
    if (data->Uin.toULong() == 0)
        return data->Screen.str();
    return QString::number(data->Uin.toULong());
}

bool ICQClient::messageReceived(Message *msg, const QString &screen)
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
            contact->setFlags(CONTACT_TEMP);
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
        msg->setClient(dataName(data));
        msg->setContact(contact->id());
        if (data->bTyping.toBool()){
            data->bTyping.asBool() = false;
            EventContact e(contact, EventContact::eStatus);;
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
    EventMessageReceived e(msg);
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

QString ICQClient::contactName(void *clientData)
{
    QString res;
    ICQUserData *data = (ICQUserData*)clientData;
    res = data->Uin.toULong() ? "ICQ: " : "AIM: ";
    if (!data->Nick.str().isEmpty()){
        res += data->Nick.str();
        res += " (";
    }
    res += data->Uin.toULong() ? QString::number(data->Uin.toULong()) : data->Screen.str();
    if (!data->Nick.str().isEmpty())
        res += ')';
    return res;
}

bool ICQClient::isSupportPlugins(ICQUserData *data)
{
    if (data->Version.toULong() < 7)
        return false;
    switch (data->InfoUpdateTime.toULong()){
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
    if ((data->InfoUpdateTime.toULong() & 0xFF7F0000L) == 0x7D000000L)
        return false;
    if (hasCap(data, CAP_TRIL_CRYPT) || hasCap(data, CAP_TRILLIAN))
        return false;
    return true;
}

void ICQClient::addPluginInfoRequest(unsigned long uin, unsigned plugin_index)
{
    Contact *contact;
    ICQUserData *data = findContact(uin, NULL, false, contact);
    if (data && !data->bNoDirect.toBool() &&
            get_ip(data->IP) && (get_ip(data->IP) == get_ip(this->data.owner.IP)) &&
            ((getInvisible() && data->VisibleId.toULong()) ||
             (!getInvisible() && (data->InvisibleId.toULong() == 0)))){
        switch (plugin_index){
        case PLUGIN_AR: {
                DirectClient *dc = dynamic_cast<DirectClient*>(data->Direct.object());
                if ((dc == NULL) && !getHideIP()){
                    dc = new DirectClient(data, this, PLUGIN_NULL);
                    data->Direct.setObject(dc);
                    dc->connect();
                }
                if (dc){
                    dc->addPluginInfoRequest(plugin_index);
                    return;
                }
                break;
            }
        case PLUGIN_QUERYxINFO:
        case PLUGIN_PHONEBOOK:
        case PLUGIN_PICTURE: {
                if (!isSupportPlugins(data))
                    return;
                DirectClient *dc = dynamic_cast<DirectClient*>(data->DirectPluginInfo.object());
                if ((dc == NULL) && !getHideIP()){
                    dc = new DirectClient(data, this, PLUGIN_INFOxMANAGER);
                    data->DirectPluginInfo.setObject(dc);
                    dc->connect();
                }
                if (dc){
                    dc->addPluginInfoRequest(plugin_index);
                    return;
                }
                break;
            }
        case PLUGIN_QUERYxSTATUS:
        case PLUGIN_FILESERVER:
        case PLUGIN_FOLLOWME:
        case PLUGIN_ICQPHONE: {
                if (!isSupportPlugins(data))
                    return;
                DirectClient *dc = dynamic_cast<DirectClient*>(data->DirectPluginStatus.object());
                if ((dc == NULL) && !getHideIP()){
                    dc = new DirectClient(data, this, PLUGIN_STATUSxMANAGER);
                    data->DirectPluginStatus.setObject(dc);
                    dc->connect();
                }
                if (dc){
                    dc->addPluginInfoRequest(plugin_index);
                    return;
                }
                break;
            }
        }
    }
    list<SendMsg>::iterator it;
    for (it = sendBgQueue.begin(); it != sendBgQueue.end(); ++it){
        SendMsg &s = *it;
        if ((s.screen.toULong() == uin) && (s.flags == plugin_index) && (s.msg == NULL))
            break;
    }
    if (it != sendBgQueue.end())
        return;
    SendMsg s;
    s.screen = QString::number(uin);
    s.flags  = plugin_index;
    sendBgQueue.push_back(s);
    processSendQueue();
}

void ICQClient::randomChatInfo(unsigned long uin)
{
    addPluginInfoRequest(uin, PLUGIN_RANDOMxCHAT);
}

unsigned short ICQClient::msgStatus()
{
    return (unsigned short)(fullStatus(getStatus()) & 0xFF);
}

static char PICT_PATH[] = "pictures/";

QString ICQClient::pictureFile(const ICQUserData *data)
{
    QString f = PICT_PATH;
    f += "icq.avatar.";
    f += data->Uin.toULong() ? QString::number(data->Uin.toULong()) : data->Screen.str();
    f += '.';
    f += QString::number(data->buddyID.toULong());
    f = user_file(f);
    return f;
}

QImage ICQClient::userPicture(unsigned id)
{
    if (id==0)
        return QImage();
    Contact *contact = getContacts()->contact(id);
    if(!contact)
        return QImage();
    ClientDataIterator it(contact->clientData, this);

    ICQUserData *d;
    while ((d = static_cast<ICQUserData*>(++it)) != NULL){
        QImage img = userPicture(d);
        if(!img.isNull())
            return img;
    }
    return QImage();
}

QImage ICQClient::userPicture(ICQUserData *d)
{
    QImage img=QImage(d ? pictureFile(d) : data.owner.Picture.str());

    if(img.isNull())
        return img;

    int w = img.width();
    int h = img.height();
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

   return img.scale(w, h);
}


void ICQClient::retry(int n, void *p)
{
    EventMessageRetry::MsgSend *m = reinterpret_cast<EventMessageRetry::MsgSend*>(p);
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
    EventCommandExec(cmd).process();
}

bool ICQClient::isOwnData(const QString &screen)
{
    if (screen.isEmpty())
        return false;
    if(data.owner.Uin.toULong())
        return (data.owner.Uin.toULong() == screen.toULong());
    return (screen.lower() == data.owner.Screen.str().lower());
}

QString ICQClient::addCRLF(const QString &str)
{
    QString res = str;
    return res.replace(QRegExp("\r?\n"), "\r\n");
}

Contact *ICQClient::getContact(ICQUserData *data)
{
    Contact *contact = NULL;
    findContact(screen(data), NULL, false, contact);
    return contact;
}

#ifndef NO_MOC_INCLUDES
#include "icqclient.moc"
#endif

