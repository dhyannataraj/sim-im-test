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
#include "xpm/jabber_ffc.xpm"
#include "xpm/jabber_invisible.xpm"
#include "xpm/icq_online.xpm"
#include "xpm/icq_offline.xpm"
#include "xpm/icq_away.xpm"
#include "xpm/icq_na.xpm"
#include "xpm/icq_dnd.xpm"
#include "xpm/icq_ffc.xpm"
#include "xpm/icq_invisible.xpm"
#include "xpm/msn_online.xpm"
#include "xpm/msn_offline.xpm"
#include "xpm/msn_away.xpm"
#include "xpm/msn_na.xpm"
#include "xpm/msn_dnd.xpm"
#include "xpm/msn_invisible.xpm"
#include "xpm/aim_online.xpm"
#include "xpm/aim_offline.xpm"
#include "xpm/aim_away.xpm"
#include "xpm/yahoo_online.xpm"
#include "xpm/yahoo_offline.xpm"
#include "xpm/yahoo_away.xpm"
#include "xpm/yahoo_na.xpm"
#include "xpm/yahoo_dnd.xpm"
#include "xpm/yahoo_ffc.xpm"
#include "xpm/register.xpm"

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
        "Jabber_invisible",
        NULL,
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INFO | PROTOCOL_SEARCH_ONLINE | PROTOCOL_AR | PROTOCOL_INVISIBLE,
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
            STATUS_FFC,
            I18N_NOOP("Free for chat"),
            "Jabber_ffc",
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

#if defined(WIN32) && (QT_VERSION < 300)
void qInitJpeg();
#endif

