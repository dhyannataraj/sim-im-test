/***************************************************************************
                          remote.cpp  -  description
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

#include "remote.h"
#include "remotecfg.h"
#include "simapi.h"

Plugin *createRemotePlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new RemotePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Remote control"),
        I18N_NOOP("Plugin provides remote control"),
        VERSION,
        createRemotePlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef remoteData[] =
    {
#ifdef WIN32
        { "Path", DATA_STRING, 1, (unsigned)"tcp:3000" },
#else
		{ "Path", DATA_STRING, 1, (unsigned)"/tcp/sim.%user%" },
#endif
        { NULL, 0, 0, 0 }
    };

RemotePlugin::RemotePlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(remoteData, &data, config);
}

RemotePlugin::~RemotePlugin()
{
    free_data(remoteData, &data);
}

string RemotePlugin::getConfig()
{
    return save_data(remoteData, &data);
}

QWidget *RemotePlugin::createConfigWindow(QWidget *parent)
{
    return new RemoteConfig(parent, this);
}

void *RemotePlugin::processEvent(Event*)
{
    return NULL;
}

static char TCP[] = "tcp:";


void RemotePlugin::bind()
{
	const char *path = getPath();
	if ((strlen(path) > strlen(TCP)) && !memcmp(path, TCP, strlen(TCP))){
		unsigned short port = (unsigned short)atol(path + strlen(TCP));
		ServerSocketNotify::bind(port, port, NULL);
#ifndef WIN32
	}else{
		ServerSocketNotify::bind(path);
#endif
	}
}

bool RemotePlugin::accept(Socket *s, unsigned long)
{
	log(L_DEBUG, "Accept remote control");
	delete s;
	return false;
}

void RemotePlugin::bind_ready(unsigned short)
{
}

bool RemotePlugin::error(const char *err)
{
	log(L_DEBUG, "Remote: %s", err);
	return false;
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

