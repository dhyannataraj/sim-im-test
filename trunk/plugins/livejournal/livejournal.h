/***************************************************************************
                          livejournal.h  -  description
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

#ifndef _LIVEJOURNAL_H
#define _LIVEJOURNAL_H

#include "simapi.h"
#include "stl.h"
#include "buffer.h"
#include "socket.h"

const unsigned JournalCmdBase			= 0x00070000;
const unsigned MessageJournal			= JournalCmdBase;
const unsigned CmdDeleteJournalMessage	= JournalCmdBase + 1;
const unsigned CmdMenuWeb				= JournalCmdBase + 2;
const unsigned MenuWeb					= JournalCmdBase + 0x10;

const unsigned LIVEJOURNAL_SIGN	= 5;

typedef struct LiveJournalUserData
{
    clientData		base;
    char			*User;
    unsigned		bChecked;
} LiveJournalUserData;

typedef struct JournalMessageData
{
    char			*Subject;
    unsigned		Private;
    unsigned		Time;
    unsigned		ID;
    unsigned		OldID;
    char			*Mood;
} JournalMessageData;

class JournalMessage : public Message
{
public:
    JournalMessage(const char *cfg = NULL);
    ~JournalMessage();
    string save();
    PROP_UTF8(Subject);
    PROP_ULONG(Private);
    PROP_ULONG(Time);
    PROP_ULONG(ID);
    PROP_ULONG(OldID);
    PROP_STR(Mood);
protected:
    QString presentation();
    JournalMessageData data;
};

class CorePlugin;

class LiveJournalPlugin : public Plugin
{
public:
    LiveJournalPlugin(unsigned);
    virtual ~LiveJournalPlugin();
    static CorePlugin *core;
    static unsigned MenuCount;
protected:
    Protocol *m_protocol;
};

class LiveJournalProtocol : public Protocol
{
public:
    LiveJournalProtocol(Plugin *plugin);
    ~LiveJournalProtocol();
    Client	*createClient(const char *cfg);
    const CommandDef *description();
    const CommandDef *statusList();
    const DataDef *userDataDef();
};

typedef struct LiveJournalClientData
{
    char		*Server;
    char		*URL;
    unsigned	Port;
    unsigned	Interval;
    void		*Mood;
    unsigned	Moods;
    void		*Menu;
    void		*MenuUrl;
    LiveJournalUserData	owner;
} LiveJournalClientData;

class LiveJournalClient;

class LiveJournalRequest
{
public:
    LiveJournalRequest(LiveJournalClient *client, const char *mode);
    virtual ~LiveJournalRequest();
    void addParam(const char *key, const char *value);
    void result(Buffer*);
    virtual void result(const char *key, const char *value) = 0;
protected:
    LiveJournalClient *m_client;
    Buffer m_buffer;
    bool getLine(Buffer *b, string &line);
    friend class LiveJournalClient;
};

class QTimer;

class LiveJournalClient : public TCPClient, public EventReceiver
{
    Q_OBJECT
public:
    LiveJournalClient(Protocol*, const char *cfg);
    ~LiveJournalClient();
    PROP_STR(Server);
    PROP_STR(URL);
    PROP_USHORT(Port);
    PROP_ULONG(Interval);
    PROP_STRLIST(Mood);
    PROP_ULONG(Moods);
    PROP_STRLIST(Menu);
    PROP_STRLIST(MenuUrl);
    void auth_fail(const char *err);
    void auth_ok();
    LiveJournalUserData	*findContact(const char *user, Contact *&contact);
protected slots:
    void timeout();
    void send();
protected:
    virtual string getConfig();
    virtual string name();
    virtual string dataName(void*);
    virtual QWidget	*setupWnd();
    virtual bool isMyData(clientData*&, Contact*&);
    virtual bool createData(clientData*&, Contact*);
    virtual void setupContact(Contact*, void *data);
    virtual bool send(Message*, void *data);
    virtual bool canSend(unsigned type, void *data);
    virtual void setStatus(unsigned status);
    virtual void socketConnect();
    virtual void disconnected();
    virtual void packet_ready();
    virtual void *processEvent(Event*);
    virtual void contactInfo(void*, unsigned long &curStatus, unsigned&, const char *&statusIcon, string *icons);
    CommandDef *configWindows();
    QWidget *configWindow(QWidget *parent, unsigned id);
    void statusChanged();
    QTimer  *m_timer;
    list<LiveJournalRequest*> m_requests;
    LiveJournalRequest		  *m_request;
    LiveJournalClientData	data;
    unsigned	 m_fetchId;
    friend class LiveJournalCfg;
    friend class LiveJournalRequest;
};

#endif

