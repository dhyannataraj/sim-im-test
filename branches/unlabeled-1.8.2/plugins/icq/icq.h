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
} ICQData;

class ICQPlugin : public Plugin
{
public:
    ICQPlugin(unsigned base, const char *cfg);
    virtual ~ICQPlugin();
    unsigned OscarPacket;
    unsigned ICQDirectPacket;
    PROP_BOOL(ShowAllEncodings);
    unsigned EventSearch;
    unsigned EventSearchDone;
    unsigned EventAutoReplyFail;
    unsigned EventRandomChat;
    unsigned EventRandomChatInfo;
    unsigned EventServiceReady;
    unsigned CmdVisibleList;
    unsigned CmdInvisibleList;
    unsigned CmdChangeEncoding;
    unsigned CmdAllEncodings;
    unsigned CmdSendMessage;
    unsigned CmdGroups;
    unsigned CmdCheckInvisibleAll;
    unsigned CmdCheckInvisible;
    unsigned MenuEncoding;
    unsigned MenuSearchResult;
    unsigned MenuGroups;
    unsigned MenuCheckInvisible;
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

