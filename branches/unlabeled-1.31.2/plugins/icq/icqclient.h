/***************************************************************************
                          icqclient.h  -  description
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

#ifndef _ICQCLIENT_H
#define _ICQCLIENT_H

#include "simapi.h"
#include "socket.h"
#include "icq.h"

#include <map>
using namespace std;

const unsigned ICQ_SIGN			= 0x0001;

const unsigned MESSAGE_DIRECT    = 0x0100;

const unsigned STATUS_INVISIBLE	  = 2;
const unsigned STATUS_OCCUPIED    = 100;

const unsigned char ICQ_TCP_VERSION = 0x08;

const unsigned char MODE_DENIED   = 0x01;
const unsigned char MODE_INDIRECT = 0x02;
const unsigned char MODE_DIRECT   = 0x04;

// Server channels
const unsigned char ICQ_CHNxNEW                    = 0x01;
const unsigned char ICQ_CHNxDATA                   = 0x02;
const unsigned char ICQ_CHNxERROR                  = 0x03;
const unsigned char ICQ_CHNxCLOSE                  = 0x04;
const unsigned char ICQ_CHNxPING                   = 0x05;

// Server SNAC families
const unsigned short ICQ_SNACxFAM_SERVICE          = 0x0001;
const unsigned short ICQ_SNACxFAM_LOCATION         = 0x0002;
const unsigned short ICQ_SNACxFAM_BUDDY            = 0x0003;
const unsigned short ICQ_SNACxFAM_MESSAGE          = 0x0004;
const unsigned short ICQ_SNACxFAM_BOS              = 0x0009;
const unsigned short ICQ_SNACxFAM_PING             = 0x000B;
const unsigned short ICQ_SNACxFAM_LISTS            = 0x0013;
const unsigned short ICQ_SNACxFAM_VARIOUS          = 0x0015;
const unsigned short ICQ_SNACxFAM_LOGIN            = 0x0017;

// Status
const unsigned short ICQ_STATUS_OFFLINE            = 0xFFFF;
const unsigned short ICQ_STATUS_ONLINE             = 0x0000;
const unsigned short ICQ_STATUS_AWAY               = 0x0001;
const unsigned short ICQ_STATUS_DND                = 0x0002;
const unsigned short ICQ_STATUS_NA                 = 0x0004;
const unsigned short ICQ_STATUS_OCCUPIED           = 0x0010;
const unsigned short ICQ_STATUS_FFC                = 0x0020;

const unsigned long ICQ_STATUS_FxFLAGS             = 0xFFFF0000;
const unsigned long ICQ_STATUS_FxUNKNOWNxFLAGS     = 0xCFC0FCC8;
const unsigned long ICQ_STATUS_FxPRIVATE           = 0x00000100;
const unsigned long ICQ_STATUS_FxPFMxAVAILABLE     = 0x00000200;  // not implemented
const unsigned long ICQ_STATUS_FxWEBxPRESENCE      = 0x00010000;
const unsigned long ICQ_STATUS_FxHIDExIP           = 0x00020000;
const unsigned long ICQ_STATUS_FxPFM               = 0x00040000;  // not implemented
const unsigned long ICQ_STATUS_FxBIRTHDAY          = 0x00080000;
const unsigned long ICQ_STATUS_FxDIRECTxDISABLED   = 0x00100000;
const unsigned long ICQ_STATUS_FxICQxHOMEPAGE      = 0x00200000;  // not implemented
const unsigned long ICQ_STATUS_FxDIRECTxLISTED     = 0x20000000;  // will accept connectio only when listed
const unsigned long ICQ_STATUS_FxDIRECTxAUTH       = 0x10000000;  // will accept connectio only when authorized

const unsigned CLASS_UNCONFIRMED	= 0x0001;	// AOL unconfirmed user flsg
const unsigned CLASS_ADMINISTRATOR	= 0x0002;   // AOL administrator flag
const unsigned CLASS_AOL			= 0x0004;	// AOL staff user flag
const unsigned CLASS_COMMERCIAL     = 0x0008;	// AOL commercial account flag
const unsigned CLASS_FREE			= 0x0010;	// ICQ non-commercial account flag
const unsigned CLASS_AWAY			= 0x0020;	// Away status flag
const unsigned CLASS_ICQ			= 0x0040;	// ICQ user sign
const unsigned CLASS_WIRELESS		= 0x0100;	// AOL wireless user

const unsigned short ICQ_MSGxMSG               = 0x0001;
const unsigned short ICQ_MSGxCHAT              = 0x0002;
const unsigned short ICQ_MSGxFILE              = 0x0003;
const unsigned short ICQ_MSGxURL               = 0x0004;
const unsigned short ICQ_MSGxAUTHxREQUEST      = 0x0006;
const unsigned short ICQ_MSGxAUTHxREFUSED      = 0x0007;
const unsigned short ICQ_MSGxAUTHxGRANTED      = 0x0008;
const unsigned short ICQ_MSGxADDEDxTOxLIST     = 0x000C;
const unsigned short ICQ_MSGxWEBxPANEL         = 0x000D;
const unsigned short ICQ_MSGxEMAILxPAGER       = 0x000E;
const unsigned short ICQ_MSGxCONTACTxLIST      = 0x0013;
const unsigned short ICQ_MSGxEXT               = 0x001A;

const unsigned short ICQ_MSGxSECURExCLOSE       = 0x00EE;
const unsigned short ICQ_MSGxSECURExOPEN       = 0x00EF;

const unsigned short ICQ_MSGxAR_AWAY           = 0x03E8;
const unsigned short ICQ_MSGxAR_OCCUPIED       = 0x03E9;
const unsigned short ICQ_MSGxAR_NA               = 0x03EA;
const unsigned short ICQ_MSGxAR_DND               = 0x03EB;
const unsigned short ICQ_MSGxAR_FFC               = 0x03EC;

const unsigned short ICQ_TCPxACK_ONLINE         = 0x0000;
const unsigned short ICQ_TCPxACK_AWAY           = 0x0004;
const unsigned short ICQ_TCPxACK_OCCUPIED       = 0x0009;
const unsigned short ICQ_TCPxACK_DND            = 0x000A;
const unsigned short ICQ_TCPxACK_OCCUPIEDxCAR   = 0x000B;
const unsigned short ICQ_TCPxACK_OCCUPIEDx2        = 0x000C;
const unsigned short ICQ_TCPxACK_NA             = 0x000E;
const unsigned short ICQ_TCPxACK_DNDxCAR        = 0x000F;
const unsigned short ICQ_TCPxACK_ACCEPT         = 0x0000;
const unsigned short ICQ_TCPxACK_REFUSE         = 0x0001;

const unsigned short ICQ_TCPxMSG_AUTOxREPLY     = 0x0000;
const unsigned short ICQ_TCPxMSG_NORMAL            = 0x0001;
const unsigned short ICQ_TCPxMSG_URGENT         = 0x0002;
const unsigned short ICQ_TCPxMSG_LIST           = 0x0004;

const unsigned short ICQ_MTN_FINISH    = 0x0000;
const unsigned short ICQ_MTN_TYPED    = 0x0001;
const unsigned short ICQ_MTN_START    = 0x0002;

const char DIRECT_MODE_DENIED    = 0x01;
const char DIRECT_MODE_INDIRECT    = 0x02;
const char DIRECT_MODE_DIRECT    = 0x04;

const unsigned MAX_MESSAGE_SIZE = 450;

const unsigned PING_TIMEOUT = 60;

const unsigned short SEARCH_DONE = (unsigned short)(-1);

class DirectClient;

typedef struct ICQUserData
{
    clientData		base;
    char            *Alias;
    char            *Cellular;
    unsigned long    Status;
    unsigned long	 Class;
    unsigned long    StatusTime;
    unsigned long    OnlineTime;
    unsigned long	 WarningLevel;
    void            *IP;
    void            *RealIP;
    unsigned long    Port;
    unsigned long    DCcookie;
    unsigned long    Caps;
    char            *AutoReply;
    unsigned long    Uin;
    char            *Screen;
    unsigned long    IcqID;
    unsigned long    bChecked;
    unsigned long    GrpId;
    unsigned long    IgnoreId;
    unsigned long    VisibleId;
    unsigned long    ContactVisibleId;
    unsigned long    InvisibleId;
    unsigned long    ContactInvisibleId;
    unsigned long    WaitAuth;
    unsigned long    WantAuth;
    unsigned long    WebAware;
    unsigned long    InfoUpdateTime;
    unsigned long    PluginInfoTime;
    unsigned long    PluginStatusTime;
    unsigned long    InfoFetchTime;
    unsigned long    PluginInfoFetchTime;
    unsigned long    PluginStatusFetchTime;
    unsigned long    Mode;
    unsigned long    Version;
    unsigned long    Build;
    char            *Nick;
    char            *FirstName;
    char            *LastName;
    char			*MiddleName;
    char			*Maiden;
    char            *EMail;
    unsigned long    HiddenEMail;
    char            *City;
    char            *State;
    char            *HomePhone;
    char            *HomeFax;
    char            *Address;
    char            *PrivateCellular;
    char            *Zip;
    unsigned long    Country;
    unsigned long    TimeZone;
    unsigned long    Age;
    unsigned long    Gender;
    char            *Homepage;
    unsigned long    BirthYear;
    unsigned long    BirthMonth;
    unsigned long    BirthDay;
    unsigned long    Language;
    char            *EMails;
    char            *WorkCity;
    char            *WorkState;
    char            *WorkPhone;
    char            *WorkFax;
    char            *WorkAddress;
    char            *WorkZip;
    unsigned long    WorkCountry;
    char            *WorkName;
    char            *WorkDepartment;
    char            *WorkPosition;
    unsigned long    Occupation;
    char            *WorkHomepage;
    char            *About;
    char            *Interests;
    char            *Backgrounds;
    char            *Affilations;
    unsigned long    FollowMe;
    unsigned long    SharedFiles;
    unsigned long    ICQPhone;
    char            *Encoding;
    char            *Picture;
    unsigned long    PictureWidth;
    unsigned long    PictureHeight;
    char            *PhoneBook;
    unsigned long	 ProfileFetch;
    unsigned long    bTyping;
    unsigned long    bBadClient;
    DirectClient    *Direct;
    DirectClient    *DirectPluginInfo;
    DirectClient    *DirectPluginStatus;
    unsigned long    bNoDirect;
    unsigned long    bInvisible;
} ICQUserData;

typedef struct ICQClientData
{
    char        *Server;
    unsigned    Port;
    unsigned    ContactsTime;
    unsigned    ContactsLength;
    unsigned    ContactsInvisible;
    unsigned    HideIP;
    unsigned    IgnoreAuth;
    unsigned    DirectMode;
    unsigned    IdleTime;
    char        *ListRequests;
    char        *Picture;
    unsigned    RandomChatGroup;
    unsigned    RandomChatGroupCurrent;
    unsigned    SendFormat;
    unsigned    AutoUpdate;
    unsigned    AutoReplyUpdate;
    unsigned    TypingNotification;
    unsigned    AcceptInDND;
    unsigned    AcceptInOccupied;
    unsigned	MinPort;
    unsigned	MaxPort;
    unsigned	WarnAnonimously;
    ICQUserData    owner;
} ICQClientData;

class ICQClient;

typedef struct SearchResult
{
    ICQUserData        data;
    unsigned short    id;
    ICQClient        *client;
} SearchResult;

class QTimer;

typedef unsigned char capability[0x10];
typedef unsigned char plugin[0x12];

const unsigned CAP_DIRECT		= 0;
const unsigned CAP_RTF          = 1;
const unsigned CAP_SRV_RELAY    = 2;
const unsigned CAP_UTF          = 3;
const unsigned CAP_TYPING       = 4;
const unsigned CAP_AIM_SUPPORT  = 5;
const unsigned CAP_SIM          = 6;
const unsigned CAP_STR_2001     = 7;
const unsigned CAP_IS_2001      = 8;
const unsigned CAP_TRILLIAN     = 9;
const unsigned CAP_TRIL_CRYPT   = 10;
const unsigned CAP_MACICQ       = 11;
const unsigned CAP_AIM_CHAT     = 12;
const unsigned CAP_AIM_BUDDYCON = 13;
const unsigned CAP_AIM_IMIMAGE  = 14;
const unsigned CAP_AIM_SENDFILE = 15;
const unsigned CAP_MICQ         = 16;
const unsigned CAP_LICQ         = 17;
const unsigned CAP_SIMOLD       = 18;
const unsigned CAP_NULL         = 19;

const unsigned PLUGIN_PHONEBOOK            = 0;
const unsigned PLUGIN_PICTURE            = 1;
const unsigned PLUGIN_FILESERVER        = 2;
const unsigned PLUGIN_FOLLOWME            = 3;
const unsigned PLUGIN_ICQPHONE            = 4;
const unsigned PLUGIN_QUERYxINFO        = 5;
const unsigned PLUGIN_QUERYxSTATUS        = 6;
const unsigned PLUGIN_INFOxMANAGER        = 7;
const unsigned PLUGIN_STATUSxMANAGER    = 8;
const unsigned PLUGIN_RANDOMxCHAT        = 9;
const unsigned PLUGIN_NULL                = 10;
const unsigned PLUGIN_FILE                = 11;
const unsigned PLUGIN_CHAT                = 12;
const unsigned PLUGIN_AR                = 13;
const unsigned PLUGIN_INVISIBLE            = 14;
const unsigned PLUGIN_REVERSE            = 15;

class ICQClient;

typedef struct ENCODING
{
    const char *language;
    const char *codec;
    int         mib;
    int			rtf_code;
    bool        bMain;
} ENCODING;

const unsigned LIST_USER_CHANGED    = 0;
const unsigned LIST_USER_DELETED    = 1;
const unsigned LIST_GROUP_CHANGED    = 2;
const unsigned LIST_GROUP_DELETED    = 3;

class ListRequest
{
public:
    ListRequest() : type(0),icq_id(0),grp_id(0),visible_id(0),invisible_id(0),ignore_id(0) {}

public:
    unsigned        type;
    string            screen;
    unsigned short    icq_id;
    unsigned short    grp_id;
    unsigned short    visible_id;
    unsigned short    invisible_id;
    unsigned short    ignore_id;
};

class ICQListener : public ServerSocketNotify
{
public:
    ICQListener(ICQClient *client);
    ~ICQListener();
    bool created() { return (m_socket != NULL); }
    unsigned short port();
protected:
    virtual void accept(Socket *s, unsigned long ip);
    ServerSocket *m_socket;
    ICQClient  *m_client;
};

class DirectSocket;
class ServerRequest;
class ListServerRequest;
class QTextCodec;

typedef struct MessageId
{
    unsigned long    id_l;
    unsigned long    id_h;
    MessageId() : id_l(0), id_h(0) {}
} MessageId;

bool operator == (const MessageId &m1, const MessageId &m2);

typedef struct SendMsg
{
    string			screen;
    MessageId		id;
    Message			*msg;
    QString			text;
    QString			part;
    unsigned		flags;
    DirectSocket    *socket;
    SendMsg() : msg(NULL), socket(NULL) {}
} SendMsg;

const unsigned SEND_PLAIN		= 0x0001;
const unsigned SEND_UTF			= 0x0002;
const unsigned SEND_RTF			= 0x0003;
const unsigned SEND_RAW			= 0x0004;
const unsigned SEND_2GO			= 0x0005;
const unsigned SEND_HTML		= 0x0006;
const unsigned SEND_MASK		= 0x000F;
const unsigned SEND_1STPART		= 0x0010;

typedef struct ar_request
{
    unsigned short    type;
    unsigned short    flags;
    unsigned short    ack;
    MessageId        id;
    unsigned short    id1;
    unsigned short    id2;
    string            screen;
    bool            bDirect;
} ar_request;

typedef map<unsigned short, string> INFO_REQ_MAP;

class DirectSocket;
class ServiceSocket;

class OscarSocket
{
public:
    OscarSocket();
    virtual ~OscarSocket();
protected:
    virtual ClientSocket *socket() = 0;
    virtual void packet() = 0;
    void sendPacket();
    void flap(char channel);
    void snac(unsigned short fam, unsigned short type, bool msgId=false, bool bType=true);
    void connect_ready();
    void packet_ready();
    bool m_bHeader;
    char m_nChannel;
    unsigned short m_nSequence;
    unsigned short m_nMsgSequence;
};

class ICQClient : public TCPClient, public EventReceiver, public OscarSocket
{
    Q_OBJECT
public:
    ICQClient(Protocol*, const char *cfg, bool bAIM);
    ~ICQClient();
    virtual string name();
    virtual QWidget    *setupWnd();
    virtual string getConfig();
    virtual unsigned getStatus();
    void setUin(unsigned long);
    void setScreen(const char*);
    unsigned long getUin();
    const char *getServer();
    void setServer(const char*);
    PROP_ULONG(Port);
    PROP_ULONG(ContactsTime);
    PROP_ULONG(ContactsLength);
    PROP_USHORT(ContactsInvisible);
    PROP_BOOL(HideIP);
    PROP_BOOL(IgnoreAuth);
    PROP_ULONG(DirectMode);
    PROP_ULONG(IdleTime);
    PROP_STR(ListRequests);
    PROP_UTF8(Picture);
    PROP_ULONG(RandomChatGroup);
    PROP_ULONG(RandomChatGroupCurrent);
    PROP_ULONG(SendFormat);
    PROP_BOOL(AutoUpdate);
    PROP_BOOL(AutoReplyUpdate);
    PROP_BOOL(TypingNotification);
    PROP_BOOL(AcceptInDND);
    PROP_BOOL(AcceptInOccupied);
    PROP_ULONG(MinPort);
    PROP_ULONG(MaxPort);
    PROP_BOOL(WarnAnonimously);
    ICQClientData    data;
    QString toUnicode(const char *str, ICQUserData *client_data);
    string fromUnicode(const QString &str, ICQUserData *client_data);
    unsigned short findByUin(unsigned long uin);
    unsigned short findWP(const char *first, const char *last, const char *nick,
                          const char *email, char age, char nGender,
                          char nLanguage, const char *city, const char *szState,
                          unsigned short nCountryCode,
                          const char *cCoName, const char *szCoDept, const char *szCoPos,
                          char nOccupation,
                          unsigned short nPast, const char *szPast,
                          unsigned short nInterests, const char *szInterests,
                          unsigned short nAffiliation, const char *szAffiliation,
                          unsigned short nHomePoge, const char *szHomePage,
                          bool bOnlineOnly);
    ICQUserData *findContact(const char *screen, const char *alias, bool bCreate, Contact *&contact, Group *grp=NULL);
    ICQUserData *findGroup(unsigned id, const char *name, Group *&group);
    void addFullInfoRequest(unsigned long uin, bool bInLast = true);
    ListRequest *findContactListRequest(const char *screen);
    ListRequest *findGroupListRequest(unsigned short id);
    void removeListRequest(ListRequest *lr);
    virtual void setupContact(Contact*, void *data);
    string clientName(ICQUserData*);
    void sendStatus();
    void sendUpdate();
    void changePassword(const char *new_pswd);
    void searchChat(unsigned short);
    void randomChatInfo(unsigned long uin);
    unsigned aimEMailSearch(const char *name);
    unsigned aimInfoSearch(const char *first, const char *last, const char *middle,
                           const char *maiden, const char *country, const char *street,
                           const char *city, const char *nick, const char *zip,
                           const char *state);
    virtual string dataName(void*);
    Message *parseMessage(unsigned short type, const char *screen,
                          string &p, Buffer &packet);
    bool messageReceived(Message*, const char *screen);
    QTextCodec *getCodec(const char *encoding);
    static QTextCodec *_getCodec(const char *encoding);
    static QString toUnicode(const char *serverText, const char *clientName, unsigned contactId);
    static bool parseRTF(const char *str, const char *encoding, QString &result);
    static QString pictureFile(ICQUserData *data);
    static const capability *capabilities;
    static const plugin *plugins;
    static const ENCODING *encodings;
    static QString convert(Tlv *tlvInfo, TlvList &tlvs, unsigned n);
    string screen(ICQUserData*);
    static unsigned warnLevel(unsigned short);
	static QString addCRLF(const QString &str);
    bool m_bAIM;
protected slots:
    void ping();
    void infoRequest();
    void infoRequestFail();
    void processSendQueue();
    void retry(int n, void*);
protected:
    virtual void setInvisible(bool bState);
    virtual void setStatus(unsigned status, bool bCommon);
    virtual void setStatus(unsigned status);
    virtual void disconnected();
    virtual void *processEvent(Event*);
    virtual bool compareData(void*, void*);
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, const char *&statusIcon, string *icons = NULL);
    virtual bool send(Message*, void*);
    virtual bool canSend(unsigned type, void*);
    virtual bool isMyData(clientData*&, Contact*&);
    virtual bool createData(clientData*&, Contact*);
    virtual QString contactTip(void *_data);
    virtual CommandDef *infoWindows(Contact *contact, void *_data);
    virtual QWidget *infoWindow(QWidget *parent, Contact *contact, void *_data, unsigned id);
    virtual CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual QWidget *searchWindow();
    virtual void updateInfo(Contact *contact, void *_data);
    virtual void setClientInfo(void *data);
    virtual QString ownerName();
    virtual QString contactName(void *clientData);
    string  dataName(const char *screen);
    Buffer            m_cookie;
    virtual ClientSocket *socket();
    virtual void packet();
    void snac_service(unsigned short, unsigned short);
    void snac_location(unsigned short, unsigned short);
    void snac_buddy(unsigned short, unsigned short);
    void snac_icmb(unsigned short, unsigned short);
    void snac_bos(unsigned short, unsigned short);
    void snac_ping(unsigned short, unsigned short);
    void snac_lists(unsigned short, unsigned short);
    void snac_various(unsigned short, unsigned short);
    void snac_login(unsigned short, unsigned short);
    void chn_login();
    void chn_close();
    void listsRequest();
    void locationRequest();
    void buddyRequest();
    void icmbRequest();
    void bosRequest();
    void sendCapability(const char *msg=NULL);
    void sendICMB(unsigned short channel, unsigned long flags);
    void sendLogonStatus();
    void sendClientReady();
    void sendMessageRequest();
    void serverRequest(unsigned short cmd, unsigned short seq=0);
    void sendServerRequest();
    void sendVisibleList();
    void sendInvisibleList();
    void sendContactList();
    void setInvisible();
    void setOffline(ICQUserData*);
    void fillDirectInfo(Buffer &directInfo);
    void removeFullInfoRequest(unsigned long uin);
    void send(bool bTimer);
    void requestService(ServiceSocket*);
    unsigned long fullStatus(unsigned status);
    string cryptPassword();
    virtual void connect_ready();
    virtual void packet_ready();
    const char* error_message(unsigned short error);
    ICQListener		*m_listener;
    list<ServiceSocket*> m_services;
    QTimer *m_sendTimer;
    QTimer *m_infoTimer;
    unsigned short m_infoRequestId;
    unsigned short m_sendSmsId;
    unsigned short m_offlineMessagesRequestId;
    ListServerRequest *m_listRequest;
    bool m_bRosters;
    bool m_bBirthday;
    bool m_bServerReady;
    list<ServerRequest*> varRequests;
    list<unsigned long>  infoRequests;
    list<string>		 buddies;
    list<ListRequest>    listRequests;
    list<SendMsg>        smsQueue;
    list<SendMsg>        sendQueue;
    list<SendMsg>        replyQueue;
    list<ar_request>    arRequests;
    void addGroupRequest(Group *group);
    void addContactRequest(Contact *contact);
    void addBuddy(Contact *contact);
    ServerRequest *findServerRequest(unsigned short id);
    void clearServerRequests();
    void clearListServerRequest();
    void clearSMSQueue();
    void clearMsgQueue();
    void processListRequest();
    void processSMSQueue();
    void infoRequestPause();
    void sendIdleTime();
    void sendPluginInfoUpdate(unsigned plugin_id);
    void sendPluginStatusUpdate(unsigned plugin_id, unsigned long status);
    bool m_bIdleTime;
    bool hasCap(ICQUserData *data, int fcap);
    string trimPhone(const char *phone);
    unsigned short getListId();
    TlvList *createListTlv(ICQUserData *data, Contact *contact);
    unsigned short sendRoster(unsigned short cmd, const char *name,
                              unsigned short grp_id,  unsigned short usr_id,
                              unsigned short subCmd=0, TlvList *tlv = NULL);
    void sendRosterGrp(const char *name, unsigned short grpId, unsigned short usrId);
    bool isContactRenamed(ICQUserData *data, Contact *contact);
    bool sendThruServer(Message *msg, void *data);
    string getUserCellular(Contact *contact);
    void setMainInfo(ICQUserData *d);
    void setAIMInfo(ICQUserData *data);
    void setProfile(ICQUserData *data);
    bool isOwnData(const char *screen);
    void packInfoList(char *str);
    string createRTF(const QString &text, unsigned long foreColor, const char *encoding);
    QString removeImages(const QString &text, unsigned maxSmile);
    void ackMessage(SendMsg &s);
    void sendThroughServer(const char *screen, unsigned short type, Buffer &b, const MessageId &id, bool bOffline);
    bool sendAuthRequest(Message *msg, void *data);
    bool sendAuthGranted(Message *msg, void *data);
    bool sendAuthRefused(Message *msg, void *data);
    void sendAdvMessage(const char *screen, Buffer &msgText, unsigned plugin_index, const MessageId &id, bool bOffline, bool bPeek, bool bDirect);
    void sendType2(const char *screen, Buffer &msgBuf, const MessageId &id, unsigned cap, bool bOffline, bool bPeek, bool bDirect);
    void sendType1(const QString &text, bool bWide, ICQUserData *data);
    void parseAdvancedMessage(const char *screen, Buffer &msg, bool needAck, MessageId id);
    void sendAutoReply(const char *screen, MessageId id,
                       const plugin p, unsigned short cookie1, unsigned short cookie2,
                       unsigned short  msgType, char msgFlags, unsigned short msgState,
                       const char *response, unsigned short response_type, Buffer &copy);
    void addPluginInfoRequest(unsigned long uin, unsigned plugin_index);
    void sendMTN(const char *screen, unsigned short type);
    void setChatGroup();
    Message *parseExtendedMessage(const char *screen, Buffer &packet);
    void parsePluginPacket(Buffer &b, unsigned plugin_index, ICQUserData *data, unsigned uin, bool bDirect);
    void pluginAnswer(unsigned plugin_type, unsigned long uin, Buffer &b);
    void packMessage(Buffer &b, Message *msg, ICQUserData *data, unsigned short &type, unsigned short nSequence);
    void requestReverseConnection(const char *screen, DirectSocket *socket);
    bool ackMessage(Message *msg, unsigned short ackFlags, const char *str);
    void fetchProfile(ICQUserData *data);
    void fetchAwayMessage(ICQUserData *data);
    void fetchProfiles();
    void setAwayMessage(const char *msg);
    void encodeString(const QString &text, const char *type, unsigned charsetTlv, unsigned infoTlv);
    void encodeString(const char *_str, unsigned nTlv, bool bWide);
    ICQUserData *findInfoRequest(unsigned short seq, Contact *&contact);
    INFO_REQ_MAP m_info_req;
    QString clearTags(const QString &text);
    string clearTags(const char *text);
    unsigned short msgStatus();
    unsigned short m_advCounter;
    unsigned m_nUpdates;
    unsigned m_nSendTimeout;
    SendMsg  m_send;
    list<Message*>     	m_processMsg;
    list<DirectSocket*>	m_sockets;
    list<Message*>		m_acceptMsg;
    friend class FullInfoRequest;
    friend class SMSRequest;
    friend class ServiceSocket;
    friend class DirectSocket;
    friend class DirectClient;
    friend class ICQListener;
};

/*
class DirectListener : public ServerSocketNotify
{
public:
    DirectListener(DirectSocket *dsock);
    ~DirectListener();
    bool created() { return (m_socket != NULL); }
    unsigned short port();
protected:
    virtual void accept(Socket*, unsigned long ip);
    ServerSocket *m_socket;
    DirectSocket *m_dsock;
};
*/

