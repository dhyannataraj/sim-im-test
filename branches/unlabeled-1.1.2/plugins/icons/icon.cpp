/***************************************************************************
                          icon.cpp  -  description
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

#include "icon.h"
#include "icondll.h"
#include "iconcfg.h"
#include "simapi.h"

Plugin *createIconsPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new IconsPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Icons"),
        I18N_NOOP("Plugin provides Miranda icons themes"),
        VERSION,
        createIconsPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*
typedef struct IconsData
{
    void *IconDLLs;
} IconsData;
*/
static DataDef iconsData[] =
    {
        { "IconDLLs", DATA_STRLIST, 1, 0 },
        { NULL, 0, 0, 0 }
    };

IconsPlugin::IconsPlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(iconsData, &data, config);
    setIcons();
}

IconsPlugin::~IconsPlugin()
{
    for (list<IconDLL*>::iterator it = dlls.begin(); it != dlls.end(); ++it){
        delete *it;
    }
    dlls.clear();
    Event e(EventIconChanged);
    e.process();
    free_data(iconsData, &data);
}

void IconsPlugin::setIcons()
{
    for (list<IconDLL*>::iterator it = dlls.begin(); it != dlls.end(); ++it){
        delete *it;
    }
    dlls.clear();
    for (unsigned n = 1; ; n++){
        const char *cfg = getIconDLLs(n);
        if ((cfg == NULL) || (*cfg == 0))
            break;
        string v = cfg;
        string name = getToken(v, ',');
        if (v.length() == 0)
            continue;
        IconDLL *dll = new IconDLL(name.c_str());
        if (!dll->load(v.c_str()))
            continue;
        dlls.push_back(dll);
    }
    Event e(EventIconChanged);
    e.process();
}

string IconsPlugin::getConfig()
{
    return save_data(iconsData, &data);
}

QWidget *IconsPlugin::createConfigWindow(QWidget *parent)
{
    return new IconCfg(parent, this);
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