JabberPlugin::JabberPlugin(unsigned base)
        : Plugin(base)
{
#if defined(WIN32) && (QT_VERSION < 300)
    qInitJpeg();
#endif

    JabberPacket = registerType();
    getContacts()->addPacketType(JabberPacket, jabber_descr.text, true);

    EventAgentFound = registerType();
    EventAgentInfo	= registerType();
    EventAgentRegister = registerType();
    EventSearch		= registerType();
    EventSearchDone	= registerType();
    EventVCard		= registerType();

    MenuSearchResult = registerType();
    MenuGroups		 = registerType();
    CmdSendMessage	 = registerType();
    CmdGroups		 = registerType();
    CmdBrowser		 = registerType();
    BarBrowser		 = registerType();
    CmdBack			 = registerType();
    CmdForward		 = registerType();
    CmdUrl			 = registerType();
    EventDiscoItem	 = registerType();
    CmdInfo			 = registerType();
    CmdRegister		 = registerType();
    CmdSearch		 = registerType();
    CmdNode			 = registerType();
    MenuClients		 = registerType();

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

    icon.name = "Jabber_ffc";
    icon.xpm = jabber_ffc;
    eIcon.process();

    icon.name = "Jabber_invisible";
    icon.xpm = jabber_invisible;
    eIcon.process();

    icon.name = "ICQ_offline";
    icon.xpm = icq_offline;
    eIcon.process();

    icon.name = "ICQ_online";
    icon.xpm = icq_online;
    eIcon.process();

    icon.name = "ICQ_away";
    icon.xpm = icq_away;
    eIcon.process();

    icon.name = "ICQ_na";
    icon.xpm = icq_na;
    eIcon.process();

    icon.name = "ICQ_dnd";
    icon.xpm = icq_dnd;
    eIcon.process();

    icon.name = "ICQ_ffc";
    icon.xpm = icq_ffc;
    eIcon.process();

    icon.name = "ICQ_invisible";
    icon.xpm = icq_invisible;
    eIcon.process();

    icon.name = "MSN_offline";
    icon.xpm = msn_offline;
    eIcon.process();

    icon.name = "MSN_online";
    icon.xpm = msn_online;
    eIcon.process();

    icon.name = "MSN_away";
    icon.xpm = msn_away;
    eIcon.process();

    icon.name = "MSN_na";
    icon.xpm = msn_na;
    eIcon.process();

    icon.name = "MSN_dnd";
    icon.xpm = msn_dnd;
    eIcon.process();

    icon.name = "MSN_invisible";
    icon.xpm = msn_invisible;
    eIcon.process();

    icon.name = "AIM_offline";
    icon.xpm = aim_offline;
    eIcon.process();

    icon.name = "AIM_online";
    icon.xpm = aim_online;
    eIcon.process();

    icon.name = "AIM_away";
    icon.xpm = aim_away;
    eIcon.process();

    icon.name = "Yahoo_offline";
    icon.xpm = yahoo_offline;
    eIcon.process();

    icon.name = "Yahoo_away";
    icon.xpm = yahoo_away;
    eIcon.process();

    icon.name = "Yahoo_na";
    icon.xpm = yahoo_na;
    eIcon.process();

    icon.name = "Yahoo_dnd";
    icon.xpm = yahoo_dnd;
    eIcon.process();

    icon.name = "Yahoo_ffc";
    icon.xpm = yahoo_ffc;
    eIcon.process();

    icon.name = "reg";
    icon.xpm = reg;
    eIcon.process();

    Event eMenuSearch(EventMenuCreate, (void*)MenuSearchResult);
    eMenuSearch.process();
    Event eMenuGroups(EventMenuCreate, (void*)MenuGroups);
    eMenuGroups.process();
    Event eMenuClients(EventMenuCreate, (void*)MenuClients);
    eMenuClients.process();
    Event eToolbar(EventToolbarCreate, (void*)BarBrowser);
    eToolbar.process();

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

    cmd->id			 = CmdBrowser;
    cmd->text		 = I18N_NOOP("Jabber browser");
    cmd->icon		 = "Jabber_online";
    cmd->menu_grp	 = 0x20F0;
    cmd->menu_id	 = MenuMain;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdBrowser;
    cmd->text		 = I18N_NOOP("Jabber browser");
    cmd->icon		 = "Jabber_online";
    cmd->menu_grp	 = 0x20F0;
    cmd->menu_id	 = MenuContact;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdBack;
    cmd->text		 = I18N_NOOP("&Back");
    cmd->icon		 = "1leftarrow";
    cmd->bar_grp	 = 0x1000;
    cmd->bar_id		 = BarBrowser;
    cmd->menu_id	 = 0;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdForward;
    cmd->text		 = I18N_NOOP("&Next");
    cmd->icon		 = "1rightarrow";
    cmd->bar_grp	 = 0x1001;
    eCmd.process();

    cmd->id			 = CmdUrl;
    cmd->text		 = I18N_NOOP("JID");
    cmd->icon		 = "run";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    eCmd.process();

    cmd->id			 = CmdNode;
    cmd->text		 = I18N_NOOP("Node");
    cmd->bar_grp	 = 0x2001;
    cmd->flags		 = BTN_COMBO | BTN_NO_BUTTON;
    eCmd.process();

    cmd->id			 = CmdSearch;
    cmd->text		 = I18N_NOOP("&Search");
    cmd->icon		 = "find";
    cmd->bar_grp	 = 0x3000;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdRegister;
    cmd->text		 = I18N_NOOP("&Register");
    cmd->icon		 = "reg";
    cmd->bar_grp	 = 0x3001;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdInfo;
    cmd->text		 = I18N_NOOP("Info");
    cmd->icon		 = "info";
    cmd->bar_grp	 = 0x3010;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdConfigure;
    cmd->text		 = I18N_NOOP("Configure");
    cmd->icon		 = "configure";
    cmd->bar_grp	 = 0x3020;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdBrowser;
    cmd->text		 = "_";
    cmd->icon		 = NULL;
    cmd->menu_grp	 = 0x1000;
    cmd->menu_id	 = MenuClients;
    cmd->bar_grp	 = 0;
    cmd->bar_id		 = 0;
    cmd->popup_id	 = 0;
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

    Event eMenuClients(EventMenuRemove, (void*)MenuClients);
    eMenuClients.process();

    Event eCmdBrowser(EventCommandRemove, (void*)CmdBrowser);
    eMenuGroups.process();

    Event eToolbar(EventToolbarRemove, (void*)BarBrowser);
    eToolbar.process();

    delete m_protocol;
    getContacts()->removePacketType(JabberPacket);
}

#ifdef WIN32
#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
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


