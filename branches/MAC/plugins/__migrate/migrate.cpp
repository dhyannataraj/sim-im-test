/***************************************************************************
                          migrate.cpp  -  description
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

#include "migrate.h"
#include "migratedlg.h"

#include <qdir.h>

Plugin *createMigratePlugin(unsigned base, bool, const char*)
{
    MigratePlugin *plugin = new MigratePlugin(base);
    if (!plugin->init()){
        delete plugin;
        return NULL;
    }
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Migrate"),
        I18N_NOOP("Plugin provides convert configuration and history from SIM 0.8"),
        VERSION,
        createMigratePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

MigratePlugin::MigratePlugin(unsigned base)
        : Plugin(base)
{
}

MigratePlugin::~MigratePlugin()
{
}

bool MigratePlugin::init()
{
    string path = user_file("");
    QString dir = QFile::decodeName(path.c_str());
    QDir d(dir);
    if (!d.exists())
        return false;
    QStringList cnvDirs;
    QStringList dirs = d.entryList(QDir::Dirs);
    QStringList::Iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it){
        if ((*it)[0] == '.')
            continue;
        QString p = dir + (*it);
#ifdef WIN32
        p += "\\";
#else
        p += "/";
#endif
        QFile icqConf(p + "icq.conf");
        QFile clientsConf(p + "clients.conf");
        if (icqConf.exists() && !clientsConf.exists()){
            cnvDirs.append(*it);
        }
    }
    if (cnvDirs.count() == 0)
        return false;
    MigrateDialog dlg(dir, cnvDirs);
    dlg.exec();
    return true;
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


