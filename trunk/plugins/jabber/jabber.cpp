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

#include "jabberclient.h"
#include "jabber.h"
#include "core.h"

#include "xpm/jabber.xpm"
#include "xpm/register.xpm"
#include "xpm/connect.xpm"

Plugin *createJabberPlugin(unsigned base, bool, Buffer *cfg)
{
    Plugin *plugin = new JabberPlugin(base, cfg);
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

Client *JabberProtocol::createClient(Buffer *cfg)
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
        PROTOCOL_INFO | PROTOCOL_AR | PROTOCOL_INVISIBLE | PROTOCOL_SEARCH | PROTOCOL_AR_OFFLINE,
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

#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
void qInitJpeg();
#endif

DataDef jabberData[] =
    {
        { "BrowserBar", DATA_LONG, 7, 0 },
        { "BrowserHistory", DATA_UTF, 1, 0 },
        { "AllLevels", DATA_BOOL, 1, 0 },
        { "BrowseType", DATA_ULONG, 1, DATA(BROWSE_DISCO | BROWSE_BROWSE | BROWSE_AGENTS) },
        { NULL, 0, 0, 0 }
    };

JabberPlugin *JabberPlugin::plugin = NULL;

JabberPlugin::JabberPlugin(unsigned base, Buffer *cfg)
        : Plugin(base)
{
#if defined(WIN32) && (COMPAT_QT_VERSION < 0x030000)
    qInitJpeg();
#endif
    plugin = this;
    load_data(jabberData, &data, cfg);
    JabberPacket = registerType();
    getContacts()->addPacketType(JabberPacket, jabber_descr.text, true);

    IconDef icon;
    icon.name = "Jabber";
    icon.xpm  = jabber;
    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name = "reg";
    icon.xpm  = reg;
    eIcon.process();

    icon.name = "connect";
    icon.xpm  = connect_xpm;
    icon.system = "connect_established";
    eIcon.process();

    Event eMenuSearch(EventMenuCreate, (void*)MenuSearchResult);
    eMenuSearch.process();
    Event eMenuGroups(EventMenuCreate, (void*)MenuJabberGroups);
    eMenuGroups.process();
    Event eMenuBrowser(EventMenuCreate, (void*)MenuBrowser);
    eMenuBrowser.process();
    Event eToolbar(EventToolbarCreate, (void*)BarBrowser);
    eToolbar.process();

    Command	cmd;
    cmd->id			 = CmdJabberMessage;
    cmd->text		 = I18N_NOOP("&Message");
    cmd->icon		 = "message";
    cmd->menu_id	 = MenuSearchResult;
    cmd->menu_grp	 = 0x1000;
    cmd->flags		 = COMMAND_DEFAULT;
    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id			 = CmdBrowseInfo;
    cmd->text		 = I18N_NOOP("User &info");
    cmd->icon		 = "info";
    cmd->menu_grp	 = 0x1001;
    eCmd.process();

    cmd->id			 = CmdGroups;
    cmd->text		 = I18N_NOOP("&Add to group");
    cmd->icon		 = NULL;
    cmd->menu_grp	 = 0x1002;
    cmd->popup_id	 = MenuJabberGroups;
    eCmd.process();

    cmd->id			 = CmdGroups;
    cmd->text		 = "_";
    cmd->menu_id	 = MenuJabberGroups;
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

    cmd->id			 = CmdBrowseSearch;
    cmd->text		 = I18N_NOOP("&Search");
    cmd->icon		 = "find";
    cmd->bar_grp	 = 0x3000;
    cmd->menu_id	 = MenuSearchOptions;
    cmd->menu_grp	 = 0x2000;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdRegister;
    cmd->text		 = I18N_NOOP("&Register");
    cmd->icon		 = "reg";
    cmd->bar_grp	 = 0x3001;
    cmd->menu_grp	 = 0x2001;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdBrowseInfo;
    cmd->text		 = I18N_NOOP("Info");
    cmd->icon		 = "info";
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->bar_grp	 = 0x3010;
    eCmd.process();

    cmd->id			 = CmdBrowseConfigure;
    cmd->text		 = I18N_NOOP("Configure");
    cmd->icon		 = "configure";
    cmd->bar_grp	 = 0x3020;
    cmd->menu_id	 = MenuSearchOptions;
    cmd->menu_grp	 = 0x2002;
    eCmd.process();

    cmd->id			 = CmdBrowseMode;
    cmd->text		 = I18N_NOOP("Browser mode");
    cmd->icon		 = "configure";
    cmd->bar_grp	 = 0x5000;
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->popup_id	 = MenuBrowser;
    eCmd.process();

    cmd->id			 = CmdOneLevel;
    cmd->text		 = I18N_NOOP("Load one level");
    cmd->icon		 = NULL;
    cmd->bar_id		 = 0;
    cmd->bar_grp	 = 0;
    cmd->menu_id	 = MenuBrowser;
    cmd->menu_grp	 = 0x1000;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_CHECK_STATE;
    eCmd.process();

    cmd->id			 = CmdAllLevels;
    cmd->text		 = I18N_NOOP("Load all levels");
    cmd->menu_grp	 = 0x1001;
    eCmd.process();

    cmd->id			 = CmdModeDisco;
    cmd->text		 = "Discovery";
    cmd->menu_grp	 = 0x2000;
    eCmd.process();

    cmd->id			 = CmdModeBrowse;
    cmd->text		 = "Browse";
    cmd->menu_grp	 = 0x2001;
    eCmd.process();

    cmd->id			 = CmdModeAgents;
    cmd->text		 = "Agents";
    cmd->menu_grp	 = 0x2002;
    eCmd.process();

    m_protocol = new JabberProtocol(this);
    registerMessages();
}

JabberPlugin::~JabberPlugin()
{
    unregisterMessages();

    Event eMenuSearch(EventMenuRemove, (void*)MenuSearchResult);
    eMenuSearch.process();

    Event eMenuGroups(EventMenuRemove, (void*)MenuJabberGroups);
    eMenuGroups.process();

    Event eMenuBrowser(EventMenuRemove, (void*)MenuBrowser);
    eMenuBrowser.process();

    Event eToolbar(EventToolbarRemove, (void*)BarBrowser);
    eToolbar.process();

    delete m_protocol;
    getContacts()->removePacketType(JabberPacket);

    free_data(jabberData, &data);
}

string JabberPlugin::getConfig()
{
    return save_data(jabberData, &data);
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


