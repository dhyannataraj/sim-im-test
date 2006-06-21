/***************************************************************************
                          msnclient.h  -  description
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

#ifndef _MSNCLIENT_H
#define _MSNCLIENT_H

#include "simapi.h"
#include "stl.h"
#include "socket.h"
#include "fetch.h"

const unsigned MSN_SIGN			= 0x0003;

const unsigned STATUS_BRB		= 101;
const unsigned STATUS_PHONE		= 102;
const unsigned STATUS_LUNCH		= 103;

const unsigned MSN_FORWARD	= 0x0001;
const unsigned MSN_ACCEPT	= 0x0002;
const unsigned MSN_BLOCKED	= 0x0004;
const unsigned MSN_REVERSE  = 0x0008;

const unsigned MSN_FLAGS	= 0x000F;
const unsigned MSN_CHECKED	= 0x1000;

class SBSocket;

struct MSNUserData : public SIM::clientData
{
    SIM::Data	EMail;
    SIM::Data	ScreenName;
    SIM::Data	Status;
    SIM::Data	StatusTime;
    SIM::Data	OnlineTime;
    SIM::Data	PhoneHome;
    SIM::Data	PhoneWork;
    SIM::Data	PhoneMobile;
    SIM::Data	Mobile;
    SIM::Data	Group;
    SIM::Data	Flags;
    SIM::Data	sFlags;
    SIM::Data	typing_time;
    SIM::Data	IP;
    SIM::Data	RealIP;
    SIM::Data	Port;
    SIM::Data	sb;
};

typedef struct MSNClientData
{
    SIM::Data	Server;
    SIM::Data	Port;
    SIM::Data	ListVer;
    SIM::Data	ListRequests;
    SIM::Data	Version;
    SIM::Data	MinPort;
    SIM::Data	MaxPort;
    SIM::Data	UseHTTP;
    SIM::Data	AutoHTTP;
    SIM::Data	Deleted;
    SIM::Data	NDeleted;
    SIM::Data	AutoAuth;
    MSNUserData	owner;
} MSNClientData;

class MSNClient;
class XfrPacket;

typedef struct msgInvite
{
    SIM::Message		*msg;
    unsigned	cookie;
} msgInvite;

class SBSocket : public QObject, public SIM::ClientSocketNotify
{
    Q_OBJECT
public:
    SBSocket(MSNClient *client, SIM::Contact *contact, MSNUserData *data);
    ~SBSocket();
    void connect(const char *addr, const char *session, const char *cookie, bool bDirection);
    void connect();
    bool send(SIM::Message *msg);
    void timer(unsigned now);
    void setTyping(bool bTyping);
    bool cancelMessage(SIM::Message *msg);
    bool acceptMessage(SIM::Message *msg, const char *dir, SIM::OverwriteMode mode);
    bool declineMessage(SIM::Message *msg, const QString &reason);
    void acceptMessage(unsigned short port, unsigned cookie, unsigned auth_cookie);
    void declineMessage(unsigned cookie);
protected:
    enum State
    {
        Unknown,
        ConnectingSend,
        ConnectingReceive,
        WaitJoin,
        Connected
    };
    virtual bool error_state(const QString &err, unsigned code);
    virtual void connect_ready();
    virtual void packet_ready();
    void send(const char *cmd, const char *args);
    void getLine(const char *line);
    void getMessage(unsigned size);
    bool getMessage();
    void messageReady();
    void process(bool bTyping=true);
    void sendMessage(const char*, const char *type);
    void sendTyping();
    void sendFile();
    std::list<msgInvite>	m_acceptMsg;
    std::list<msgInvite>	m_waitMsg;

    State			m_state;
    SIM::Contact	*m_contact;
    MSNClient		*m_client;
    MSNUserData		*m_data;
    std::string		m_session;
    std::string		m_cookie;
    SIM::ClientSocket *m_socket;
    unsigned		m_packet_id;
    std::string		m_message;
    unsigned		m_messageSize;
    bool			m_bTyping;
    XfrPacket		*m_packet;
    std::list<SIM::Message*>  m_queue;
    QString			m_msgText;
    QString			m_msgPart;
    unsigned		m_msg_id;
    unsigned		m_invite_cookie;
};

const unsigned LR_CONTACTxCHANGED		= 0;
const unsigned LR_CONTACTxREMOVED		= 1;
const unsigned LR_CONTACTxREMOVED_BL	= 2;
const unsigned LR_GROUPxCHANGED			= 3;
const unsigned LR_GROUPxREMOVED			= 4;

const unsigned NO_GROUP					= (unsigned)(-1);

typedef struct MSNListRequest
{
    unsigned	Type;
    QString	    Name;
    unsigned	Group;
} MSNListRequest;

class MSNPacket;
class MSNServerMessage;

class MSNClient : public SIM::TCPClient, public FetchClient
{
    Q_OBJECT
public:
    MSNClient(SIM::Protocol*, ConfigBuffer *cfg);
    ~MSNClient();
    virtual QString name();
    virtual QString dataName(void*);
    virtual QWidget	*setupWnd();
    virtual QString getConfig();
    PROP_STR(Server);
    PROP_USHORT(Port);
    PROP_ULONG(ListVer);
    PROP_UTF8(ListRequests);
    PROP_STR(Version);
    PROP_USHORT(MinPort);
    PROP_USHORT(MaxPort);
    PROP_BOOL(UseHTTP);
    PROP_BOOL(AutoHTTP);
    PROP_STRLIST(Deleted);
    PROP_ULONG(NDeleted);
    PROP_BOOL(AutoAuth);
    QString getLogin();
    QString unquote(const QString&);
    QString quote(const QString&);
    void sendLine(const char *line, bool crlf = true);
    void setLogin(const QString&);
    MSNClientData	data;
    MSNUserData *findContact(const char *mail, const char *name, SIM::Contact *&contact, bool nJoin=true);
    MSNUserData *findContact(const char *mail, SIM::Contact *&contact);
    MSNUserData *findGroup(unsigned long id, const char *name, SIM::Group *&grp);
    void auth_message(SIM::Contact *contact, unsigned type, MSNUserData *data);
    std::list<MSNListRequest> m_requests;
    void processRequests();
    MSNListRequest *findRequest(unsigned long id, unsigned type, bool bDelete=false);
    MSNListRequest *findRequest(const QString &name, unsigned type, bool bDelete=false);
    bool add(const char *mail, const char *name, unsigned grp);
    std::list<SBSocket*> m_SBsockets;
    virtual void setupContact(SIM::Contact*, void *data);
    bool		 m_bJoin;
    SIM::Socket *createSBSocket();
    void connected();
protected slots:
    void ping();
    void authOk();
    void authFailed();
protected:
    virtual bool done(unsigned code, Buffer &data, const char *headers);
    virtual QString contactName(void *clientData);
    virtual void setInvisible(bool bState);
    virtual bool compareData(void*, void*);
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, QString &statusIcon, QString *icons = NULL);
    virtual QString contactTip(void *_data);
    virtual SIM::CommandDef *infoWindows(SIM::Contact*, void *_data);
    virtual SIM::CommandDef *configWindows();
    virtual QWidget *infoWindow(QWidget *parent, SIM::Contact*, void *_data, unsigned id);
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual bool send(SIM::Message*, void*);
    virtual bool canSend(unsigned, void*);
    virtual void *processEvent(SIM::Event*);
    virtual QWidget *searchWindow(QWidget *parent);
    virtual bool isMyData(SIM::clientData*&, SIM::Contact*&);
    virtual bool createData(SIM::clientData*&, SIM::Contact*);
    SIM::Socket *createSocket();
    void getLine(const char*);
    void clearPackets();
    void sendStatus();
    void checkEndSync();
    void processLSG(unsigned id, const char *name);
    void processLST(const char *mail, const char *alias, unsigned state, unsigned id);
    virtual void	packet_ready();
    virtual void	connect_ready();
    virtual void	setStatus(unsigned status);
    virtual void	disconnected();
    std::string		getValue(const char *key, const char *str);
    std::string		getHeader(const char *name, const char *headers);
    unsigned			m_packetId;
    unsigned			m_pingTime;
    std::list<MSNPacket*>m_packets;
    MSNServerMessage	*m_msg;
    std::string				m_curBuddy;
    void		 requestLoginHost(const char *url);
    void		 requestTWN(const char *url);
    enum AuthState
    {
        None,
        LoginHost,
        TWN
    };
    AuthState	 m_state;
    QString 	 m_authChallenge;
    QString	     m_init_mail;
    QString 	 m_new_mail;
    bool		 m_bFirstTry;
    bool		 m_bHTTP;
    bool		 m_bFirst;
    unsigned	 m_nBuddies;
    unsigned	 m_nGroups;
    friend class MSNPacket;
    friend class UsrPacket;
    friend class QryPacket;
    friend class SynPacket;
    friend class MSNServerMessage;
    friend class SBSocket;
};

class MSNFileTransfer : public QObject, public SIM::FileTransfer, public SIM::ClientSocketNotify, public SIM::ServerSocketNotify
{
    Q_OBJECT
public:
    MSNFileTransfer(SIM::FileMessage *msg, MSNClient *client, MSNUserData *data);
    ~MSNFileTransfer();
    void connect();
    void listen();
    void setSocket(SIM::Socket *s);
    unsigned ip1;
    unsigned ip2;
    unsigned short port1;
    unsigned short port2;
    unsigned auth_cookie;
    unsigned cookie;

protected slots:

    void timeout();
protected:
    enum State
    {
        None,
        ConnectIP1,
        ConnectIP2,
        ConnectIP3,
        Connected,
        Send,
        Wait,
        Listen,
        Receive,
        Incoming,
        WaitDisconnect,
        WaitBye
    };
    virtual bool    error_state(const QString &err, unsigned code);
    virtual void	packet_ready();
    virtual void	connect_ready();
    virtual void	write_ready();
    virtual void	startReceive(unsigned pos);
    virtual bool	accept(SIM::Socket*, unsigned long ip);
    virtual void	bind_ready(unsigned short port);
    virtual bool	error(const char *err);
    void			send(const char *line);
    bool			getLine(const char *line);
    bool			m_bHeader;
    unsigned		m_size;
    State			m_state;
    SIM::ClientSocket    *m_socket;
    MSNClient		*m_client;
    MSNUserData		*m_data;
    QTimer			*m_timer;
};

#endif

