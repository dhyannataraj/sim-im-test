/***************************************************************************
                          action.cpp  -  description
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

#include "action.h"
#include "actioncfg.h"
#include "core.h"

Plugin *createActionPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new ActionPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Action"),
        I18N_NOOP("Plugin execute external programs on event or from contact menu"),
        VERSION,
        createActionPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*

typedef struct ActionUserData
{
	char	*OnLine;
	char	*Status;
	char	*Message;
	char	*Menu;
} ActionUserData;

*/

static DataDef actionUserData[] =
    {
        { "OnLine", DATA_UTF, 1, 0 },
        { "Status", DATA_UTF, 1, 0 },
        { "Message", DATA_UTFLIST, 1, 0 },
        { "Menu", DATA_UTFLIST, 1, 0 },
        { NULL, 0, 0, 0 }
    };

static ActionPlugin *plugin = NULL;

static QWidget *getActionSetup(QWidget *parent, void *data)
{
    return new ActionConfig(parent, (ActionUserData*)data, plugin);
}

ActionPlugin::ActionPlugin(unsigned base)
        : Plugin(base)
{
    plugin = this;

    action_data_id = getContacts()->registerUserData(info.title, actionUserData);

    Command cmd;
    cmd->id		 = action_data_id + 1;
    cmd->text	 = I18N_NOOP("&Action");
    cmd->icon	 = "run";
    cmd->icon_on  = NULL;
    cmd->param	 = (void*)getActionSetup;
    Event e(EventAddPreferences, cmd);
    e.process();

    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
}

ActionPlugin::~ActionPlugin()
{
    Event e(EventRemovePreferences, (void*)action_data_id);
    e.process();
    getContacts()->unregisterUserData(action_data_id);
}

QWidget *ActionPlugin::createConfigWindow(QWidget *parent)
{
    ActionUserData *data = (ActionUserData*)(getContacts()->getUserData(action_data_id));
    return new ActionConfig(parent, data, this);
}

void *ActionPlugin::processEvent(Event*)
{
    return NULL;
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


