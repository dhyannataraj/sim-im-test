/***************************************************************************
                          jabber.cpp  -  description
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

#include "jabber.h"
#include "jabberclient.h"
#include "simapi.h"
#include "core.h"

#include "xpm/jabber_online.xpm"
#include "xpm/jabber_offline.xpm"
#include "xpm/jabber_away.xpm"
#include "xpm/jabber_na.xpm"
#include "xpm/jabber_dnd.xpm"

Plugin *createJabberPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new JabberPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createJabberPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

JabberProtocol::JabberProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

JabberProtocol::~JabberProtocol()
{
}

Client *JabberProtocol::createClient(const char *cfg)
{
    return new JabberClient(this, cfg);
}


static CommandDef jabber_descr =
    {
        0,
        I18N_NOOP("Jabber"),
        "Jabber_online",
        NULL,
        NULL,
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INFO | PROTOCOL_SEARCH_ONLINE | PROTOCOL_AR,
        NULL,
        NULL
    };

const CommandDef *JabberProtocol::description()
{
    return &jabber_descr;
}

static CommandDef jabber_status_list[] =
    {
        {
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "Jabber_online",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            STATUS_AWAY,
            I18N_NOOP("Away"),
            "Jabber_away",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            STATUS_NA,
            I18N_NOOP("N/A"),
            "Jabber_na",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            STATUS_DND,
            I18N_NOOP("Do not Disturb"),
            "Jabber_dnd",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },

        {
            STATUS_OFFLINE,
            I18N_NOOP("Offline"),
            "Jabber_offline",
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        },
        {
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            0,
            0,
            0,
            NULL,
            NULL
        }
    };

const CommandDef *JabberProtocol::statusList()
{
    return jabber_status_list;
}

JabberPlugin::JabberPlugin(unsigned base)
        : Plugin(base)
{
    JabberPacket = registerType();
    getContacts()->addPacketType(JabberPacket, jabber_descr.text, PACKET_TEXT);

    EventAgentFound = registerType();
    EventAgentInfo	= registerType();
    EventSearch		= registerType();
    EventSearchDone	= registerType();

    MenuSearchResult = registerType();
    MenuGroups		 = registerType();
    CmdSendMessage	 = registerType();
    CmdGroups		 = registerType();

    IconDef icon;
    icon.name = "Jabber_online";
    icon.xpm = jabber_online;
    icon.isSystem = false;

    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name = "Jabber_offline";
    icon.xpm = jabber_offline;
    eIcon.process();

    icon.name = "Jabber_away";
    icon.xpm = jabber_away;
    eIcon.process();

    icon.name = "Jabber_na";
    icon.xpm = jabber_na;
    eIcon.process();

    icon.name = "Jabber_dnd";
    icon.xpm = jabber_dnd;
    eIcon.process();

    Event eMenuSearch(EventMenuCreate, (void*)MenuSearchResult);
    eMenuSearch.process();
    Event eMenuGroups(EventMenuCreate, (void*)MenuGroups);
    eMenuGroups.process();

    Command	cmd;
    cmd->id			 = CmdSendMessage;
    cmd->text		 = I18N_NOOP("&Message");
    cmd->icon		 = "message";
    cmd->menu_id	 = MenuSearchResult;
    cmd->menu_grp	 = 0x1000;
    cmd->flags		 = COMMAND_DEFAULT;
    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id			 = CmdInfo;
    cmd->text		 = I18N_NOOP("User &info");
    cmd->icon		 = "info";
    cmd->menu_grp	 = 0x1001;
    eCmd.process();

    cmd->id			 = CmdGroups;
    cmd->text		 = I18N_NOOP("&Add to group");
    cmd->icon		 = NULL;
    cmd->menu_grp	 = 0x1002;
    cmd->popup_id	 = MenuGroups;
    eCmd.process();

    cmd->id			 = CmdGroups;
    cmd->text		 = "_";
    cmd->menu_id	 = MenuGroups;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    m_protocol = new JabberProtocol(this);
    registerMessages();
}

JabberPlugin::~JabberPlugin()
{
    unregisterMessages();

    Event eMenuSearch(EventMenuRemove, (void*)MenuSearchResult);
    eMenuSearch.process();

    Event eMenuGroups(EventMenuRemove, (void*)MenuGroups);
    eMenuGroups.process();

    delete m_protocol;
    getContacts()->removePacketType(JabberPacket);
}

#ifdef WIN32
#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif


