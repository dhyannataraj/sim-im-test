/***************************************************************************
                          replace.cpp  -  description
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

#include "replace.h"
#include "replacecfg.h"

Plugin *createReplacePlugin(unsigned base, bool, const char *cfg)
{
    Plugin *plugin = new ReplacePlugin(base, cfg);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Replace text"),
        I18N_NOOP("Plugin provides text replacing in message edit window"),
        VERSION,
        createReplacePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef replaceData[] =
    {
		{ "Keys", DATA_UTFLIST, 1, 0 },
        { "Key", DATA_UTFLIST, 1, 0 },
        { "Value", DATA_UTFLIST, 1, 0 },
        { NULL, 0, 0, 0 }
    };

ReplacePlugin::ReplacePlugin(unsigned base, const char *cfg)
        : Plugin(base)
{
	load_data(replaceData, &data, cfg);
}

ReplacePlugin::~ReplacePlugin()
{
	free_data(replaceData, &data);
}

string ReplacePlugin::getConfig()
{
	return save_data(replaceData, &data);
}

QWidget *ReplacePlugin::createConfigWindow(QWidget *parent)
{
	return new ReplaceCfg(parent, this);
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

#ifndef WIN32
#include "replace.moc"
#endif



