/***************************************************************************
                          icqclient.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#if HAVE_INTTYPES_H
#include <inttypes.h>
#else
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#pragma warning(disable:4355)
#pragma warning(disable:4786)
#endif
#endif

#include <list>
#include <vector>
#include <string>

#include "socket.h"
#include "cfg.h"

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
const unsigned short ICQ_SNACxFAM_LOGIN		   = 0x0017;

// Status
const unsigned short ICQ_STATUS_OFFLINE            = 0xFFFF;
const unsigned short ICQ_STATUS_ONLINE             = 0x0000;
const unsigned short ICQ_STATUS_AWAY               = 0x0001;
const unsigned short ICQ_STATUS_DND                = 0x0002;
const unsigned short ICQ_STATUS_NA                 = 0x0004;
const unsigned short ICQ_STATUS_OCCUPIED           = 0x0010;
const unsigned short ICQ_STATUS_FREEFORCHAT        = 0x0020;

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
const unsigned short ICQ_MSGxMAIL			   = 0x00ED;
const unsigned short ICQ_MSGxSMS               = 0x00EC;
const unsigned short ICQ_MSGxSMSxRECEIPT       = 0x00EE;
const unsigned short ICQ_MSGxCONTACTxREQUEST   = 0x00EF;

const unsigned long  UIN_SPECIAL    = 0xF0000000L;

const unsigned short USER_TYPE_ICQ	= 0;
const unsigned short USER_TYPE_EXT  = 1;

class ICQGroup : public ConfigArray
{
public:
    ICQGroup();
    ConfigString Name;
    ConfigUShort Id;
    ConfigBool   Expand;
    bool bChecked;
};

class EMailInfo : public ConfigArray
{
public:
    EMailInfo();
    ConfigString Email;
    ConfigBool	 Hide;
    ConfigBool	 MyInfo;
};

class EMailPtrList : public ConfigPtrList
{
public:
    EMailPtrList() {}
    ConfigArray *create() { return new EMailInfo; }
    const ConfigPtrList &operator = (const ConfigPtrList &v)
    { return ConfigPtrList::operator =(v); }
};

class EMailList : public ConfigList
{
public:
    EMailList(ConfigArray *arr, const char *name) : ConfigList(arr, name) {}
    ConfigArray *create() { return new EMailInfo; }
    const ConfigPtrList &operator = (const ConfigPtrList &v)
    { return ConfigPtrList::operator =(v); }
    EMailList &operator() () { return *this; }
    bool operator == (const EMailList &b) const { return ConfigList::operator == (b); }
    bool operator != (const EMailList &b) const { return ConfigList::operator != (b); }
};

class ExtInfo : public ConfigArray
{
public:
    ExtInfo();
    ConfigUShort	Category;
    ConfigString	Specific;
};

class ExtInfoPtrList : public ConfigPtrList
{
public:
    ExtInfoPtrList() {}
    ConfigArray *create() { return new ExtInfo; }
    const ConfigPtrList &operator = (const ConfigPtrList &v)
    { return ConfigPtrList::operator = (v); }
};

class ExtInfoList : public ConfigList
{
public:
    ExtInfoList(ConfigArray *arr, const char *name) : ConfigList(arr, name) {}
    ConfigArray *create() { return new ExtInfo; }
    ExtInfoList &operator() () { return *this; }
    const ConfigPtrList &operator = (const ConfigPtrList &v)
    { return ConfigPtrList::operator = (v); }
    bool operator == (const ExtInfoList &b) const { return ConfigList::operator == (b); }
    bool operator != (const ExtInfoList &b) const { return ConfigList::operator != (b); }
};

const unsigned long PHONE = 0;
const unsigned long MOBILE = 1;
const unsigned long SMS = 2;
const unsigned long FAX = 3;
const unsigned long PAGER = 4;

class PhoneInfo : public ConfigArray
{
public:
    PhoneInfo();
    ConfigString	Name;
    ConfigULong		Type;
    ConfigBool		Active;
    ConfigString	Country;
    ConfigString	AreaCode;
    ConfigString	Number;
    ConfigString	Extension;
    ConfigString	Provider;
    ConfigString	Gateway;
    ConfigBool		Publish;
    ConfigBool		FromInfo;
    ConfigBool		MyInfo;
    string getNumber();
    void setNumber(const string &number, unsigned long type);
    bool isEqual(const char *number);
};

class PhonePtrList : public ConfigPtrList
{
public:
    PhonePtrList() {}
    ConfigArray *create() { return new PhoneInfo; }
};

class PhoneBook : public ConfigList
{
public:
    PhoneBook(ConfigArray *arr, const char *name) : ConfigList(arr, name) {}
    ConfigArray *create() { return new PhoneInfo; }
    void add(const char *number, const char *name, unsigned long type, bool bMyInfo);
    void add(const PhonePtrList &l);
    bool operator == (const PhoneBook &b) const { return ConfigList::operator == (b); }
    bool operator != (const PhoneBook &b) const { return ConfigList::operator != (b); }
};

class ICQUser;
class ICQClient;
class ICQMessage;

// ________________________________________________________________________________________

const int EVENT_STATUS_CHANGED		= 1;
const int EVENT_MESSAGE_RECEIVED	= 2;
const int EVENT_MESSAGE_SEND		= 3;
const int EVENT_INFO_CHANGED		= 4;
const int EVENT_BAD_PASSWORD		= 5;
const int EVENT_ANOTHER_LOCATION    = 6;
const int EVENT_SEARCH				= 7;
const int EVENT_RATE_LIMIT			= 8;
const int EVENT_LOGIN_ERROR			= 9;
const int EVENT_USER_DELETED		= 10;
const int EVENT_USERGROUP_CHANGED   = 11;
const int EVENT_GROUP_CHANGED		= 12;
const int EVENT_GROUP_CREATED		= 13;
const int EVENT_AUTH_REQUIRED		= 14;
const int EVENT_ACKED				= 15;
const int EVENT_DONE			    = 16;
const int EVENT_CHAT				= 17;

const int EVENT_SUBTYPE_FULLINFO	= 1;

class ICQEvent
{
public:
    enum State
    {
        Unknown,
        Success,
        Fail
    };
    ICQEvent(int type, unsigned long uin = 0, unsigned long subbtype = 0, ICQMessage *msg = NULL);
    virtual int type() { return m_nType; }
    int subType() { return m_nSubType; }
    virtual ~ICQEvent() {}
    ICQMessage *message() { return msg; }
    unsigned long Uin() { return m_nUin; }
    State state;
    void setType(int type) { m_nType = type; }
    string text;
    unsigned long info;
protected:
    int m_nType;
    int m_nSubType;
    unsigned short m_nId;
    unsigned long  m_nUin;
    ICQMessage *msg;
    virtual bool processAnswer(ICQClient*, Buffer &b, unsigned short nSubtype);
    virtual void failAnswer(ICQClient*);
    friend class ICQClient;
};

const char SEARCH_STATE_OFFLINE  = 0;
const char SEARCH_STATE_ONLINE   = 1;
const char SEARCH_STATE_DISABLED = 2;

class SearchEvent : public ICQEvent
{
public:
    SearchEvent(unsigned short id);
    string nick;
    string firstName;
    string lastName;
    string email;
    char auth;
    char state;
    bool lastResult;
protected:
    bool processAnswer(ICQClient*, Buffer&, unsigned short nSubtype);
};

// ___________________________________________________________________________________

class ICQFile;
class ICQChat;

class DirectSocket : public ClientSocket
{
public:
    enum SocketState{
        NotConnected,
        ConnectIP1,
        ConnectIP2,
        WaitInit,
        WaitAck,
        Logged
    };
    DirectSocket(int fd, const char *host, unsigned short port, ICQClient*);
    DirectSocket(unsigned long real_ip, unsigned long ip, unsigned short port, ICQUser*, ICQClient*);
    virtual void packet_ready();
    SocketState state;
    void connect();
    virtual void error_state();
    virtual void connect_ready();
protected:
    virtual void processPacket() = 0;
    virtual void connected() = 0;
    void init();
    void sendInit();
    void sendInitAck();
    bool m_bHeader;
    ICQClient *client;
    bool m_bIncoming;
    bool m_bUseInternalIP;
    unsigned short m_nSequence;
    char version;
    unsigned long m_nSessionId;
    unsigned long m_packetOffs;
    unsigned long real_ip;
    unsigned long ip;
    unsigned short port;
    unsigned long uin;
    unsigned long DCcookie;
};

class ICQListener : public ServerSocket
{
public:
    ICQListener(ICQClient *client);
    virtual void accept(int fd, const char *host, unsigned short port);
protected:
    ICQClient *client;
};

class DirectClient : public DirectSocket
{
public:
    DirectClient(int fd, const char *host, unsigned short port, ICQClient *client);
    ~DirectClient();
    void acceptMessage(ICQMessage*);
    void declineMessage(ICQMessage*, const char *reason);
protected:
    enum State{
        None,
        WaitInit2,
        Logged
    };
    void processPacket();
    void connected();
    State state;
    ICQUser *u;
    void sendInit2();
    void startPacket(unsigned short cms, unsigned short seq);
    void sendPacket();
    void sendAck(unsigned short);
};

class FileTransferListener : public ServerSocket
{
public:
    FileTransferListener(ICQFile *file, ICQClient *client);
    virtual void accept(int fd, const char *host, unsigned short port);
protected:
    ICQClient *client;
    ICQFile *file;
};

class FileTransfer : public DirectSocket
{
public:
    FileTransfer(int fd, const char *host, unsigned short port, ICQClient *client, ICQFile *file);
    FileTransfer(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClient *client, ICQFile *file);
    void resume(int mode);
    int  speed() { return m_nSpeed; }
    void setSpeed(int nSpeed);
protected:
    enum State
    {
        None,
        WaitInit,
        InitSend,
        InitReceive,
        Send,
        Receive,
        Wait
    };

    ICQFile *file;
    void processPacket();
    void connected();
    void error_state();
    void write_ready();
    bool have_data();

    void startPacket(char cmd);
    void sendPacket(bool dump=true);

    State state;
    unsigned long m_packetOffs;
    unsigned long m_nSpeed;

    unsigned sendTime;
    unsigned sendSize;
};

class ChatListener : public ServerSocket
{
public:
    ChatListener(ICQChat *chat, ICQClient *client);
    virtual void accept(int fd, const char *host, unsigned short port);
protected:
    ICQClient *client;
    ICQChat *chat;
};

const char CHAT_ESCAPE		= 0x00;
const char CHAT_COLORxFG    = 0x00;
const char CHAT_COLORxBG    = 0x01;
const char CHAT_FOCUSxIN    = 0x03;
const char CHAT_FOCUSxOUT   = 0x04;
const char CHAT_BEEP		= 0x07;
const char CHAT_BACKSPACE	= 0x08;
const char CHAT_NEWLINE		= 0x0D;
const char CHAT_FONT_FACE	= 0x11;

const char CHAT_CONNECT		= (char)0xFE;
const char CHAT_TEXT		= (char)0xFF;

const unsigned long FONT_PLAIN     = 0x00000000;
const unsigned long FONT_BOLD      = 0x00000001;
const unsigned long FONT_ITALIC    = 0x00000002;
const unsigned long FONT_UNDERLINE = 0x00000004;

class ChatClient
{
public:
    ChatClient() {}
    unsigned long uin;
    string alias;
    unsigned long bgColor, fdColor;
    string font;
};

class ChatSocket : public DirectSocket
{
public:
    ChatSocket(int fd, const char *host, unsigned short port, ICQClient *client, ICQChat *file);
    ChatSocket(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClient *client, ICQChat *chat);

    void sendLine(const char *str);

    string fontFamily;
    unsigned long fontSize;
    unsigned long fontFace;
    unsigned long bgColor, fgColor;

protected:
    list<ChatClient> clients;
    ICQChat *chat;

    enum State
    {
        None,
        WaitInit,
        WaitFontInfo,
        WaitFont,
        Connected
    };
    State state;

    void init();

    unsigned long myFontFace;
    unsigned long curMyFontFace;
    unsigned long myFgColor;
    unsigned long curMyFgColor;

    void error_state();
    void startPacket();
    void sendPacket();
    void read_ready();

    void putText(string &s);

    virtual void processPacket();
    virtual void connected();
};

// ____________________________________________________________________________________________

class ICQUser : public ConfigArray
{
public:
    ICQUser();
    ~ICQUser();

    virtual bool load(istream &s, string &nextPart);

    string name(bool useUin = true);

    unsigned    NewMessages;
    unsigned    LastMessageType;

    ConfigUShort Type;
    ConfigString Alias;
    ConfigUShort Id;
    ConfigUShort GrpId;
    ConfigULong  Uin;
    ConfigBool   inIgnore;
    ConfigBool   inVisible;
    ConfigBool   inInvisible;
    ConfigBool   WaitAuth;

    ConfigString AutoResponseAway;
    ConfigString AutoResponseNA;
    ConfigString AutoResponseDND;
    ConfigString AutoResponseOccupied;
    ConfigString AutoResponseFFC;

    string AutoReply;

    unsigned long uStatus;
    unsigned long prevStatus;
    bool GetRTF;
    bool CanPlugin;
    bool CanResponse;

    ConfigULongs unreadMsgs;

    ConfigULong	LastActive;
    ConfigULong	OnlineTime;
    ConfigULong StatusTime;

    ConfigULong	 IP;
    ConfigUShort Port;
    ConfigULong	 RealIP;

    ConfigString HostName;
    ConfigString RealHostName;

    ConfigUShort Version;
    ConfigUShort Mode;

    // General info
    ConfigString Nick;
    ConfigString FirstName;
    ConfigString LastName;
    ConfigString City;
    ConfigString State;
    ConfigString Address;
    ConfigString Zip;
    ConfigUShort Country;
    ConfigChar   TimeZone;
    ConfigString HomePhone;
    ConfigString HomeFax;
    ConfigString PrivateCellular;
    ConfigString EMail;
    ConfigBool   HiddenEMail;

    ConfigString Notes;

    // Email info
    EMailList	 EMails;

    // More info
    ConfigChar   Age;
    ConfigChar   Gender;
    ConfigString Homepage;
    ConfigUShort BirthYear;
    ConfigChar   BirthMonth;
    ConfigChar   BirthDay;
    ConfigChar   Language1;
    ConfigChar   Language2;
    ConfigChar   Language3;

    // Company info
    ConfigString WorkCity;
    ConfigString WorkState;
    ConfigString WorkZip;
    ConfigString WorkAddress;
    ConfigString WorkName;
    ConfigString WorkDepartment;
    ConfigString WorkPosition;
    ConfigUShort WorkCountry;
    ConfigUShort Occupation;
    ConfigString WorkHomepage;
    ConfigString WorkPhone;
    ConfigString WorkFax;

    // About info
    ConfigString About;

    // Background info
    ExtInfoList Backgrounds;
    ExtInfoList Affilations;

    // Personal interests info
    ExtInfoList Interests;

    // PhoneBook
    PhoneBook	 Phones;
    ConfigChar	 PhoneState;
    ConfigULong	 PhoneBookTime;
    ConfigULong  PhoneStatusTime;

    bool bMyInfo;

    // Alert mode
    ConfigBool	 AlertOverride;
    ConfigBool	 AlertAway;
    ConfigBool	 AlertBlink;
    ConfigBool	 AlertSound;
    ConfigBool	 AlertOnScreen;
    ConfigBool	 AlertPopup;
    ConfigBool	 AlertWindow;

    // Accept mode
    ConfigUShort AcceptFileMode;
    ConfigBool	 AcceptFileOverride;
    ConfigBool	 AcceptFileOverwrite;
    ConfigString AcceptFilePath;
    ConfigString DeclineFileMessage;

    void adjustPhones();
    void adjustEMails(const ConfigPtrList &mails);

    void setOffline();
    unsigned long DCcookie;

    DirectClient *direct;

    bool isOnline();
    bool bPhoneChanged;
};

class ICQClient;

class ICQContactList : public ConfigArray
{
public:
    ICQContactList(ICQClient*);
    ~ICQContactList();
    ConfigUShort Len;
    ConfigULong  Time;
    ConfigBool   Expand;
    list<ICQUser*>  users;
    vector<ICQGroup*> groups;
    unsigned short getUserId(ICQUser *u);
    unsigned short getGroupId(ICQGroup *g);
    unsigned long findByEmail(const string &name, const string &email);
    unsigned long findByPhone(const string &number);
protected:
    ICQClient *client;
    ICQGroup *getGroup(unsigned short id, bool create = true);
    ICQUser  *getUser(unsigned long uin, bool create = true);
    friend class ICQClient;
};

// ________________________________________________________________

const unsigned long MSG_PROCESS_ID	= 0x80000000L;

class ICQMessage : public ConfigArray
{
public:
    ICQMessage(unsigned short type);
    virtual ~ICQMessage() {}
    unsigned short Type() { return m_nType; }
    ConfigULong Time;
    ConfigBool Received;
    ConfigULongs Uin;
    ConfigBool Direct;
    unsigned long Id;
    unsigned long	state;
    unsigned short	id1;
    unsigned short	id2;
    unsigned long   timestamp1;
    unsigned long	timestamp2;
    unsigned short	cookie1;
    unsigned short	cookie2;
    string			DeclineReason;
    bool			isExt;
    bool			bDelete;
    unsigned long	getUin();
protected:
    unsigned short m_nType;
};

class ICQMsg : public ICQMessage
{
public:
    ICQMsg();
    ConfigString Message;
    ConfigULong  ForeColor;
    ConfigULong  BackColor;
};

class ICQUrl : public ICQMessage
{
public:
    ICQUrl();
    ConfigString	URL;
    ConfigString	Message;
};

class ICQAuthRequest : public ICQMessage
{
public:
    ICQAuthRequest();
    ConfigString	Message;
};

class ICQAuthGranted : public ICQMessage
{
public:
    ICQAuthGranted();
};

class ICQAuthRefused : public ICQMessage
{
public:
    ICQAuthRefused();
    ConfigString	Message;
};

class ICQAddedToList : public ICQMessage
{
public:
    ICQAddedToList();
};

class ICQSMS : public ICQMessage
{
public:
    ICQSMS();
    ConfigString Phone;
    ConfigString Message;
    ConfigString Network;
};

class ICQSMSReceipt : public ICQMessage
{
public:
    ICQSMSReceipt();
    ConfigString MessageId;
    ConfigString Destination;
    ConfigString Delivered;
    ConfigString Message;
};

class ICQMsgExt : public ICQMessage
{
public:
    ICQMsgExt();
    ConfigString  MessageType;
};

const unsigned long ICQ_ACCEPTED = 0;
const unsigned long ICQ_DECLINED = (unsigned long)-1;

const int FT_DEFAULT	= 0;
const int FT_REPLACE	= 1;
const int FT_RESUME		= 2;

class ICQFile : public ICQMessage
{
public:
    ICQFile();
    ~ICQFile();

    ConfigString	Name;
    ConfigString	Description;
    ConfigULong		Size;

    FileTransferListener *listener;
    FileTransfer		 *ft;

    string  localName;
    string  shortName();

    void resume(int mode);

    bool wait;
    bool autoAccept;
    unsigned long p;
};

class ICQChat : public ICQMessage
{
public:
    ICQChat();
    ~ICQChat();

    ConfigString	Reason;
    ConfigString	Clients;

    ChatListener	*listener;
    ChatSocket		*chat;
};

class ICQWebPanel : public ICQMessage
{
public:
    ICQWebPanel();
    ConfigString	Name;
    ConfigString	Email;
    ConfigString	Message;
};

class ICQEmailPager : public ICQMessage
{
public:
    ICQEmailPager();
    ConfigString	Name;
    ConfigString	Email;
    ConfigString	Message;
};

class Contact : public ConfigArray
{
public:
    Contact();
    ConfigULong		Uin;
    ConfigString	Alias;
};

class ContactList : public ConfigList
{
public:
    ContactList(ConfigArray *arr, const char *name) : ConfigList(arr, name) {}
    ConfigArray *create() { return new Contact; }
};

class ICQContacts : public ICQMessage
{
public:
    ICQContacts();
    ContactList	Contacts;
};

class ICQContactRequest : public ICQMessage
{
public:
    ICQContactRequest();
    ConfigString	Message;
};

// _______________________________________________________________________________________


typedef struct msg_id
{
    unsigned long h;
    unsigned long l;
} msg_id;

typedef struct list_req
{
    unsigned long uin;
    int list_type;
    bool bSet;
} list_req;

bool operator == (const list_req &r1, const list_req &r2);

typedef unsigned char capability[0x10];

class ICQClient : public ClientSocket, public ICQUser
{
public:
    ICQClient();
    ~ICQClient();

    void setStatus(unsigned short status);
    void setInvisible(bool bInvisible);

    ConfigString ServerHost;
    ConfigUShort ServerPort;

    ConfigUShort MinTCPPort;
    ConfigUShort MaxTCPPort;

    ConfigString DecryptedPassword;
    ConfigString EncryptedPassword;
    ConfigBool   WebAware;
    ConfigBool   Authorize;
    ConfigBool   HideIp;
    ConfigBool   Invisible;

    ConfigShort		ProxyType;
    ConfigString	ProxyHost;
    ConfigShort		ProxyPort;
    ConfigBool		ProxyAuth;
    ConfigString	ProxyUser;
    ConfigString	ProxyPasswd;

    void fromServer(string &s);
    void toServer(string &s);
    bool translate(const char *to, const char *from, string &s);
    const char *localCharset();

    ICQContactList contacts;
    ICQGroup *getGroup(unsigned short id, bool create = false);
    ICQUser  *getUser(unsigned long uin, bool create = false);

    bool cancelMessage(ICQMessage*, bool bSendCancel=true);

    ICQEvent *sendMessage(ICQMessage *msg);
    ICQEvent *searchByName(const char *first, const char *last, const char *nick);
    ICQEvent *searchByUin(unsigned long uin);
    ICQEvent *searchWP(const char *first, const char *last, const char *nick,
                       const char *email, char age, char nGender,
                       char nLanguage, const char *city, const char *szState,
                       unsigned short nCountryCode,
                       const char *cCoName, const char *szCoDept, const char *szCoPos,
                       bool bOnlineOnly);

    void setInVisible(ICQUser*, bool);
    void setInInvisible(ICQUser*, bool);
    void setInIgnore(ICQUser*, bool);

    void moveUser(ICQUser*, ICQGroup*);
    void deleteUser(ICQUser*);
    void renameUser(ICQUser*, const char *name);

    void deleteGroup(ICQGroup*);
    void renameGroup(ICQGroup*, const char *name);
    void createGroup(const char *name);

    void setPassword(const char *passwd);
    void setSecurityInfo(bool bAuthorize, bool bWebAware);

    void setInfo(ICQUser *u);

    void declineMessage(ICQMessage *f, const char *reason);
    void acceptMessage(ICQMessage *f);

    virtual void process_event(ICQEvent*);

    virtual void save(ostream &s);
    virtual bool load(istream &s, string &nextPart);

    void addInfoRequest(unsigned long uin, bool bPriority=false);
    void addPhoneRequest(unsigned long uin, bool bPriority=false);
    void addResponseRequest(unsigned long uin, bool bPriority=false);

    bool updatePhoneBook();
    bool updatePhoneStatus();

    string parseRTF(const char *packet);
    string createRTF(const char *html, unsigned long foreColor);
    string clearHTML(const char *html);

    enum LoginState{
        Logoff,
        Connect,
        Login,
        Logged,
        Register,
        Reconnect,
        ForceReconnect
    };
    LoginState m_state;
    list<ICQEvent*> processQueue;
    static void quoteText(const char *text, string &msg);

    void setupProxy();
    void storePassword(const char*);

protected:
    ICQListener *listener;

    virtual bool createFile(ICQFile *f, int mode);
    virtual bool openFile(ICQFile *f);
    virtual bool seekFile(ICQFile *f, unsigned long pos);
    virtual bool readFile(ICQFile *f, Buffer &b, unsigned short size);
    virtual bool writeFile(ICQFile *f, Buffer &b);
    virtual void closeFile(ICQFile *f);

    virtual unsigned long getFileSize(const char *name);

    unsigned short m_nLogonStatus;
    char m_nChannel;
    bool m_bHeader;
    unsigned short m_nSequence;
    unsigned short m_nMsgSequence;
    virtual void packet_ready();
    virtual void error_state();
    virtual void create_socket();
    virtual void connect_ready();
    virtual void idle();

    time_t m_lastTime;
    time_t m_reconnectTime;

    void chn_login();
    void chn_close();

    bool m_bBirthday;

    list<unsigned long> infoRequestQueue;
    list<unsigned long> phoneRequestQueue;
    list<unsigned long> responseRequestQueue;

    void processInfoRequestQueue();
    void processPhoneRequestQueue(unsigned short seq);
    void processResponseRequestQueue(unsigned short seq);

    unsigned short phoneRequestSeq;
    unsigned short responseRequestSeq;

    unsigned long lastInfoRequestTime;
    unsigned long lastPhoneRequestTime;
    unsigned long lastResponseRequestTime;

    list<list_req>  listQueue;
    void processListQueue();

    list<ICQEvent*> msgQueue;
    void processMsgQueue();
    void processMsgQueueThruServer();
    void processMsgQueueSMS();
    void processMsgQueueAuth();

    list<ICQEvent*> varEvents;
    ICQEvent *findVarEvent(unsigned short id);

    list<ICQEvent*> listEvents;
    ICQEvent *findListEvent(unsigned short id);

    void snac_service(unsigned short type, unsigned short seq);
    void snac_location(unsigned short type, unsigned short seq);
    void snac_buddy(unsigned short type, unsigned short seq);
    void snac_message(unsigned short type, unsigned short seq);
    void snac_bos(unsigned short type, unsigned short seq);
    void snac_ping(unsigned short type, unsigned short seq);
    void snac_lists(unsigned short type, unsigned short seq);
    void snac_various(unsigned short type, unsigned short seq);
    void snac_login(unsigned short type, unsigned short seq);

    void locationRequest();
    void buddyRequest();
    void messageRequest();
    void listsRequest();
    void bosRequest();
    void sendCapability();
    void sendICMB();
    void sendClientReady();

    void sendUpdate(Buffer &b, unsigned long c1, unsigned long c2, unsigned long c3);
    bool needPhonebookUpdate;
    bool needPhoneStatusUpdate;

    void requestKey(const char*);

    void sendContactList();
    void sendVisibleList();
    void sendInvisibleList();
    void sendLogonStatus();
    void sendStatus(unsigned long status);
    void sendMessageRequest();
    void serverRequest(unsigned short cmd, unsigned short seq=0);
    void sendServerRequest();
    unsigned long fullStatus(unsigned long status);

    void getAutoResponse(unsigned long uin, string &s);

    void requestAutoResponse(unsigned long uin);
    void requestInfo(unsigned long uin);
    void requestPhoneBook(unsigned long uin);

    void sendThroughServer(unsigned long uin, unsigned short type, Buffer &b, msg_id *id=NULL, bool addTlv=true);
    void cancelSendFile(ICQFile*);
    void sendPhoneInit();

    void messageReceived(ICQMessage *msg);

    void parseAdvancedMessage(unsigned long uin, Buffer &msg, bool needAck, unsigned long t1, unsigned long t2);
    ICQMessage *parseMessage(unsigned short type, unsigned long uin, string &p, Buffer &b,
                             unsigned short cookie1, unsigned short cookie2, unsigned long timestamp1, unsigned long timestamp2);
    void parseMessageText(const char *packet, string &msg);
    bool parseFE(const char *str, vector<string> &l, unsigned n);

    unsigned m_nPacketStart;
    void flap(char channel);
    void snac(unsigned short fam, unsigned short type, bool msgId=false);
    void sendPacket();
    void dropPacket();

    void addToContacts(unsigned long uin);
    void addToVisibleList(unsigned long uin);
    void addToInvisibleList(unsigned long uin);
    void removeFromVisibleList(unsigned long uin);
    void removeFromInvisibleList(unsigned long uin);

    void sendRoster(ICQEvent *e, unsigned short cmd, const char *name, unsigned short grp_id,
                    unsigned short usr_id, unsigned short subCmd, const char *alias=NULL, bool waitAuth=false);
    void sendRoster(ICQEvent *e, unsigned short cmd, unsigned long uin, unsigned short grp_id,
                    unsigned short usr_id, unsigned short subCmd, const char *alias=NULL, bool waitAuth=false);
    void sendRosterGrp(const char *name, unsigned short grpId, unsigned short usrId);

    bool setAboutInfo(ICQUser *u);
    bool setMoreInfo(ICQUser *u);
    bool setWorkInfo(ICQUser *u);
    bool setMainInfo(ICQUser *u);
    bool setInterestsInfo(ICQUser *u);
    bool setBackgroundInfo(ICQUser *u);
    bool setMailInfo(ICQUser *u);

    void sendInfoUpdate();
    void sendPhoneStatus();

    const char *serverCharset();

    void fromUTF(string &s);
    void toUTF(string &s);

    void close();
    unsigned short advCounter;

    void packInfoList(const ExtInfoList &info);

    void packMessage(Buffer &b, ICQMessage *m, const char *msg, unsigned short msgFlags,
                     unsigned short msgState, char oper, bool bShort=false);

    unsigned long m_nProcessId;

    static const capability capabilities[4];

    static const unsigned char PHONEBOOK_SIGN[16];
    static const unsigned char PLUGINS_SIGN[16];
    static const unsigned char PICTURE_SIGN[16];

    friend class ICQEvent;
    friend class ICQSetListEvent;
    friend class MoveUserEvent;
    friend class ICQListenSocket;
    friend class DirectSocket;
    friend class DirectClient;
    friend class FileTransfer;
    friend class ChatSocket;
};

#endif

