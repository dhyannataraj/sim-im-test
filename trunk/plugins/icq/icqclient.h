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

const unsigned MESSAGE_DIRECT	= 0x0100;

const unsigned STATUS_OCCUPIED	= 0x10;
const unsigned STATUS_FFC		= 0x11;

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
const unsigned short ICQ_SNACxFAM_LOGIN			   = 0x0017;

// Status
const unsigned short ICQ_STATUS_OFFLINE            = 0xFFFF;
const unsigned short ICQ_STATUS_ONLINE             = 0x0000;
const unsigned short ICQ_STATUS_AWAY               = 0x0001;
const unsigned short ICQ_STATUS_DND                = 0x0002;
const unsigned short ICQ_STATUS_NA                 = 0x0004;
const unsigned short ICQ_STATUS_OCCUPIED           = 0x0010;
const unsigned short ICQ_STATUS_FFC				   = 0x0020;

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
const unsigned short ICQ_MSGxEXT			   = 0x001A;

const unsigned short ICQ_MSGxSECURExCLOSE	   = 0x00EE;
const unsigned short ICQ_MSGxSECURExOPEN	   = 0x00EF;

const unsigned short ICQ_MSGxAR_AWAY		   = 0x03E8;
const unsigned short ICQ_MSGxAR_OCCUPIED	   = 0x03E9;
const unsigned short ICQ_MSGxAR_NA			   = 0x03EA;
const unsigned short ICQ_MSGxAR_DND			   = 0x03EB;
const unsigned short ICQ_MSGxAR_FFC			   = 0x03EC;

const unsigned short ICQ_MTN_FINISH	= 0x0000;
const unsigned short ICQ_MTN_TYPED	= 0x0001;
const unsigned short ICQ_MTN_START	= 0x0002;

const char DIRECT_MODE_DENIED	= 0x01;
const char DIRECT_MODE_INDIRECT	= 0x02;
const char DIRECT_MODE_DIRECT	= 0x04;

const unsigned MAX_MESSAGE_SIZE = 450;

const unsigned PING_TIMEOUT = 60;

class DirectClient;

typedef struct ICQUserData
{
    char			*Alias;
    char			*Cellular;
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
    unsigned long	ContactVisibleId;
    unsigned long	InvisibleId;
    unsigned long	ContactInvisibleId;
    unsigned long	WaitAuth;
    unsigned long	WantAuth;
    unsigned long	WebAware;
    unsigned long	InfoUpdateTime;
    unsigned long	PluginInfoTime;
    unsigned long	PluginStatusTime;
    unsigned long	InfoFetchTime;
    unsigned long	PluginInfoFetchTime;
    unsigned long	PluginStatusFetchTime;
    unsigned long	Mode;
    unsigned long	Version;
    unsigned long	Build;
    char			*Nick;
    char			*FirstName;
    char			*LastName;
    char			*EMail;
    unsigned long	HiddenEMail;
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
    char			*WorkZip;
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
    unsigned long	FollowMe;
    unsigned long	SharedFiles;
    unsigned long	ICQPhone;
    char			*Encoding;
    char			*Picture;
    unsigned long	PictureWidth;
    unsigned long	PictureHeight;
    char			*PhoneBook;
    unsigned long	bTyping;
    unsigned long	bBadClient;
    DirectClient	*Direct;
    DirectClient	*DirectPluginInfo;
    DirectClient	*DirectPluginStatus;
    unsigned long	bNoDirect;
    unsigned long	bInvisible;
} ICQUserData;

typedef struct ICQClientData
{
    char		*Server;
    unsigned	Port;
    unsigned	ContactsTime;
    unsigned	ContactsLength;
    unsigned	ContactsInvisible;
    unsigned	HideIP;
    unsigned	IgnoreAuth;
    unsigned	DirectMode;
    unsigned	IdleTime;
    char		*ListRequests;
    char		*Picture;
    unsigned	RandomChatGroup;
    unsigned	RandomChatGroupCurrent;
    unsigned	SendFormat;
    unsigned	AutoUpdate;
    unsigned	TypingNotification;
    ICQUserData	owner;
} ICQClientData;

