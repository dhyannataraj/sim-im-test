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
#include "iconcfg.h"
#include "icons.h"

Plugin *createIconsPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new IconsPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Icons"),
        I18N_NOOP("Plugin provides JISP icons themes and emoticons"),
        VERSION,
        createIconsPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef iconsData[] =
    {
        { "Icons", DATA_STRLIST, 1, 0 },
        { "NIcons", DATA_ULONG, 1, 0 },
        { "Default", DATA_BOOL, 1, DATA(1) },
        { NULL, 0, 0, 0 }
    };

IconsPlugin::IconsPlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    load_data(iconsData, &data, config);
    setIcons(false);
}

IconsPlugin::~IconsPlugin()
{
    free_data(iconsData, &data);
}

void IconsPlugin::setIcons(bool bForce)
{
    if (!bForce && getDefault())
        return;
    list<IconSet*> &sets = getIcons()->m_customSets;
    for (list<IconSet*>::iterator it = sets.begin(); it != sets.end(); ){
        IconSet *set = *it;
        it++;
        delete set;
    }
    sets.clear();
    if (getDefault()){
        getIcons()->addIconSet("icons/smile.jisp", false);
    }else{
        for (unsigned i = 1; i <= getNIcons(); i++)
            getIcons()->addIconSet(getIcon(i), false);
    }
    Event e(EventIconChanged, NULL);
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


