/***************************************************************************
                          icqclient.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
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

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
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

const unsigned char ICQ_TCP_VERSION = 0x08;

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

const unsigned short ICQ_MSGxMAIL			   = 0x00E0;
const unsigned short ICQ_MSGxSMS               = 0x00E1;
const unsigned short ICQ_MSGxSMSxRECEIPT       = 0x00E2;
const unsigned short ICQ_MSGxCONTACTxREQUEST   = 0x00E3;
const unsigned short ICQ_MSGxSTATUS			   = 0x00E4;

const unsigned short ICQ_MSGxSECURExCLOSE	= 0x00EE;
const unsigned short ICQ_MSGxSECURExOPEN	= 0x00EF;

const unsigned short ICQ_READxAWAYxMSG		= 0x03E8;
const unsigned short ICQ_READxOCCUPIEDxMSG	= 0x03E9;
const unsigned short ICQ_READxNAxMSG		= 0x03EA;
const unsigned short ICQ_READxDNDxMSG		= 0x03EB;
const unsigned short ICQ_READxFFCxMSG		= 0x03EC;

const unsigned long  UIN_SPECIAL    = 0xF0000000L;

const unsigned short USER_TYPE_ICQ	= 0;
const unsigned short USER_TYPE_EXT  = 1;

const unsigned short ICQ_GROUPS					= 0x0001;
const unsigned short ICQ_VISIBLE_LIST			= 0x0002;
const unsigned short ICQ_INVISIBLE_LIST			= 0x0003;
const unsigned short ICQ_INVISIBLE_STATE		= 0x0004;
const unsigned short ICQ_IGNORE_LIST			= 0x000E;

class ICQGroup
{
public:
    ICQGroup();
    virtual ~ICQGroup() {}
    string			Name;
    unsigned short	Id;
protected:
    bool			bChecked;
    friend class	ICQClient;
    friend class	ICQClientPrivate;
};

class EMailInfo
{
public:
    EMailInfo();
    virtual ~EMailInfo() {}
    string			Email;
    bool			Hide;
    bool			MyInfo;
};

class EMailList : public list<EMailInfo*>
{
public:
    EMailList() {}
    ~EMailList();
    EMailList &operator = (const EMailList&);
private:
    EMailList(const EMailList&);
};

class ExtInfo
{
public:
    ExtInfo();
    ~ExtInfo() {}
    unsigned short	Category;
    string			Specific;
};

class ExtInfoList : public list<ExtInfo*>
{
public:
    ExtInfoList() {}
    ~ExtInfoList();
    ExtInfoList &operator = (const ExtInfoList&);
private:
    ExtInfoList(const ExtInfoList&);
};

const unsigned long PHONE = 0;
const unsigned long MOBILE = 1;
const unsigned long SMS = 2;
const unsigned long FAX = 3;
const unsigned long PAGER = 4;

class PhoneInfo
{
public:
    PhoneInfo();
    virtual ~PhoneInfo() {}

    string			Name;
    unsigned short	Type;
    bool			Active;
    string			Country;
    string			AreaCode;
    string			Number;
    string			Extension;
    string			Provider;
    string			Gateway;
    bool			Publish;
    bool			FromInfo;
    bool			MyInfo;
    string getNumber();
    void setNumber(const string &number, unsigned long type);
    bool isEqual(const char *number);
    static bool isEqual(const char *n1, const char *n2);
};

class PhoneBook : public list<PhoneInfo*>
{
public:
    PhoneBook() {}
    ~PhoneBook();
    void add(const char *number, const char *name, unsigned long type, bool ownInfo, bool bPublish);
    void add(const PhoneBook &l);
    PhoneBook &operator = (const PhoneBook&);
private:
    PhoneBook(const PhoneBook&);
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
const int EVENT_PROXY_ERROR			= 18;
const int EVENT_PROXY_BAD_AUTH		= 19;

const int EVENT_SUBTYPE_FULLINFO	= 1;
const int EVENT_SUBTYPE_AUTOREPLY	= 2;

class ICQEvent
{
public:
    enum State
    {
        Unknown,
        Success,
        Fail,
        Send
    };
    ICQEvent(int type, unsigned long uin = 0, unsigned long subbtype = 0, ICQMessage *msg = NULL);
    virtual int type() { return m_nType; }
    int subType() { return m_nSubType; }
    virtual ~ICQEvent() {}
    ICQMessage *message() { return msg; }
    void setMessage(ICQMessage *m) { msg = m; }
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
    virtual bool processAnswer(ICQClientPrivate*, Buffer &b, unsigned short nSubtype);
    virtual void failAnswer(ICQClientPrivate*);
    friend class ICQClient;
    friend class ICQClientPrivate;
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
    bool processAnswer(ICQClientPrivate*, Buffer&, unsigned short nSubtype);
};

// _______________________________________________________________________________

class DirectClient;
class ICQClientPrivate;

class ICQUser
{
public:
    ICQUser();
    virtual ~ICQUser();

    string name(bool useUin = true);

    unsigned    NewMessages;
    unsigned    LastMessageType;

    unsigned short	Type;
    string			Alias;
    unsigned short	Id;
    unsigned short	VisibleId;
    unsigned short	InvisibleId;
    unsigned short	IgnoreId;
    unsigned short	GrpId;
    unsigned long	Uin;
    bool			WaitAuth;

    string			AutoResponseAway;
    string			AutoResponseNA;
    string			AutoResponseDND;
    string			AutoResponseOccupied;
    string			AutoResponseFFC;

    string AutoReply;

    unsigned long uStatus;
    unsigned long prevStatus;
    bool CanPlugin;

    list<unsigned long> unreadMsgs;

    unsigned long	LastActive;
    unsigned long	OnlineTime;
    unsigned long	StatusTime;

    unsigned long	IP;
    unsigned short	Port;
    unsigned long	RealIP;

    string			HostName;
    string			RealHostName;

    unsigned short		Version;
    unsigned short		Mode;

    // General info
    string			Nick;
    string			FirstName;
    string			LastName;
    string			City;
    string			State;
    string			Address;
    string			Zip;
    unsigned short		Country;
    char			TimeZone;
    string			HomePhone;
    string			HomeFax;
    string			PrivateCellular;
    string			EMail;
    bool			HiddenEMail;

    // Email info
    EMailList			EMails;

    // More info
    char			Age;
    char			Gender;
    string			Homepage;
    unsigned short		BirthYear;
    char			BirthMonth;
    char			BirthDay;
    char			Language1;
    char			Language2;
    char			Language3;

    // Company info
    string			WorkCity;
    string			WorkState;
    string			WorkZip;
    string			WorkAddress;
    string			WorkName;
    string			WorkDepartment;
    string			WorkPosition;
    unsigned short		WorkCountry;
    unsigned short		Occupation;
    string			WorkHomepage;
    string			WorkPhone;
    string			WorkFax;

    // About info
    string			About;

    // Background info
    ExtInfoList			Backgrounds;
    ExtInfoList			Affilations;

    // Personal interests info
    ExtInfoList			Interests;

    // PhoneBook
    PhoneBook			Phones;
    char				PhoneState;
    unsigned long   	InfoUpdateTime;
    unsigned long		PhoneStatusTime;
    unsigned long		PhoneBookTime;
    bool				bMyInfo;

    unsigned long		Caps;
    unsigned long		Build;
    bool				hasCap(unsigned nCap);

    string				Encoding;

    string	client();
    bool canRTF();
    bool isSecure();

    void adjustPhones();
    void adjustEMails(EMailList *mails = NULL, bool bOwn=false);

    void setOffline();
    unsigned long DCcookie;

    DirectClient *direct;

    bool bIsTemp;
    bool isOnline();
    bool bPhoneChanged;

    ICQEvent *addMessage(ICQMessage*, ICQClientPrivate*);
    list<ICQEvent*> msgQueue;
    void processMsgQueue(ICQClientPrivate*);

    void requestSecureChannel(ICQClientPrivate*);
    void closeSecureChannel(ICQClientPrivate*);
};

class ICQClient;

class ICQContactList
{
public:
    ICQContactList(ICQClient*);
    ~ICQContactList();
    unsigned short	Len;
    unsigned short	Invisible;
    unsigned long	Time;
    bool			Expand;
    list<ICQUser*>  users;
    vector<ICQGroup*> groups;
    unsigned short getUserId(ICQUser *u, unsigned short grp, bool bNoSet);
    unsigned short getGroupId(ICQGroup *g);
    unsigned long findByEmail(const string &name, const string &email);
    unsigned long findByPhone(const string &number);
    void init();
protected:
    ICQClient *client;
    ICQGroup *getGroup(unsigned short id, bool create = true);
    ICQUser  *getUser(unsigned long uin, bool create = true);
    friend class ICQClient;
};

// ________________________________________________________________

const unsigned long MSG_PROCESS_ID	= 0x80000000L;
const unsigned long MSG_NEW			= 0xFFFFFFFFL;

class ICQMessage
{
public:
    ICQMessage(unsigned short type);
    virtual ~ICQMessage() {}
    unsigned short Type() { return m_nType; }
    void setType(unsigned short type) { m_nType = type; }
    unsigned long	Time;
    bool			Received;
    list<unsigned long>	Uin;
    bool			Direct;
    string			Charset;
    unsigned long	Id;
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
    string			Message;
    unsigned long	ForeColor;
    unsigned long	BackColor;
};

class ICQUrl : public ICQMessage
{
public:
    ICQUrl();
    string			URL;
    string			Message;
};

class ICQAuthRequest : public ICQMessage
{
public:
    ICQAuthRequest();
    string			Message;
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
    string			Message;
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
    string			Phone;
    string			Message;
    string			Network;
};

class ICQSMSReceipt : public ICQMessage
{
public:
    ICQSMSReceipt();
    string			MessageId;
    string			Destination;
    string			Delivered;
    string			Message;
};

class ICQMsgExt : public ICQMessage
{
public:
    ICQMsgExt();
    string			MessageType;
};

class ICQSecureOn : public ICQMessage
{
public:
    ICQSecureOn();
};

class ICQSecureOff : public ICQMessage
{
public:
    ICQSecureOff();
};

class ICQAutoResponse : public ICQMessage
{
public:
    ICQAutoResponse();
};

class ICQStatus : public ICQMessage
{
public:
    ICQStatus();
    unsigned long status;
};

const unsigned long ICQ_ACCEPTED = 0;
const unsigned long ICQ_DECLINED = (unsigned long)-1;

const int FT_DEFAULT	= 0;
const int FT_REPLACE	= 1;
const int FT_RESUME		= 2;
const int FT_SKIP		= 3;

typedef struct fileName
{
    string localName;
    string name;
    unsigned long size;
} fileName;

class FileTransferListener;
class FileTransfer;

class ICQFile : public ICQMessage
{
public:
    ICQFile();
    ~ICQFile();

    string			Name;
    string			Description;
    unsigned long	Size;

    FileTransferListener *listener;
    FileTransfer		 *ft;

    string  localName;
    string  shortName();

    ICQClient *client;

    void resume(int mode);
    vector<fileName> files;

    bool wait;
    bool autoAccept;
    unsigned long p;

    unsigned nFiles();
    unsigned curFile();
    unsigned short speed();

    unsigned long totalSize();
    unsigned long sendSize();
    unsigned long curSize();
    string curName();

    void setCurName(const char*);
    void setPos(unsigned long);
    void setSpeed(unsigned short);
};

class ChatListener;
class ChatSocket;

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

class ICQChat : public ICQMessage
{
public:
    ICQChat();
    ~ICQChat();

    string			Reason;
    string			Clients;

    ChatListener	*listener;
    ChatSocket		*chat;

    void sendLine(const char *line);
    bool isBold();
    bool isItalic();
    bool isUnderline();
    unsigned long bgColor();
    unsigned long fgColor();
};

class ICQWebPanel : public ICQMessage
{
public:
    ICQWebPanel();
    string			Name;
    string			Email;
    string			Message;
};

class ICQEmailPager : public ICQMessage
{
public:
    ICQEmailPager();
    string			Name;
    string			Email;
    string			Message;
};

class Contact
{
public:
    Contact();
    unsigned long	Uin;
    string			Alias;
};

class ContactList : public list<Contact*>
{
public:
    ContactList() {}
    ~ContactList();
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
    string			Message;
};

// _______________________________________________________________________________________

class ICQClientPrivate;

class ICQClient
{
public:
    ICQClient(SocketFactory *factory);
    virtual ~ICQClient();

    ICQUser *owner;

    void setStatus(unsigned short status);
    void setInvisible(bool bInvisible);

    string			ServerHost;
    unsigned short	ServerPort;

    string			DecryptedPassword;
    string			EncryptedPassword;
    bool			WebAware;
    bool			Authorize;
    bool			HideIp;
    bool			RejectMessage;
    bool			RejectURL;
    bool			RejectWeb;
    bool			RejectEmail;
    bool			RejectOther;
    string			RejectFilter;

    unsigned short	DirectMode;
    bool			BypassAuth;

    string			ShareDir;
    bool		    ShareOn;

    virtual ICQUser		*createUser();
    virtual ICQGroup	*createGroup();

    void fromServer(string &s, ICQUser*);
    void toServer(string &s, ICQUser*);
    const char *localCharset(ICQUser*);

    static void fromServer(string &s, const char *name);
    static void toServer(string &s, const char *name);
    static bool translate(const char *to, const char *from, string &s);
    static const char *localCharset();

    ICQContactList contacts;
    ICQGroup *getGroup(unsigned short id, bool create = false);
    ICQUser  *getUser(unsigned long uin, bool create = false, bool bIsTemp = false);

    bool cancelMessage(ICQMessage*, bool bSendCancel=true);

    ICQEvent *sendMessage(ICQMessage *msg);
    ICQEvent *searchByName(const char *first, const char *last, const char *nick, bool bOnlineOnly);
    ICQEvent *searchByUin(unsigned long uin);
    ICQEvent *searchWP(const char *first, const char *last, const char *nick,
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

    void addInfoRequest(unsigned long uin, bool bPriority=false);
    void addPhoneRequest(unsigned long uin, bool bPriority=false);
    void addResponseRequest(unsigned long uin, bool bPriority=false);

    bool updatePhoneBook();
    bool updatePhoneStatus();

    string clearHTML(const string &html);

    bool isLogged();
    bool isConnecting();

    static string quoteText(const char *text);
    static string unquoteText(const char *text);

    void storePassword(const char*);

    void setRejectFilter(const char*);

    static bool fromUTF(string &s, const char *encoding);
    static bool toUTF(string &s, const char *encoding);
    static const char *serverCharset(const char *l=NULL);

    void setShare(bool bState);
    void close();

    void requestSecureChannel(ICQUser*);
    void closeSecureChannel(ICQUser*);
    ICQMessage *getProcessMessage(unsigned long offs);

    virtual bool createFile(ICQFile *f, int mode);
    virtual bool openFile(ICQFile *f);
    virtual bool seekFile(ICQFile *f, unsigned long pos);
    virtual bool readFile(ICQFile *f, Buffer &b, unsigned short size);
    virtual bool writeFile(ICQFile *f, Buffer &b);
    virtual void closeFile(ICQFile *f);

    virtual unsigned long getFileSize(const char *name, int *nSrcFile, vector<fileName> &files);

    void idle();
    SocketFactory *factory();

    static string cryptPassword(const char *p);

protected:
    void init();
    ICQClientPrivate *p;
};

#endif