class ICQClient;

typedef struct SearchResult
{
    ICQUserData		data;
    unsigned short	id;
    ICQClient		*client;
} SearchResult;

class QTimer;

typedef unsigned char capability[0x10];
typedef unsigned char plugin[0x12];

const unsigned CAP_DIRECT		= 0;
const unsigned CAP_RTF			= 1;
const unsigned CAP_SRV_RELAY	= 2;
const unsigned CAP_UTF			= 3;
const unsigned CAP_TYPING		= 4;
const unsigned CAP_SIM			= 5;
const unsigned CAP_STR_2001		= 6;
const unsigned CAP_IS_2001		= 7;
const unsigned CAP_TRILLIAN		= 8;
const unsigned CAP_TRIL_CRYPT	= 9;
const unsigned CAP_MACICQ		= 10;
const unsigned CAP_AIM_CHAT		= 11;
const unsigned CAP_MICQ			= 12;
const unsigned CAP_LICQ			= 13;
const unsigned CAP_SIMOLD		= 14;
const unsigned CAP_NULL			= 15;

const unsigned PLUGIN_PHONEBOOK			= 0;
const unsigned PLUGIN_PICTURE			= 1;
const unsigned PLUGIN_FILESERVER		= 2;
const unsigned PLUGIN_FOLLOWME			= 3;
const unsigned PLUGIN_ICQPHONE			= 4;
const unsigned PLUGIN_QUERYxINFO		= 5;
const unsigned PLUGIN_QUERYxSTATUS		= 6;
const unsigned PLUGIN_INFOxMANAGER		= 7;
const unsigned PLUGIN_STATUSxMANAGER	= 8;
const unsigned PLUGIN_RANDOMxCHAT		= 9;
const unsigned PLUGIN_NULL				= 10;
const unsigned PLUGIN_FILE				= 11;
const unsigned PLUGIN_CHAT				= 12;
const unsigned PLUGIN_AR				= 13;
const unsigned PLUGIN_INVISIBLE			= 14;
const unsigned PLUGIN_REVERSE			= 15;

class ICQClient;

typedef struct ENCODING
{
    const char *language;
    const char *codec;
    int			mib;
    bool		bMain;
} ENCODING;

const unsigned LIST_USER_CHANGED	= 0;
const unsigned LIST_USER_DELETED	= 1;
const unsigned LIST_GROUP_CHANGED	= 2;
const unsigned LIST_GROUP_DELETED	= 3;

typedef struct ListRequest
{
    unsigned		type;
    unsigned long	uin;
    unsigned short	icq_id;
    unsigned short	grp_id;
    unsigned short	visible_id;
    unsigned short	invisible_id;
    unsigned short	ignore_id;
} ListRequest;

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
    unsigned long	id_l;
    unsigned long	id_h;
    MessageId() : id_l(0), id_h(0) {}
} MessageId;

bool operator == (const MessageId &m1, const MessageId &m2);

typedef struct SendMsg
{
    unsigned long	uin;
    MessageId		id;
    Message			*msg;
    QString			text;
    QString			part;
    unsigned		flags;
    DirectSocket	*socket;
    SendMsg() : msg(NULL), socket(NULL), uin(0) {}
} SendMsg;

const unsigned SEND_PLAIN	= 0x0001;
const unsigned SEND_UTF		= 0x0002;
const unsigned SEND_RTF		= 0x0003;
const unsigned SEND_RAW		= 0x0004;
const unsigned SEND_2GO		= 0x0005;
const unsigned SEND_MASK	= 0x000F;
const unsigned SEND_1STPART	= 0x0010;

typedef struct rtf_charset
{
    const char *name;
    int			rtf_code;
} charset;

typedef struct ar_request
{
    unsigned short	type;
    MessageId		id;
    unsigned short	id1;
    unsigned short	id2;
    unsigned long	uin;
    bool			bDirect;
} ar_request;

class DirectSocket;

