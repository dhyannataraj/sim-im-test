/***************************************************************************
                          sms.h  -  description
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

#ifndef _SMS_H
#define _SMS_H

#include "simapi.h"
#include "socket.h"

class SMSProtocol;
class GsmTA;
class MainInfo;

const unsigned SMSCmdBase			= 0x00080000;
const unsigned MessagePhoneCall		= SMSCmdBase;

class SMSPlugin : public QObject, public Plugin
{
    Q_OBJECT
public:
    SMSPlugin(unsigned);
    virtual ~SMSPlugin();
    static unsigned SerialPacket;
    void setPhoneCol();
    void setPhoneCol(MainInfo *w);
    void removePhoneCol();
    void removePhoneCol(MainInfo *w);
protected:
    bool eventFilter(QObject *obj, QEvent *e);
    SMSProtocol *m_protocol;
};

class SMSProtocol : public Protocol
{
public:
    SMSProtocol(Plugin *plugin);
    ~SMSProtocol();
    Client	*createClient(const char *cfg);
    const CommandDef *description();
    const CommandDef *statusList();
    const DataDef *userDataDef();
};

typedef struct SMSClientData
{
    Data	Device;
    Data	BaudRate;
    Data	XonXoff;
    Data	Charge;
    Data	Charging;
    Data	Quality;
} SMSClientData;

const unsigned SMS_SIGN	= 6;

typedef struct smsUserData
{
    clientData	base;
    Data	Name;
    Data	Phone;
    Data	Index;
    Data	Type;
} smsUserData;

class SMSClient : public TCPClient
{
    Q_OBJECT
public:
    SMSClient(Protocol *protocol, const char *cfg);
    ~SMSClient();
    PROP_STR(Device);
    PROP_ULONG(BaudRate);
    PROP_BOOL(XonXoff);
    PROP_ULONG(Charge);
    PROP_BOOL(Charging);
    PROP_ULONG(Quality);
    string model();
    string oper();
protected slots:
    void error();
    void init();
    void ta_error();
    void charge(bool, unsigned);
    void quality(unsigned);
    void phoneCall(const QString&);
    void phonebookEntry(int, int, const QString&, const QString&);
    void callTimeout();
protected:
    virtual const char		*getServer() const;
    virtual unsigned short	getPort() const;
    virtual void	setStatus(unsigned status);
    virtual void	disconnected();
    virtual string	getConfig();
    virtual string	name();
    virtual string	dataName(void*);
    virtual bool	isMyData(clientData*&, Contact*&);
    virtual bool	createData(clientData*&, Contact*);
    virtual void	setupContact(Contact*, void *data);
    virtual bool	send(Message*, void *data);
    virtual bool	canSend(unsigned type, void *data);
    virtual void	packet_ready();
    virtual void	socketConnect();
    virtual CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual QWidget	*setupWnd();
    QString			m_callNumber;
    QTimer			*m_callTimer;
    Message			*m_call;
    bool			m_bCall;
    GsmTA			*m_ta;
    SMSClientData	data;
};

#endif

