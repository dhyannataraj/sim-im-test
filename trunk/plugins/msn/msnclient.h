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
#include "socket.h"

#include <list>
using namespace std;

const unsigned STATUS_BRB		= 0x10;
const unsigned STATUS_PHONE		= 0x11;
const unsigned STATUS_LUNCH		= 0x12;

const unsigned MSN_ACCEPT	= 0x0001;
const unsigned MSN_BLOCKED	= 0x0002;
const unsigned MSN_REVERSE  = 0x0004;
const unsigned MSN_FORWARD	= 0x0008;
const unsigned MSN_CHECKED	= 0x0010;

class SBSocket;

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
    unsigned	typing_time;
    SBSocket	*sb;
} MSNUserData;

typedef struct MSNClientData
{
    char		*Server;
    unsigned	Port;
    unsigned	ListVer;
    char		*ListRequests;
    MSNUserData	owner;
} MSNClientData;

class MSNClient;
class XfrPacket;

class SBSocket : public ClientSocketNotify
{
public:
    SBSocket(MSNClient *client, Contact *contact, MSNUserData *data);
    ~SBSocket();
    void connect(const char *addr, const char *session, const char *cookie, bool bDirection);
    void connect();
    bool send(Message *msg);
    void timer(unsigned now);
    void setTyping(bool bTyping);
    bool cancelMessage(Message *msg);
protected:
    enum State
    {
        Unknown,
        ConnectingSend,
        ConnectingReceive,
        WaitJoin,
        Connected
    };
    virtual bool error_state(const char *err, unsigned code);
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
    State			m_state;
    Contact			*m_contact;
    MSNClient		*m_client;
    MSNUserData		*m_data;
    string			m_session;
    string			m_cookie;
    ClientSocket	*m_socket;
    unsigned		m_packet_id;
    string			m_message;
    unsigned		m_messageSize;
    bool			m_bTyping;
    XfrPacket		*m_packet;
    list<Message*>  m_queue;
    QString			m_msgText;
    QString			m_msgPart;
    unsigned		m_msg_id;
};

const unsigned LR_CONTACTxCHANGED		= 0;
const unsigned LR_CONTACTxREMOVED		= 1;
const unsigned LR_CONTACTxREMOVED_AL	= 2;
const unsigned LR_CONTACTxREMOVED_BL	= 3;
const unsigned LR_GROUPxCHANGED			= 4;
const unsigned LR_GROUPxREMOVED			= 5;

typedef struct MSNListRequest
{
    unsigned	Type;
    string		Name;
} MSNListRequest;

class MSNPacket;
class MSNServerMessage;

class MSNClient : public TCPClient, public EventReceiver
{
    Q_OBJECT
public:
    MSNClient(Protocol*, const char *cfg);
    ~MSNClient();
    virtual string name();
    virtual QWidget	*setupWnd();
    virtual string getConfig();
    PROP_STR(Server);
    PROP_ULONG(Port);
    PROP_ULONG(ListVer);
    PROP_UTF8(ListRequests);
    QString getLogin();
    QString unquote(const QString&);
    QString quote(const QString&);
    void sendLine(const char *line, bool crlf = true);
    void setLogin(const QString&);
    MSNClientData	data;
    string dataName(void*);
    MSNUserData *findContact(const char *mail, const char *name, Contact *&contact);
    MSNUserData *findContact(const char *mail, Contact *&contact);
    MSNUserData *findGroup(unsigned long id, const char *name, Group *&grp);
    void auth_message(Contact *contact, unsigned type, MSNUserData *data);
    list<MSNListRequest> m_requests;
    void processRequests();
    MSNListRequest *findRequest(unsigned long id, unsigned type, bool bDelete=false);
    MSNListRequest *findRequest(const char *name, unsigned type, bool bDelete=false);
    bool add(const char *mail, const char *name, unsigned grp);
    list<SBSocket*> m_SBsockets;
protected slots:
    void ping();
    void authOk();
    void authFailed();
protected:
    virtual void setInvisible(bool bState);
    virtual bool compareData(void*, void*);
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, const char *&statusIcon, string *icons = NULL);
    virtual QString contactTip(void *_data);
    virtual CommandDef *infoWindows(Contact*, void *_data);
    virtual CommandDef *configWindows();
    virtual QWidget *infoWindow(QWidget *parent, Contact*, void *_data, unsigned id);
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual bool send(Message*, void*);
    virtual bool canSend(unsigned, void*);
    virtual void setupContact(Contact*, void *data);
    virtual void *processEvent(Event*);
    virtual QWidget *searchWindow();
    void getLine(const char*);
    void clearPackets();
    void sendStatus();
    virtual void	packet_ready();
    virtual void	connect_ready();
    virtual void	setStatus(unsigned status);
    virtual void	disconnected();
    unsigned			m_packetId;
    unsigned			m_pingTime;
    MSNPacket			*m_curPacket;
    list<MSNPacket*>	m_packets;
    MSNServerMessage	*m_msg;
    friend class MSNPacket;
    friend class UsrPacket;
    friend class QryPacket;
    friend class MSNServerMessage;
};

#endif

