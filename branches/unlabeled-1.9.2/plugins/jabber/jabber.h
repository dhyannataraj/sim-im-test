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

#include "simapi.h"

const unsigned JabberCmdBase	= 0x00050000;

const unsigned EventAgentFound		= JabberCmdBase + 1;
const unsigned EventAgentInfo		= JabberCmdBase + 2;
const unsigned EventAgentRegister	= JabberCmdBase + 3;
const unsigned EventSearch			= JabberCmdBase + 4;
const unsigned EventSearchDone		= JabberCmdBase + 5;
const unsigned EventDiscoItem		= JabberCmdBase + 6;
const unsigned EventVCard			= JabberCmdBase + 7;

const unsigned CmdJabberMessage		= JabberCmdBase + 1;
const unsigned CmdGroups			= JabberCmdBase + 2;
const unsigned CmdBrowser			= JabberCmdBase + 3;
const unsigned CmdBack				= JabberCmdBase + 4;
const unsigned CmdForward			= JabberCmdBase + 5;
const unsigned CmdUrl				= JabberCmdBase + 6;
const unsigned CmdBrowseInfo		= JabberCmdBase + 7;
const unsigned CmdBrowseSearch		= JabberCmdBase + 8;
const unsigned CmdRegister			= JabberCmdBase + 9;
const unsigned CmdNode				= JabberCmdBase + 10;
const unsigned CmdBrowseMode		= JabberCmdBase + 11;
const unsigned CmdBrowseConfigure	= JabberCmdBase + 12;
const unsigned CmdOneLevel			= JabberCmdBase + 13;
const unsigned CmdAllLevels			= JabberCmdBase + 14;
const unsigned CmdModeDisco			= JabberCmdBase + 15;
const unsigned CmdModeBrowse		= JabberCmdBase + 16;
const unsigned CmdModeAgents		= JabberCmdBase + 17;

const unsigned MenuSearchResult		= JabberCmdBase + 1;
const unsigned MenuJabberGroups		= JabberCmdBase + 2;
const unsigned MenuClients			= JabberCmdBase + 3;
const unsigned MenuBrowser			= JabberCmdBase + 4;
    
const unsigned BarBrowser			= JabberCmdBase + 1;


class JabberProtocol : public Protocol
{
public:
    JabberProtocol(Plugin *plugin);
    ~JabberProtocol();
    Client	*createClient(const char *cfg);
    const CommandDef *description();
    const CommandDef *statusList();
    virtual const DataDef *userDataDef();
};

class JabberPlugin : public Plugin
{
public:
    JabberPlugin(unsigned base);
    virtual ~JabberPlugin();
    unsigned JabberPacket;
    void registerMessages();
    void unregisterMessages();
protected:
    Protocol *m_protocol;
};

#endif

