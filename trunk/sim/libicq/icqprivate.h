/***************************************************************************
                          icqprivate.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _ICQPRIVATE_H
#define _ICQPRIVATE_H

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

typedef struct msg_id
{
    unsigned long h;
    unsigned long l;
} msg_id;

typedef struct rtf_charset
{
    const char *name;
    int			rtf_code;
} charset;

typedef struct info_request
{
    unsigned long uin;
    bool		  bAuto;
    bool operator == (const info_request &r) { return (r.uin == uin); }
} info_request;

typedef unsigned char capability[0x10];

const unsigned CAP_ISISQ	= 0;
const unsigned CAP_RTF		= 1;
const unsigned CAP_STR_2002	= 2;
const unsigned CAP_IS_2002	= 3;
const unsigned CAP_SIM		= 4;
const unsigned CAP_STR_2001	= 5;
const unsigned CAP_IS_2001	= 6;
const unsigned CAP_IS_WEB	= 7;
const unsigned CAP_TRILLIAN	= 8;
const unsigned CAP_TRIL_CRYPT	= 9;
const unsigned CAP_LICQ		= 10;
const unsigned CAP_MACICQ	= 11;
const unsigned CAP_AIM_CHAT	= 12;
const unsigned CAP_MICQ		= 13;

class ICQListener;

class ICQClientPrivate : public ClientSocketNotify
{
public:
    ICQClientPrivate(ICQClient *client, SocketFactory *factory);
    ~ICQClientPrivate();

    ICQClient *client;
    SocketFactory *factory;
    Buffer cookie;

    virtual void idle();

    void close();
    void create_socket();
    void setStatus(unsigned short);

    ClientSocket	*sock;
    ICQListener		*listener;

    unsigned short m_nLogonStatus;
    char m_nChannel;
    bool m_bHeader;
    unsigned short m_nSequence;
    unsigned short m_nMsgSequence;

    virtual void packet_ready();
    virtual bool error_state(SocketError);
    virtual void connect_ready();

    time_t m_lastTime;
    time_t m_reconnectTime;

    void chn_login();
    void chn_close();

    bool m_bBirthday;
    bool m_bRosters;

    list<info_request> infoRequestQueue;
    list<info_request> phoneRequestQueue;
    list<info_request> responseRequestQueue;
    bool addRequest(unsigned long uin, bool bPriority, list<info_request> &queue);

    void processInfoRequestQueue();
    void processPhoneRequestQueue(unsigned short seq);
    void processResponseRequestQueue(unsigned short seq);

    unsigned short phoneRequestSeq;
    unsigned short responseRequestSeq;

    unsigned long lastInfoRequestTime;
    unsigned long lastPhoneRequestTime;
    unsigned long lastResponseRequestTime;

    list<ICQEvent*> msgQueue;
    void processMsgQueue();
    void processMsgQueueThruServer();
    void processMsgQueueSMS();
    void processMsgQueueAuth();

    list<ICQEvent*> varEvents;
    ICQEvent *findVarEvent(unsigned short id);

    list<ICQEvent*> listEvents;
    ICQEvent *findListEvent(unsigned short id);

    list<ICQEvent*> processQueue;

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

    void fillDirectInfo(Buffer &b);
    void sendUpdate(Buffer &b);
    bool needPhonebookUpdate;
    bool needPhoneStatusUpdate;
    bool needShareUpdate;

    void requestKey(const char*);
    void moveUser(ICQUser *u, ICQGroup *g);
    void renameUser(ICQUser *u, const char *alias);
    void deleteUser(ICQUser *u);
    void createGroup(const char *name);
    void deleteGroup(ICQGroup *g);
    void renameGroup(ICQGroup *g, const char *name);

    void sendContactList();
    void sendLogonStatus();
    void sendStatus(unsigned long status);
    void sendMessageRequest();
    void serverRequest(unsigned short cmd, unsigned short seq=0);
    void sendServerRequest();
    unsigned long fullStatus(unsigned long status);

    void setChatGroup();
    void searchChat(unsigned short group);

    bool requestAutoResponse(unsigned long uin, bool bAuto);
    bool requestInfo(unsigned long uin, bool bAuto);
    bool requestPhoneBook(unsigned long uin, bool bAuto);

    void sendThroughServer(unsigned long uin, unsigned short type, Buffer &b, msg_id *id=NULL, bool addTlv=true);
    void cancelSendFile(ICQFile*);

    void declineMessage(ICQMessage *f, const char *reason);
    void acceptMessage(ICQMessage *f);

    void messageReceived(ICQMessage *msg);

    void parseAdvancedMessage(unsigned long uin, Buffer &msg, bool needAck, unsigned long t1, unsigned long t2);
    ICQMessage *parseMessage(unsigned short type, unsigned long uin, string &p, Buffer &b,
                             unsigned short cookie1, unsigned short cookie2, unsigned long timestamp1, unsigned long timestamp2);
    bool parseMessageText(const string &packet, string &msg, ICQUser *u);
    bool parseFE(const char *str, vector<string> &l, unsigned n);

    unsigned m_nPacketStart;
    void flap(char channel);
    void snac(unsigned short fam, unsigned short type, bool msgId=false);
    void sendPacket();
    void dropPacket();

    void addToContacts(unsigned long uin);
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

    bool match(const char *str, const char *pat);

    void sendInfoUpdate();
    void sendPhoneStatus();
    void sendShareUpdate();
    void sendAutoReply(unsigned long uin, unsigned long timestamp1, unsigned long timestamp2,
                       char info[18], unsigned short cookie1, unsigned short cookie2,
                       unsigned char msgType, unsigned char msgFlags, unsigned long msgState,
                       string response, unsigned short response_type, Buffer &copy);

    string makeMessageText(ICQMsg *msg, ICQUser *u);

    string parseRTF(const char *packet, ICQUser *u);
    string createRTF(const string &html, unsigned long foreColor, const char *encoding);

    unsigned short advCounter;

    void packInfoList(const ExtInfoList &info);

    void packMessage(Buffer &b, ICQMessage *m, const char *msg, unsigned short msgFlags,
                     unsigned short msgState, char oper, bool bShort, bool bConvert);

    unsigned long m_nProcessId;

    void checkBirthDay();

    void sendVisibleList();
    void sendInvisibleList();
    void setInList(ICQUser *u, bool bSet, unsigned short list_type);

    bool utf16to8(unsigned short c, string &s);

    enum LoginState{
        Logoff,
        Connect,
        Login,
        Logged,
        Register,
        Reconnect,
        ForceReconnect,
        ErrorState
    };
    LoginState m_state;

    unsigned check_time;

    static const capability *capabilities;

    static const unsigned char PHONEBOOK_SIGN[16];
    static const unsigned char SHARED_FILES_SIGN[16];
    static const unsigned char PLUGINS_SIGN[16];
    static const unsigned char PICTURE_SIGN[16];

    friend class ICQUser;
    friend class ICQEvent;
    friend class ICQSetListEvent;
    friend class MoveUserEvent;
    friend class ICQListenSocket;
    friend class DirectSocket;
    friend class DirectClient;
    friend class FileTransfer;
    friend class ChatSocket;
};

// ___________________________________________________________________________________

class ICQFile;
class ICQChat;

class DirectSocket : public ClientSocketNotify
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
    DirectSocket(Socket *s, ICQClientPrivate*);
    DirectSocket(unsigned long real_ip, unsigned long ip, unsigned short port, ICQUser*, ICQClientPrivate*);
    ~DirectSocket();
    virtual void packet_ready();
    SocketState state;
    void connect();
    virtual bool error_state(SocketError);
    virtual void connect_ready();
protected:
    virtual void processPacket() = 0;
    ClientSocket *sock;
    void init();
    void sendInit();
    void sendInitAck();
    bool m_bHeader;
    ICQClientPrivate *client;
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

class ICQListener : public ServerSocketNotify
{
public:
    ICQListener(ICQClientPrivate *client);
    ~ICQListener();
    bool created() { return (sock != NULL); }
    unsigned short port();
protected:
    virtual void accept(Socket *s);
    ServerSocket *sock;
    ICQClientPrivate  *client;
};

class SSLClient;

class DirectClient : public DirectSocket
{
public:
    DirectClient(Socket *s, ICQClientPrivate *client);
    DirectClient(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClientPrivate *client);
    ~DirectClient();
    unsigned short sendMessage(ICQMessage*);
    void sendAutoResponse(ICQMessage*, string string);
    void acceptMessage(ICQMessage*);
    void declineMessage(ICQMessage*, const char *reason);
    bool isLogged() { return (state != None) && (state != WaitInit2); }
    bool isSecure();
protected:
    enum State{
        None,
        WaitLogin,
        WaitInit2,
        Logged,
        SSLconnect
    };
    State state;
    void processPacket();
    void connect_ready();
    bool error_state(SocketError);
    ICQUser *u;
    void sendInit2();
    void startPacket(unsigned short cms, unsigned short seq);
    void startMsgPacket(unsigned short msgType, const string &s);
    void sendPacket();
    void sendAck(unsigned short, unsigned short msgType);
    void secureConnect();
    void secureListen();
    void secureStop(bool bShutdown);
#ifdef USE_OPENSSL
    SSLClient *ssl;
#endif
    friend class ICQUser;
};

class FileTransferListener : public ServerSocketNotify
{
public:
    FileTransferListener(ICQFile *file, ICQClientPrivate *client);
    unsigned short port();
protected:
    virtual void accept(Socket *s);
    ServerSocket *sock;
    ICQClientPrivate *client;
    ICQFile *file;
};

class FileTransfer : public DirectSocket
{
public:
    FileTransfer(Socket *s, ICQClientPrivate *client, ICQFile *file);
    FileTransfer(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClientPrivate *client, ICQFile *file);
    void resume(int mode);
    int  speed() { return m_nSpeed; }
    void setSpeed(int nSpeed);
    void setPos(unsigned long n) { m_fileSize = n; }
    unsigned long sendSize()	{ return m_fileSize; }
    unsigned long totalSize()	{ return m_totalSize; }
    unsigned long curFile()		{ return m_curFile; }
    unsigned long nFiles()		{ return m_nFiles; }
    unsigned curSize()	{ return m_curSize; }
    string curName;
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

    ICQFile *file;
    void processPacket();
    void connect_ready();
    bool error_state(SocketError);

    void write_ready();
    void init();
    void startPacket(char cmd);
    void sendPacket(bool dump=true);
    void sendFileInfo();

    State state;
    unsigned long m_packetOffs;
    unsigned long m_nSpeed;
    unsigned long m_nFiles;
    unsigned long m_curFile;
    unsigned long m_curSize;
    unsigned long m_fileSize;
    unsigned long m_totalSize;

    unsigned long m_sendTime;
    unsigned long m_sendSize;
};

class ChatListener : public ServerSocketNotify
{
public:
    ChatListener(ICQChat *chat, ICQClientPrivate *client);
    unsigned short port();
protected:
    virtual void accept(Socket *s);
    ServerSocket *sock;
    ICQClientPrivate *client;
    ICQChat *chat;
};

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
    ChatSocket(Socket *s, ICQClientPrivate *client, ICQChat *file);
    ChatSocket(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClientPrivate *client, ICQChat *chat);

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
        WaitLogin,
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

    bool error_state(SocketError);
    void startPacket();
    void sendPacket();
    void packet_ready();
    void connect_ready();

    void putText(string &s);

    virtual void processPacket();
};

#endif

