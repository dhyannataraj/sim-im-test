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
    unsigned EventAgentFound;
    unsigned EventAgentInfo;
	unsigned EventAgentRegister;
    unsigned EventSearch;
    unsigned EventSearchDone;
    unsigned CmdSendMessage;
    unsigned CmdGroups;
    unsigned MenuSearchResult;
    unsigned MenuGroups;
    void registerMessages();
    void unregisterMessages();
protected:
    Protocol *m_protocol;
};

#endif

