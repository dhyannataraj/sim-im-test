/***************************************************************************
                          icq.h  -  description
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

#ifndef _ICQ_H
#define _ICQ_H

#include "simapi.h"

const unsigned IcqCmdBase	= 0x00040000;

const unsigned EventSearch			= IcqCmdBase;
const unsigned EventSearchDone		= IcqCmdBase + 1;
const unsigned EventAutoReplyFail	= IcqCmdBase + 2;
const unsigned EventRandomChat		= IcqCmdBase + 3;
const unsigned EventRandomChatInfo	= IcqCmdBase + 4;
const unsigned EventServiceReady	= IcqCmdBase + 5;

const unsigned CmdVisibleList		= IcqCmdBase;
const unsigned CmdInvisibleList		= IcqCmdBase + 1;
const unsigned CmdChangeEncoding	= IcqCmdBase + 2;
const unsigned CmdAllEncodings		= IcqCmdBase + 3;
const unsigned CmdGroups			= IcqCmdBase + 4;
const unsigned CmdCheckInvisibleAll	= IcqCmdBase + 5;
const unsigned CmdCheckInvisible	= IcqCmdBase + 6;
const unsigned CmdUrlInput			= IcqCmdBase + 7;
const unsigned CmdIcqSendMessage	= IcqCmdBase + 8;

const unsigned MenuEncoding			= IcqCmdBase;
const unsigned MenuSearchResult		= IcqCmdBase + 1;
const unsigned MenuCheckInvisible	= IcqCmdBase + 2;
const unsigned MenuIcqGroups		= IcqCmdBase + 2;

class ICQProtocol : public Protocol
{
public:
    ICQProtocol(Plugin *plugin);
    ~ICQProtocol();
    Client	*createClient(const char *cfg);
    const CommandDef *description();
    const CommandDef *statusList();
    static const CommandDef *_statusList();
    virtual const DataDef *userDataDef();
    static const DataDef *icqUserData;
};

class AIMProtocol : public Protocol
{
public:
    AIMProtocol(Plugin *plugin);
    ~AIMProtocol();
    Client	*createClient(const char *cfg);
    const CommandDef *description();
    const CommandDef *statusList();
    virtual const DataDef *userDataDef();
    static const DataDef *icqUserData;
};

typedef struct ICQData
{
    unsigned	ShowAllEncodings;
    char		*DefaultEncoding;
} ICQData;

class ICQPlugin : public Plugin
{
public:
    ICQPlugin(unsigned base, const char *cfg);
    virtual ~ICQPlugin();
    unsigned OscarPacket;
    unsigned ICQDirectPacket;
    PROP_BOOL(ShowAllEncodings);
    PROP_STR(DefaultEncoding);
    unsigned RetrySendDND;
    unsigned RetrySendOccupied;
    static Protocol *m_icq;
    static Protocol *m_aim;
    static ICQPlugin *icq_plugin;
    void registerMessages();
    void unregisterMessages();
protected:
    virtual string getConfig();
    ICQData	 data;
};

#endif

