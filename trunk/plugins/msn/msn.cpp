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

#include "xpm/lunch.xpm"
#include "xpm/onback.xpm"
#include "xpm/onphone.xpm"

Plugin *createMSNPlugin(unsigned base, bool, Buffer*)
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

Client *MSNProtocol::createClient(Buffer *cfg)
{
    return new MSNClient(this, cfg);
}

static CommandDef msn_descr =
    {
        0,
        I18N_NOOP("MSN"),
        "MSN_online",
        "MSN_invisible",
        I18N_NOOP("http://help.microsoft.com/!data/en_us/data/messengerv47_xp.its51/$content$/xptroublesignin.htm"),
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_INVISIBLE,
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
            "MSN_onback",
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
            "MSN_onphone",
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
            "MSN_lunch",
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
    EventAddOk   = registerType();
    EventAddFail = registerType();
    MSNInitMail  = registerType();
    MSNNewMail   = registerType();

    getContacts()->addPacketType(MSNPacket, msn_descr.text, true);

    IconDef icon;
    icon.name  = "lunch";
    icon.xpm   = lunch;
    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    icon.name  = "onback";
    icon.xpm   = onback;
    eIcon.process();

    icon.name  = "onphone";
    icon.xpm   = onphone;
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