class ServiceSocket : public ClientSocketNotify, public OscarSocket
{
public:
    ServiceSocket(ICQClient*, unsigned short id);
    ~ServiceSocket();
    unsigned short id() { return m_id; }
    void connect(const char *addr, unsigned short port, const char *cookie, unsigned cookie_size);
    virtual bool error_state(const char *err, unsigned code);
    bool connected() { return m_bConnected; }
protected:
    virtual void connect_ready();
    virtual void packet_ready();
    virtual ClientSocket *socket();
    virtual void packet();
    virtual void data(unsigned short fam, unsigned short type, unsigned short seq) = 0;
    unsigned short m_id;
    Buffer  m_cookie;
    bool    m_bConnected;
    ClientSocket *m_socket;
    ICQClient *m_client;
};

class DirectSocket : public QObject, public ClientSocketNotify
{
    Q_OBJECT
public:
    enum SocketState{
        NotConnected,
        ConnectIP1,
        ConnectIP2,
        ConnectFail,
        WaitInit,
        WaitAck,
        WaitReverse,
        ReverseConnect,
        Logged
    };
    DirectSocket(Socket *s, ICQClient*);
    DirectSocket(ICQUserData *data, ICQClient *client);
    ~DirectSocket();
    virtual void packet_ready();
    SocketState m_state;
    void connect();
    void reverseConnect(unsigned long ip, unsigned short port);
    void acceptReverse(Socket *s);
    virtual bool error_state(const char *err, unsigned code);
    virtual void connect_ready();
    unsigned short localPort();
    unsigned short remotePort();
    unsigned long Uin();
protected slots:
    void timeout();
protected:
    virtual void processPacket() = 0;
    void init();
    void sendInit();
    void sendInitAck();
    void removeFromClient();
    bool            m_bIncoming;
    unsigned short    m_nSequence;
    unsigned short    m_port;
    char            m_version;
    bool			m_bHeader;
    unsigned long    m_nSessionId;
    ICQUserData        *m_data;
    ClientSocket    *m_socket;
    ICQClient        *m_client;
    //    DirectListener    *m_listener;
};

