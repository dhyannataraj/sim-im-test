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

class SMSPlugin : public Plugin
{
public:
    SMSPlugin(unsigned);
    virtual ~SMSPlugin();
    static unsigned SerialPacket;
protected:
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
    char		*Device;
    char		*InitString;
    unsigned	BaudRate;
    unsigned	XonXoff;
} SMSClientData;

const unsigned SMS_SIGN	= 6;

typedef struct smsUserData
{
    clientData	base;
    char		*Name;
    char		*Phone;
    unsigned	Index;
} smsUserData;

class SMSClient : public TCPClient
{
    Q_OBJECT
public:
    SMSClient(Protocol *protocol, const char *cfg);
    ~SMSClient();
    PROP_STR(Device);
    PROP_STR(InitString);
    PROP_ULONG(BaudRate);
    PROP_BOOL(XonXoff);
protected slots:
    void error();
    void init();
    void ta_error();
    void phonebookEntry(int, const QString&, const QString&);
protected:
    virtual const char		*getServer();
    virtual unsigned short	getPort();
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
    GsmTA			*m_ta;
    SMSClientData	data;
};

#endif

