/***************************************************************************
                          icq.cpp  -  description
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

#include "icq.h"
#include "icqconfig.h"
#include "simapi.h"
#include "core.h"

#include "xpm/online.xpm"
#include "xpm/offline.xpm"
#include "xpm/away.xpm"
#include "xpm/na.xpm"
#include "xpm/occupied.xpm"
#include "xpm/dnd.xpm"
#include "xpm/ffc.xpm"
#include "xpm/invisible.xpm"
#include "xpm/birthday.xpm"
#include "xpm/more.xpm"
#include "xpm/interest.xpm"
#include "xpm/past.xpm"
#include "xpm/useronline.xpm"
#include "xpm/useroffline.xpm"
#include "xpm/userunknown.xpm"
#include "xpm/encoding.xpm"
#include "xpm/url.xpm"
#include "xpm/contacts.xpm"
#include "xpm/web.xpm"
#include "xpm/mailpager.xpm"
#include "xpm/icqphone.xpm"
#include "xpm/icqphonebusy.xpm"
#include "xpm/sharedfiles.xpm"
#include "xpm/pict.xpm"
#include "xpm/aim_offline.xpm"
#include "xpm/aim_online.xpm"
#include "xpm/aim_away.xpm"

Plugin *createICQPlugin(unsigned base, bool, const char *cfg)
{
    Plugin *plugin = new ICQPlugin(base, cfg);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createICQPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

ICQProtocol::ICQProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

ICQProtocol::~ICQProtocol()
{
}

Client *ICQProtocol::createClient(const char *cfg)
{
    return new ICQClient(this, cfg);
}

static CommandDef icq_descr =
    {
        0,
        I18N_NOOP("ICQ"),
        "ICQ_online",
        "ICQ_invisible",
        NULL,
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INFO | PROTOCOL_SEARCH_ONLINE | PROTOCOL_INVISIBLE | PROTOCOL_AR_USER | PROTOCOL_FOLLOWME,
        NULL,
        NULL
    };

const CommandDef *ICQProtocol::description()
{
    return &icq_descr;
}

static CommandDef icq_status_list[] =
    {
        {
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "ICQ_online",
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
            "ICQ_away",
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
            "ICQ_na",
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
            "ICQ_dnd",
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
            STATUS_OCCUPIED,
            I18N_NOOP("Occupied"),
            "ICQ_occupied",
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
            "ICQ_ffc",
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
            "ICQ_offline",
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

const CommandDef *ICQProtocol::statusList()
{
    return icq_status_list;
}

/*
typedef struct IconsData
{
    void *IconDLLs;
} IconsData;
*/
static DataDef icqData[] =
    {
        { "ShowAllEncodings", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Protocol *ICQPlugin::m_protocol = NULL;

#ifdef WIN32
void qInitJpeg();
#endif

ICQPlugin::ICQPlugin(unsigned base, const char *cfg)
        : Plugin(base)
{
#ifdef WIN32
    qInitJpeg();
#endif

    load_data(icqData, &data, cfg);

    ICQPacket = registerType();
    getContacts()->addPacketType(ICQPacket, icq_descr.text);
    ICQDirectPacket = registerType();
    getContacts()->addPacketType(ICQDirectPacket, "ICQ.Direct");

    IconDef icon;
    icon.name = "ICQ_online";
    icon.xpm = online;
    icon.isSystem = false;

    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name = "ICQ_offline";
    icon.xpm = offline;
    eIcon.process();

    icon.name = "ICQ_away";
    icon.xpm = away;
    eIcon.process();

    icon.name = "ICQ_na";
    icon.xpm = na;
    eIcon.process();

    icon.name = "ICQ_occupied";
    icon.xpm = occupied;
    eIcon.process();

    icon.name = "ICQ_dnd";
    icon.xpm = dnd;
    eIcon.process();

    icon.name = "ICQ_ffc";
    icon.xpm = ffc;
    eIcon.process();

    icon.name = "ICQ_invisible";
    icon.xpm = invisible;
    eIcon.process();

    icon.name = "birthday";
    icon.xpm = birthday;
    eIcon.process();

    icon.name = "more";
    icon.xpm = more;
    eIcon.process();

    icon.name = "interest";
    icon.xpm = interest;
    eIcon.process();

    icon.name = "past";
    icon.xpm = past;
    eIcon.process();

    icon.name = "useronline";
    icon.xpm = useronline;
    eIcon.process();

    icon.name = "useroffline";
    icon.xpm = useroffline;
    eIcon.process();

    icon.name = "userunknown";
    icon.xpm = userunknown;
    eIcon.process();

    icon.name = "encoding";
    icon.xpm = encoding;
    eIcon.process();

    icon.name = "url";
    icon.xpm = url;
    eIcon.process();

    icon.name = "contacts";
    icon.xpm = contacts;
    eIcon.process();

    icon.name = "web";
    icon.xpm = web;
    eIcon.process();

    icon.name = "mailpager";
    icon.xpm = mailpager;
    eIcon.process();

    icon.name = "icqphone";
    icon.xpm = icqphone;
    eIcon.process();

    icon.name = "icqphonebusy";
    icon.xpm = icqphonebusy;
    eIcon.process();

    icon.name = "sharedfiles";
    icon.xpm = sharedfiles;
    eIcon.process();

    icon.name = "pict";
    icon.xpm = pict;
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

    EventSearch = EventUser + registerType();
    EventSearchDone = EventUser + registerType();
    EventAutoReplyFail = EventUser + registerType();
    EventRandomChat = EventUser + registerType();
    EventRandomChatInfo = EventUser + registerType();
    MenuEncoding = registerType();
    MenuSearchResult = registerType();
    MenuGroups = registerType();

    CmdVisibleList = registerType();
    CmdInvisibleList = registerType();
    CmdChangeEncoding = registerType();
    CmdAllEncodings = registerType();
    CmdSendMessage = registerType();
    CmdGroups = registerType();

    m_protocol = new ICQProtocol(this);

    Event eMenuEncoding(EventMenuCreate, (void*)MenuEncoding);
    eMenuEncoding.process();
    Event eMenuSearch(EventMenuCreate, (void*)MenuSearchResult);
    eMenuSearch.process();
    Event eMenuGroups(EventMenuCreate, (void*)MenuGroups);
    eMenuGroups.process();

    Command cmd;
    cmd->id          = CmdVisibleList;
    cmd->text        = I18N_NOOP("Visible list");
    cmd->menu_id     = MenuContactGroup;
    cmd->menu_grp    = 0x8010;
    cmd->flags		 = COMMAND_CHECK_STATE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id			 = CmdInvisibleList;
    cmd->text		 = I18N_NOOP("Invisible list");
    cmd->menu_grp	 = 0x8011;
    eCmd.process();

    cmd->id			 = CmdChangeEncoding;
    cmd->text		 = "_";
    cmd->menu_id	 = MenuEncoding;
    cmd->menu_grp	 = 0x1000;
    eCmd.process();

    cmd->id			 = CmdAllEncodings;
    cmd->text		 = I18N_NOOP("&Show all encodings");
    cmd->menu_id	 = MenuEncoding;
    cmd->menu_grp	 = 0x8000;
    eCmd.process();

    cmd->id			 = CmdChangeEncoding;
    cmd->text		 = I18N_NOOP("Change &encoding");
    cmd->icon		 = "encoding";
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->bar_id		 = ToolBarContainer;
    cmd->bar_grp	 = 0x8080;
    cmd->popup_id	 = MenuEncoding;
    eCmd.process();

    cmd->id			 = CmdChangeEncoding;
    cmd->text		 = I18N_NOOP("Change &encoding");
    cmd->icon		 = "encoding";
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->bar_id		 = BarHistory;
    cmd->bar_grp	 = 0x8080;
    cmd->popup_id	 = MenuEncoding;
    eCmd.process();

    cmd->id			 = CmdSendMessage;
    cmd->text		 = I18N_NOOP("&Message");
    cmd->icon		 = "message";
    cmd->menu_id	 = MenuSearchResult;
    cmd->menu_grp	 = 0x1000;
    cmd->bar_id		 = 0;
    cmd->popup_id	 = 0;
    cmd->flags		 = COMMAND_DEFAULT;
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

    registerMessages();
}

ICQPlugin::~ICQPlugin()
{
    unregisterMessages();

    delete m_protocol;
    getContacts()->removePacketType(ICQPacket);
    getContacts()->removePacketType(ICQDirectPacket);

    Event eVisible(EventCommandRemove, (void*)CmdVisibleList);
    eVisible.process();

    Event eInvisible(EventCommandRemove, (void*)CmdInvisibleList);
    eInvisible.process();

    Event eEncoding(EventCommandRemove, (void*)CmdChangeEncoding);
    eEncoding.process();

    Event eAllEncodings(EventCommandRemove, (void*)CmdAllEncodings);
    eAllEncodings.process();

    Event eMenuEncoding(EventMenuRemove, (void*)MenuEncoding);
    eMenuEncoding.process();

    Event eMenuSearch(EventMenuRemove, (void*)MenuSearchResult);
    eMenuSearch.process();

    Event eMenuGroups(EventMenuRemove, (void*)MenuGroups);
    eMenuGroups.process();
}

string ICQPlugin::getConfig()
{
    return save_data(icqData, &data);
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


