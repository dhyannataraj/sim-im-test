/***************************************************************************
                          yahoo.cpp  -  description
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

#include "yahoo.h"
#include "yahooclient.h"
#include "core.h"

Plugin *createYahooPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new YahooPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        NULL,
        NULL,
        VERSION,
        createYahooPlugin,
        PLUGIN_PROTOCOL
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

unsigned YahooPlugin::YahooPacket = 0;
CorePlugin *YahooPlugin::core = NULL;

YahooPlugin::YahooPlugin(unsigned base)
        : Plugin(base)
{
    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
    YahooPacket = registerType();
    getContacts()->addPacketType(YahooPacket, "Yahoo!");
    m_protocol = new YahooProtocol(this);
}

YahooPlugin::~YahooPlugin()
{
    delete m_protocol;
    getContacts()->removePacketType(YahooPacket);
}

YahooProtocol::YahooProtocol(Plugin *plugin)
        : Protocol(plugin)
{
}

YahooProtocol::~YahooProtocol()
{
}

Client *YahooProtocol::createClient(const char *cfg)
{
    return new YahooClient(this, cfg);
}

static CommandDef yahoo_descr =
    {
        0,
        I18N_NOOP("Yahoo!"),
        "Yahoo!_online",
        "Yahoo!_invisible",
        I18N_NOOP("http://edit.yahoo.com/config/eval_forgot_pw?.src=pg&.done=http://messenger.yahoo.com/&.redir_from=MESSENGER"),
        0,
        0,
        0,
        0,
        0,
        PROTOCOL_SEARCH_ONLINE | PROTOCOL_INVISIBLE,
        NULL,
        NULL
    };

const CommandDef *YahooProtocol::description()
{
    return &yahoo_descr;
}

static CommandDef yahoo_status_list[] =
    {
        {
            STATUS_ONLINE,
            I18N_NOOP("Online"),
            "Yahoo!_online",
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
            "Yahoo!_away",
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
            "Yahoo!_na",
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
            "Yahoo!_dnd",
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
            "Yahoo!_offline",
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

const CommandDef *YahooProtocol::statusList()
{
    return yahoo_status_list;
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



