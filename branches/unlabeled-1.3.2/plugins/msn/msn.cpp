/***************************************************************************
                          msn.cpp  -  description
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

#include "msn.h"
#include "msnclient.h"
#include "simapi.h"

#include "xpm/msn_online.xpm"
#include "xpm/msn_offline.xpm"
#include "xpm/msn_away.xpm"
#include "xpm/msn_na.xpm"
#include "xpm/msn_dnd.xpm"
#include "xpm/msn_invisible.xpm"

Plugin *createMSNPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new MSNPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createMSNPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

MSNProtocol::MSNProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

MSNProtocol::~MSNProtocol()
{
}

Client *MSNProtocol::createClient(const char *cfg)
{
    return new MSNClient(this, cfg);
}

static CommandDef msn_descr =
    {
        0,
        I18N_NOOP("MSN"),
        "MSN_online",
        "MSN_invisible",
        I18N_NOOP("http://help.microsoft.com/!data/ru_ru/data/messengerv47_xp.its51/$content$/xptroublesignin.htm"),
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_SEARCH_ONLINE | PROTOCOL_INVISIBLE,
        NULL,
        NULL
    };

const CommandDef *MSNProtocol::description()
{
    return &msn_descr;
}

static CommandDef msn_status_list[] =
    {
        {
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "MSN_online",
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
            "MSN_away",
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
            "MSN_na",
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
            I18N_NOOP("Busy"),
            "MSN_dnd",
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
            STATUS_BRB,
            I18N_NOOP("Be right back"),
            "MSN_away",
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
            STATUS_PHONE,
            I18N_NOOP("On the phone"),
            "MSN_away",
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
            STATUS_LUNCH,
            I18N_NOOP("On the lunch"),
            "MSN_away",
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
            "MSN_offline",
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

const CommandDef *MSNProtocol::statusList()
{
    return msn_status_list;
}

MSNPlugin::MSNPlugin(unsigned base)
        : Plugin(base)
{
    MSNPacket = registerType();
    getContacts()->addPacketType(MSNPacket, msn_descr.text, PACKET_TEXT);

    IconDef icon;
    icon.name = "MSN_online";
    icon.xpm = msn_online;
    icon.isSystem = false;

    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name = "MSN_offline";
    icon.xpm = msn_offline;
    icon.isSystem = false;
    eIcon.process();

    icon.name = "MSN_away";
    icon.xpm = msn_away;
    icon.isSystem = false;
    eIcon.process();

    icon.name = "MSN_na";
    icon.xpm = msn_na;
    icon.isSystem = false;
    eIcon.process();

    icon.name = "MSN_dnd";
    icon.xpm = msn_dnd;
    icon.isSystem = false;
    eIcon.process();

    icon.name = "MSN_invisible";
    icon.xpm = msn_invisible;
    icon.isSystem = false;
    eIcon.process();

    m_protocol = new MSNProtocol(this);
}

MSNPlugin::~MSNPlugin()
{
    getContacts()->removePacketType(MSNPacket);
    delete m_protocol;
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