class ICQClient : public TCPClient, public EventReceiver
{
    Q_OBJECT
public:
    ICQClient(ICQProtocol*, const char *cfg);
    ~ICQClient();
    virtual string name();
    virtual QWidget	*setupWnd();
    virtual string getConfig();
    virtual unsigned getStatus();
    void setUin(unsigned long);
    unsigned long getUin();
    PROP_STR(Server);
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
    PROP_BOOL(TypingNotification);
    ICQClientData	data;
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
    ICQUserData *findContact(unsigned long uin, const char *alias, bool bCreate, Contact *&contact, Group *grp=NULL);
    ICQUserData *findGroup(unsigned id, const char *name, Group *&group);
    void addFullInfoRequest(unsigned long uin, bool bInLast = true);
    ListRequest *findContactListRequest(unsigned long uin);
    ListRequest *findGroupListRequest(unsigned short id);
    virtual void setupContact(Contact*, void *data);
    string clientName(ICQUserData*);
    void sendStatus();
    void sendUpdate();
    void changePassword(const char *new_pswd);
    void searchChat(unsigned short);
    void randomChatInfo(unsigned long uin);
    virtual string dataName(void*);
    Message *parseMessage(unsigned short type, unsigned long uin,
                          string &p, Buffer &packet,
                          unsigned short cookie1, unsigned short cookie2,
                          MessageId id);
    void messageReceived(Message*, unsigned long);
    static QTextCodec *_getCodec(const char *encoding);
    static QString toUnicode(const char *serverText, const char *clientName, unsigned contactId);
    static QString parseRTF(const char *str, const char *encoding);
    static QString pictureFile(ICQUserData *data);
    static const capability *capabilities;
    static const plugin *plugins;
    static rtf_charset rtf_charsets[];
protected slots:
    void ping();
    void infoRequest();
    void infoRequestFail();
    void processSendQueue();
protected:
    virtual void setInvisible(bool bState);
    virtual void setStatus(unsigned status, bool bCommon);
    virtual void setStatus(unsigned status);
    virtual void disconnected();
    virtual void *processEvent(Event*);
    string dataName(unsigned long uin);
    virtual bool compareData(void*, void*);
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, const char *&statusIcon, string *icons = NULL);
    virtual bool send(Message*, void*);
    virtual bool canSend(unsigned type, void*);
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
    bool			m_bHeader;
    char			m_nChannel;
    Buffer			m_cookie;
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
    void sendCapability();
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
    unsigned long fullStatus(unsigned status);
    string cryptPassword();
    virtual void connect_ready();
    virtual void packet_ready();
    void sendPacket();
    void flap(char channel);
    void snac(unsigned short fam, unsigned short type, bool msgId=false);
    unsigned short m_nSequence;
    unsigned short m_nMsgSequence;
    ICQListener *m_listener;
    QTimer *m_sendTimer;
    QTimer *m_infoTimer;
    unsigned short m_infoRequestId;
    unsigned short m_sendSmsId;
    ListServerRequest *m_listRequest;
    bool m_bRosters;
    bool m_bBirthday;
    bool m_bServerReady;
    list<ServerRequest*> varRequests;
    list<unsigned long> infoRequests;
    list<unsigned long> buddies;
    list<ListRequest>	listRequests;
    list<SendMsg>		smsQueue;
    list<SendMsg>		sendQueue;
    list<SendMsg>		replyQueue;
    list<ar_request>	arRequests;
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
    bool hasCap(ICQUserData *data, int cap);
    string trimPhone(const char *phone);
    QTextCodec *getCodec(const char *encoding);
    unsigned short getListId();
    TlvList *createListTlv(ICQUserData *data, Contact *contact);
    unsigned short sendRoster(unsigned short cmd, const char *name,
                              unsigned short grp_id,  unsigned short usr_id,
                              unsigned short subCmd=0, TlvList *tlv = NULL);
    unsigned short sendRoster(unsigned short cmd, unsigned long uin,
                              unsigned short grp_id,  unsigned short usr_id,
                              unsigned short subCmd=0, TlvList *tlv = NULL);
    void sendRosterGrp(const char *name, unsigned short grpId, unsigned short usrId);
    bool isContactRenamed(ICQUserData *data, Contact *contact);
    bool sendThruServer(Message *msg, void *data);
    string getUserCellular(Contact *contact);
    void setMainInfo(ICQUserData *d);
    void packInfoList(char *str);
    string createRTF(const char *text, unsigned long foreColor, const char *encoding);
    void ackMessage();
    void sendThroughServer(unsigned long uin, unsigned short type, Buffer &b, unsigned long id_l, unsigned long id_h, bool bOffline);
    bool sendAuthRequest(Message *msg, void *data);
    bool sendAuthGranted(Message *msg, void *data);
    bool sendAuthRefused(Message *msg, void *data);
    void sendAdvMessage(unsigned long uin, Buffer &msgText, unsigned plugin_index, const MessageId &id, bool bOffline, bool bPeek, bool bDirect);
    void sendType2(unsigned long uin, Buffer &msgBuf, const MessageId &id, unsigned cap, bool bOffline, bool bPeek, bool bDirect);
    void parseAdvancedMessage(unsigned long uin, Buffer &msg, bool needAck, MessageId id);
    void sendAutoReply(unsigned long uin, MessageId id,
                       const plugin p, unsigned short cookie1, unsigned short cookie2,
                       unsigned char msgType, unsigned char msgFlags, unsigned long msgState,
                       const char *response, unsigned short response_type, Buffer &copy);
    void addPluginInfoRequest(unsigned long uin, unsigned plugin_index);
    void sendMTN(unsigned long uin, unsigned short type);
    void setChatGroup();
    Message *parseExtendedMessage(unsigned long uin, Buffer &packet);
    void parsePluginPacket(Buffer &b, unsigned plugin_index, ICQUserData *data, unsigned uin, bool bDirect);
    void pluginAnswer(unsigned plugin_type, unsigned long uin, Buffer &b);
    void packMessage(Buffer &b, Message *msg, ICQUserData *data, unsigned short &type, unsigned short nSequence);
    void requestReverseConnection(unsigned long uin, DirectSocket *socket);
    unsigned short m_advCounter;
    unsigned m_nUpdates;
    unsigned m_nSendTimeout;
    SendMsg  m_send;
    list<Message*>		m_processMsg;
    list<DirectSocket*>	m_sockets;
    friend class FullInfoRequest;
    friend class SMSRequest;
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