typedef struct SendDirectMsg
{
    Message        *msg;
    unsigned    type;
    unsigned short    seq;
    unsigned short    icq_type;
} SendDirectMsg;

class DirectClient : public DirectSocket
{
public:
    DirectClient(Socket *s, ICQClient *client);
    DirectClient(ICQUserData *data, ICQClient *client, unsigned channel = PLUGIN_NULL);
    ~DirectClient();
    bool sendMessage(Message*);
    void acceptMessage(Message*);
    void declineMessage(Message*, const char *reason);
    bool cancelMessage(Message*);
    void sendAck(unsigned short, unsigned short msgType, unsigned short msgFlags, const char *message=NULL);
    bool isLogged() { return (m_state != None) && (m_state != WaitInit2); }
    bool isSecure();
    void addPluginInfoRequest(unsigned plugin_index);
protected:
    enum State{
        None,
        WaitLogin,
        WaitInit2,
        Logged,
        SSLconnect
    };
    State        m_state;
    unsigned    m_channel;
    void processPacket();
    void connect_ready();
    bool error_state(const char *err, unsigned code);
    void sendInit2();
    void startPacket(unsigned short cms, unsigned short seq);
    void startMsgPacket(unsigned short msgType, const string &s);
    void sendPacket();
    void processMsgQueue();
    bool copyQueue(DirectClient *to);
    list<SendDirectMsg> m_queue;
    const char *name();
    string m_name;
#ifdef USE_OPENSSL
    void secureConnect();
    void secureListen();
    void secureStop(bool bShutdown);
    SSLClient *m_ssl;
#endif
};

class ICQFileTransfer : public FileTransfer, public DirectSocket
{
public:
    ICQFileTransfer(FileMessage *msg, ICQUserData *data, ICQClient *client);
    ~ICQFileTransfer();
    void connect(unsigned short port);
    void setSocket(ClientSocket *socket);
protected:
    enum State
    {
        None,
        WaitLogin,
        WaitInit,
        InitSend,
        InitReceive,
        Send,
        Receive,
        Wait
    };
    QFile *m_f;
    State m_state;

    unsigned m_sendTime;
    unsigned m_sendSize;
    unsigned m_transfer;

    virtual void processPacket();
    virtual void connect_ready();
    virtual bool error_state(const char *err, unsigned code);
    virtual void write_ready();
    virtual void setSpeed(unsigned speed);

    void init();
    void sendInit();
    void startPacket(char cmd);
    void sendPacket(bool dump=true);
    void sendFileInfo();
};

#endif

