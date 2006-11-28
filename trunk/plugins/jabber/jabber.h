/***************************************************************************
                          jabber.h  -  description
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

#ifndef _JABBER_H
#define _JABBER_H

#include "contacts.h"

const unsigned JabberCmdBase	= 0x00050000;

const unsigned EventAgentFound		= JabberCmdBase + 1;
const unsigned EventAgentInfo		= JabberCmdBase + 2;
const unsigned EventAgentRegister	= JabberCmdBase + 3;
const unsigned EventSearch			= JabberCmdBase + 4;
const unsigned EventSearchDone		= JabberCmdBase + 5;
const unsigned EventDiscoItem		= JabberCmdBase + 6;
const unsigned EventVCard			= JabberCmdBase + 7;
const unsigned EventClientVersion	= JabberCmdBase + 8;
const unsigned EventClientLastInfo	= JabberCmdBase + 9;
const unsigned EventClientTimeInfo	= JabberCmdBase + 10;

const unsigned long CmdJabberMessage		= JabberCmdBase + 1;
const unsigned long CmdGroups			= JabberCmdBase + 2;
const unsigned long CmdBack				= JabberCmdBase + 3;
const unsigned long CmdForward			= JabberCmdBase + 4;
const unsigned long CmdUrl				= JabberCmdBase + 5;
const unsigned long CmdBrowseInfo		= JabberCmdBase + 6;
const unsigned long CmdBrowseSearch		= JabberCmdBase + 7;
const unsigned long CmdRegister			= JabberCmdBase + 8;
const unsigned long CmdNode				= JabberCmdBase + 9;
const unsigned long CmdBrowseMode		= JabberCmdBase + 10;
const unsigned long CmdBrowseConfigure	= JabberCmdBase + 11;
const unsigned long CmdOneLevel			= JabberCmdBase + 12;
const unsigned long CmdAllLevels			= JabberCmdBase + 13;
const unsigned long CmdModeDisco			= JabberCmdBase + 14;
const unsigned long CmdModeBrowse		= JabberCmdBase + 15;
const unsigned long CmdModeAgents		= JabberCmdBase + 16;

const unsigned long MenuSearchResult		= JabberCmdBase + 1;
const unsigned long MenuJabberGroups		= JabberCmdBase + 2;
const unsigned long MenuBrowser			= JabberCmdBase + 3;

const unsigned long BarBrowser			= JabberCmdBase + 1;

const unsigned BROWSE_DISCO		= 1;
const unsigned BROWSE_BROWSE	= 2;
const unsigned BROWSE_AGENTS	= 4;

class JabberProtocol : public SIM::Protocol
{
public:
    JabberProtocol(SIM::Plugin *plugin);
    ~JabberProtocol();
    SIM::Client	*createClient(Buffer *cfg);
    const SIM::CommandDef *description();
    const SIM::CommandDef *statusList();
    virtual const SIM::DataDef *userDataDef();
};

struct JabberData
{
    SIM::Data		browser_bar[7];
    SIM::Data		BrowserHistory;
    SIM::Data		AllLevels;
    SIM::Data		BrowseType;
};

class JabberPlugin : public SIM::Plugin
{
public:
    JabberPlugin(unsigned base, Buffer *cfg);
    virtual ~JabberPlugin();
    unsigned JabberPacket;
    void registerMessages();
    void unregisterMessages();
    JabberData	data;
    PROP_UTF8(BrowserHistory);
    PROP_BOOL(AllLevels);
    PROP_ULONG(BrowseType);
    static JabberPlugin *plugin;
protected:
    virtual std::string getConfig();
    SIM::Protocol *m_protocol;
};

#endif