class DirectSocket : public ClientSocketNotify
{
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
protected:
    virtual void processPacket() = 0;
    void init();
    void sendInit();
    void sendInitAck();
    void removeFromClient();
    bool			m_bHeader;
    bool			m_bIncoming;
    unsigned short	m_nSequence;
    unsigned short	m_port;
    char			m_version;
    unsigned long	m_nSessionId;
    ICQUserData		*m_data;
    ClientSocket	*m_socket;
    ICQClient		*m_client;
    //    DirectListener	*m_listener;
};

typedef struct SendDirectMsg
{
    Message		*msg;
    unsigned	type;
    unsigned short	seq;
    unsigned short	icq_type;
} SendDirectMsg;

class DirectClient : public DirectSocket
{
public:
    DirectClient(Socket *s, ICQClient *client);
    DirectClient(ICQUserData *data, ICQClient *client, unsigned channel = PLUGIN_NULL);
    ~DirectClient();
    bool sendMessage(Message*);
    void sendAutoResponse(unsigned short seq, unsigned short type, const char *answer);
    void acceptMessage(Message*);
    void declineMessage(Message*, const char *reason);
    bool cancelMessage(Message*);
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
    State		m_state;
    unsigned	m_channel;
    void processPacket();
    void connect_ready();
    bool error_state(const char *err, unsigned code);
    void sendInit2();
    void startPacket(unsigned short cms, unsigned short seq);
    void startMsgPacket(unsigned short msgType, const string &s);
    void sendPacket();
    void sendAck(unsigned short, unsigned short msgType, const char *message=NULL);
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

