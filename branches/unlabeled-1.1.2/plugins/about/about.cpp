/***************************************************************************
                          about.cpp  -  description
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

#include "about.h"
#include "aboutdata.h"
#include "simapi.h"

#ifdef USE_KDE
#include <kaboutapplication.h>
#include <kaboutkde.h>
#else
#include "aboutdlg.h"
#endif
#include <qtimer.h>

Plugin *createAboutPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new AboutPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("About"),
        I18N_NOOP("Plugin provides about information"),
        VERSION,
        createAboutPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

AboutPlugin::AboutPlugin(unsigned base)
        : Plugin(base)
{
    CmdBugReport = registerType();
    CmdAbout = registerType();
#ifdef USE_KDE
    CmdAboutKDE = registerType();
#endif

    Command cmd;
    cmd->id			= CmdBugReport;
    cmd->text		= I18N_NOOP("&Bug report");
    cmd->bar_id		= ToolBarMain;
    cmd->menu_id	= MenuMain;
    cmd->menu_grp	= 0xF000;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    about = NULL;
    cmd->id			= CmdAbout;
    cmd->text		= I18N_NOOP("&About SIM");
    cmd->icon		= "licq";
    eCmd.process();

#ifdef USE_KDE
    about_kde = NULL;
    cmd->id			= CmdAboutKDE;
    cmd->text		= I18N_NOOP("About &KDE");
    cmd->icon		= "about_kde";
    eCmd.process();
#endif
}

AboutPlugin::~AboutPlugin()
{
    if (about)
        delete about;
#ifdef USE_KDE
    if (about_kde)
        delete about_kde;
#endif
    Event eBug(EventCommandRemove, (void*)CmdBugReport);
    eBug.process();
    Event eAbout(EventCommandRemove, (void*)CmdAbout);
    eAbout.process();
}

void *AboutPlugin::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdBugReport){
            Event eURL(EventGoURL, (void*)"http://sourceforge.net/tracker/?group_id=56866");
            eURL.process();
        }
        if (cmd->id == CmdAbout){
            if (about == NULL)
            {
                KAboutData *about_data = getAboutData();
                about_data->setTranslator(
                    I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"),
                    I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

                about = new KAboutApplication( about_data, NULL, "about", false );
                connect(about, SIGNAL(finished()), this, SLOT(aboutDestroyed()));
            }
            raiseWindow(about);
        }
#ifdef USE_KDE
        if (cmd->id == CmdAboutKDE){
            if (about_kde == NULL)
            {
                about_kde = new KAboutKDE( NULL, "aboutkde", false);
                connect(about_kde, SIGNAL(finished()), this, SLOT(aboutDestroyed()));
            }
            raiseWindow(about_kde);
        }
#endif
    }
    return NULL;
}

void AboutPlugin::aboutDestroyed()
{
    QTimer::singleShot( 0, this, SLOT(realDestroy()) );
}

void AboutPlugin::realDestroy()
{
    if ((about != NULL) && about->isVisible() == false )
    {
        delete about;
        about = NULL;
    }
#ifdef USE_KDE
    if ((about_kde != NULL) && about_kde->isVisible() == false )
    {
        delete about_kde;
        about_kde = NULL;
    }
#endif
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

#ifndef WIN32
#include "about.moc"
#endif

