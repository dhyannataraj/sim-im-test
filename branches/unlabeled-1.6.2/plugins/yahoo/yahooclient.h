/***************************************************************************
                          yahooclient.h  -  description
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

#ifndef _YAHOOCLIENT_H
#define _YAHOOCLIENT_H

#include "simapi.h"
#include "socket.h"
#include "stl.h"

const unsigned YAHOO_SIGN = 9;

const unsigned long YAHOO_STATUS_AVAILABLE		= 0;
const unsigned long YAHOO_STATUS_BRB			= 1;
const unsigned long YAHOO_STATUS_BUSY			= 2;
const unsigned long YAHOO_STATUS_NOTATHOME		= 3;
const unsigned long YAHOO_STATUS_NOTATDESK		= 4;
const unsigned long YAHOO_STATUS_NOTINOFFICE	= 5;
const unsigned long YAHOO_STATUS_ONPHONE		= 6;
const unsigned long YAHOO_STATUS_ONVACATION		= 7;
const unsigned long YAHOO_STATUS_OUTTOLUNCH		= 8;
const unsigned long YAHOO_STATUS_STEPPEDOUT		= 9;
const unsigned long YAHOO_STATUS_INVISIBLE		= 12;
const unsigned long YAHOO_STATUS_CUSTOM			= 99;
const unsigned long YAHOO_STATUS_OFFLINE		= (unsigned long)(-1);

const unsigned short YAHOO_SERVICE_LOGON			= 0x01;
const unsigned short YAHOO_SERVICE_LOGOFF			= 0x02;
const unsigned short YAHOO_SERVICE_ISAWAY			= 0x03;
const unsigned short YAHOO_SERVICE_ISBACK			= 0x04;
const unsigned short YAHOO_SERVICE_IDLE				= 0x05;
const unsigned short YAHOO_SERVICE_MESSAGE			= 0x06;
const unsigned short YAHOO_SERVICE_IDACT			= 0x07;
const unsigned short YAHOO_SERVICE_IDDEACT			= 0x08;
const unsigned short YAHOO_SERVICE_MAILSTAT			= 0x09;
const unsigned short YAHOO_SERVICE_USERSTAT			= 0x0A;
const unsigned short YAHOO_SERVICE_NEWMAIL			= 0x0B;
const unsigned short YAHOO_SERVICE_CHATINVITE		= 0x0C;
const unsigned short YAHOO_SERVICE_CALENDAR			= 0x0D;
const unsigned short YAHOO_SERVICE_NEWPERSONALMAIL	= 0x0E;
const unsigned short YAHOO_SERVICE_NEWCONTACT		= 0x0F;
const unsigned short YAHOO_SERVICE_ADDIDENT			= 0x10;
const unsigned short YAHOO_SERVICE_ADDIGNORE		= 0x11;
const unsigned short YAHOO_SERVICE_PING				= 0x12;
const unsigned short YAHOO_SERVICE_GOTGROUPRENAME	= 0x13;
const unsigned short YAHOO_SERVICE_SYSMESSAGE		= 0x14;
const unsigned short YAHOO_SERVICE_PASSTHROUGH2		= 0x16;
const unsigned short YAHOO_SERVICE_CONFINVITE		= 0x18;
const unsigned short YAHOO_SERVICE_CONFLOGON		= 0x19;
const unsigned short YAHOO_SERVICE_CONFDECLINE		= 0x1A;
const unsigned short YAHOO_SERVICE_CONFLOGOFF		= 0x1B;
const unsigned short YAHOO_SERVICE_CONFADDINVITE	= 0x1C;
const unsigned short YAHOO_SERVICE_CONFMSG			= 0x1D;
const unsigned short YAHOO_SERVICE_CHATLOGON		= 0x1E;
const unsigned short YAHOO_SERVICE_CHATLOGOFF		= 0x1F;
const unsigned short YAHOO_SERVICE_CHATMSG			= 0x20;
const unsigned short YAHOO_SERVICE_GAMELOGON		= 0x28;
const unsigned short YAHOO_SERVICE_GAMELOGOFF		= 0x29;
const unsigned short YAHOO_SERVICE_GAMEMSG			= 0x2A;
const unsigned short YAHOO_SERVICE_FILETRANSFER		= 0x46;
const unsigned short YAHOO_SERVICE_VOICECHAT		= 0x4A;
const unsigned short YAHOO_SERVICE_NOTIFY			= 0x4B;
const unsigned short YAHOO_SERVICE_VERIFY			= 0x4C;
const unsigned short YAHOO_SERVICE_P2PFILEXFER		= 0x4D;
const unsigned short YAHOO_SERVICE_PEERTOPEER		= 0x4F;
const unsigned short YAHOO_SERVICE_WEBCAM			= 0x4E;
const unsigned short YAHOO_SERVICE_AUTHRESP			= 0x54;
const unsigned short YAHOO_SERVICE_LIST				= 0x55;
const unsigned short YAHOO_SERVICE_AUTH				= 0x57;
const unsigned short YAHOO_SERVICE_ADDBUDDY			= 0x83;
const unsigned short YAHOO_SERVICE_REMBUDDY			= 0x84;
const unsigned short YAHOO_SERVICE_IGNORECONTACT	= 0x85;
const unsigned short YAHOO_SERVICE_REJECTCONTACT	= 0x86;
const unsigned short YAHOO_SERVICE_GROUPRENAME		= 0x89;
const unsigned short YAHOO_SERVICE_CHATONLINE		= 0x96;
const unsigned short YAHOO_SERVICE_CHATGOTO			= 0x97;
const unsigned short YAHOO_SERVICE_CHATJOIN			= 0x98;
const unsigned short YAHOO_SERVICE_CHATLEAVE		= 0x99;
const unsigned short YAHOO_SERVICE_CHATEXIT			= 0x9B;
const unsigned short YAHOO_SERVICE_CHATLOGOUT		= 0xA0;
const unsigned short YAHOO_SERVICE_CHATPING			= 0xA1;
const unsigned short YAHOO_SERVICE_COMMENT			= 0xA8;

typedef struct YahooUserData
{
    clientData	base;
    Data		Login;
    Data		Nick;
    Data		First;
    Data		Last;
    Data		EMail;
    Data		Status;
    Data		bAway;
    Data		AwayMessage;
    Data		OnlineTime;
    Data		StatusTime;
    Data		Group;
    Data		bChecked;
    Data		bTyping;
    Data		Encoding;
} YahooUserData;

typedef struct YahooClientData
{
    Data	Server;
    Data	Port;
    Data	FTServer;
    Data	FTPort;
    Data	CookieY;
    Data	CookieT;
    Data	UseHTTP;
    Data	AutoHTTP;
    YahooUserData	owner;
} YahooClientData;

typedef pair<unsigned, string> PARAM;

class QTextCodec;

class YahooClient : public TCPClient
{
    Q_OBJECT
public:
    YahooClient(Protocol*, const char *cfg);
    ~YahooClient();
    PROP_STR(Server);
    PROP_USHORT(Port);
    PROP_STR(FTServer);
    PROP_USHORT(FTPort);
    PROP_STR(CookieY);
    PROP_STR(CookieT);
    PROP_BOOL(UseHTTP);
    PROP_BOOL(AutoHTTP);
    virtual string getConfig();
    QString getLogin();
    void setLogin(const QString&);
    string  dataName(void*);
    YahooClientData	data;
    virtual void contactInfo(void *_data, unsigned long &status, unsigned &style, const char *&statusIcon, string *icons = NULL);
    YahooUserData *findContact(const char *id, const char *grp, Contact *&contact, bool bSend=true);
    QString toUnicode(const char *str, YahooUserData *client_data);
    string  fromUnicode(const QString &str, YahooUserData *client_data);
    QTextCodec *getCodec(const char *encoding);
	static QTextCodec *_getCodec(const char *encoding);
	static QString toUnicode(const char *serverText, const char *clientName, unsigned contactId);
    void sendFile(FileMessage *msg, FileMessage::Iterator &it, YahooUserData *data);
protected slots:
    void ping();
protected:
    void	*processEvent(Event*);
    void	setStatus(unsigned status);
    virtual void setInvisible(bool bState);
    void	disconnected();
    Socket  *createSocket();
    string	name();
    QWidget	*setupWnd();
    bool isMyData(clientData*&, Contact*&);
    bool createData(clientData*&, Contact*);
    void setupContact(Contact*, void *data);
    bool send(Message*, void *data);
    bool canSend(unsigned type, void *data);
    CommandDef *infoWindows(Contact*, void *_data);
    CommandDef *configWindows();
    QWidget *infoWindow(QWidget *parent, Contact*, void *_data, unsigned id);
    QWidget *configWindow(QWidget *parent, unsigned id);
    virtual QWidget *searchWindow();
    virtual QString contactTip(void *_data);
    virtual void connect_ready();
    virtual void packet_ready();
    void process_packet();
    void process_auth(const char *method, const char *seed, const char *sn);
    void process_auth_0x0b(const char *seed, const char *sn);
    void sendPacket(unsigned short service, unsigned long status=YAHOO_STATUS_AVAILABLE);
    void addParam(unsigned key, const char *value);
    void loadList(const char *data);
    void contact_added(const char *id, const char *message);
    void contact_rejected(const char *id, const char *message);
    void processStatus(unsigned short service, const char *id,
                       const char *_state, const char *_msg,
                       const char *_away, const char *_idle);
    void messageReceived(Message *msg, const char *id);
    void process_message(const char *id, const char *msg, const char *utf);
    void process_file(const char *id, const char *fileName, const char *fileSize, const char *msg, const char *url);
    void notify(const char *id, const char *msg, const char *state);
    void sendMessage(const QString &msgText, Message *msg, YahooUserData*);
    void sendTyping(YahooUserData*, bool);
    void addBuddy(YahooUserData*);
    void removeBuddy(YahooUserData*);
    void moveBuddy(YahooUserData *data, const char *grp);
    void sendStatus(unsigned long status, const char *msg = NULL);
    list<PARAM>			m_values;
	list<FileMessage*>	m_waitMsg;
    unsigned long  m_session;
    unsigned long  m_pkt_status;
    unsigned short m_data_size;
    unsigned short m_service;
    string	m_session_id;
    bool m_bHeader;
    bool m_bHTTP;
    bool m_bFirstTry;
    void authOk();
};

typedef struct YahooFileData
{
    Data	MsgText;
    Data	Url;
} YahooFileData;

class YahooFileMessage : public FileMessage
{
public:
    YahooFileMessage(const char *cfg=NULL);
    ~YahooFileMessage();
    PROP_STR(MsgText);
    PROP_STR(Url);
    virtual	string save();
    virtual QString getText() const;
protected:
    YahooFileData data;
};

class YahooFileTransfer : public FileTransfer
{
public:
	YahooFileTransfer(FileMessage *msg, YahooUserData *data, YahooClient *client);
protected:
	FileMessage::Iterator m_it;
	YahooClient		*m_client;
	YahooUserData	*m_data;
    virtual void startReceive(unsigned pos);
};

#endif

