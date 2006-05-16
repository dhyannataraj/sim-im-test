/***************************************************************************
                          homedir.cpp  -  description
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

#include "homedir.h"
#include "simapi.h"

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>

#include <qlibrary.h>
#include <qsettings.h>

#include "homedircfg.h"

static BOOL (WINAPI *_SHGetSpecialFolderPathA)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
static BOOL (WINAPI *_SHGetSpecialFolderPathW)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;

#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

#include <qdir.h>

using namespace SIM;

Plugin *createHomeDirPlugin(unsigned base, bool, ConfigBuffer*)
{
    Plugin *plugin = new HomeDirPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
#ifdef WIN32
        I18N_NOOP("Home directory"),
        I18N_NOOP("Plugin provides select directory for store config files"),
#else
NULL,
        NULL,
#endif
        VERSION,
        createHomeDirPlugin,
        PLUGIN_NO_CONFIG_PATH | PLUGIN_NODISABLE
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

#ifdef WIN32

static char key_name[] = "Software\\SIM";
static char path_value[] = "Path";

#endif

HomeDirPlugin::HomeDirPlugin(unsigned base)
        : Plugin(base)
{
#ifdef WIN32
    m_bSave    = true;
	QSettings setting( QSettings::Native );
	setting.setPath( "/SIM", "", QSettings::User );
	m_homeDir = setting.readEntry( "Path" );
    m_bDefault = m_homeDir.isNull();
#endif
	QString d;
	std::string value;
    CmdParam p = { "-b:", I18N_NOOP("Set home directory"), &value };
    Event e(EventArg, &p);
    if (e.process()){
        d = QFile::decodeName(value.c_str());
#ifdef WIN32
        m_bSave   = false;
#endif
	} else {
		d = m_homeDir;
	}
	QDir dir( d );
    if ( !dir.exists() ) {
        m_homeDir  = defaultPath();
#ifdef WIN32
        m_bDefault = true;
        m_bSave   = false;
#endif
    }
}

QString HomeDirPlugin::defaultPath()
{
    QString s;
#ifndef WIN32
    struct passwd *pwd = getpwuid(getuid());
    if (pwd){
        s = pwd->pw_dir;
    }else{
        log(L_ERROR, "Can't get pwd");
    }
    if (s.right(1) != "/")
        s += '/';
#ifdef USE_KDE
    char *kdehome = getenv("KDEHOME");
    if (kdehome){
        s = kdehome;
    }else{
        s += ".kde/";
    }
    if (s.length() == 0)
		s += '/';
    if (s.right(1) != "/")
		s += '/';
    s += "share/apps/sim";
#else
    s += ".sim";
#endif
#else
    char szPath[1024];
    szPath[0] = 0;
    QString defPath;
    (DWORD&)_SHGetSpecialFolderPathW = (DWORD)QLibrary::resolve("Shell32.dll","SHGetSpecialFolderPathW");
    (DWORD&)_SHGetSpecialFolderPathA = (DWORD)QLibrary::resolve("Shell32.dll","SHGetSpecialFolderPathA");
    if (_SHGetSpecialFolderPathW && _SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, true)){
		defPath = QString::fromUcs2((unsigned short*)szPath);
    }else if (_SHGetSpecialFolderPathA && _SHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, true)){
        defPath = QFile::decodeName(szPath);
    }
    if (!defPath.isEmpty()){
        if (defPath.right(1) != "\\")
            defPath += "\\";
        defPath += "sim";
        QString ss;
        ss = defPath;
        ss += "\\";
		makedir(QFile::encodeName(ss).data());
        QString lockTest = defPath + "\\.lock";
        QFile f(lockTest);
        if (!f.open(IO_ReadWrite | IO_Truncate))
            defPath = "";
        f.close();
        QFile::remove(lockTest);
    }
    if (!defPath.isEmpty()){
        s = QFile::encodeName(defPath);
    }else{
        s = app_file("");
    }
#endif
#ifdef HAVE_CHMOD
    chmod(s.local8Bit(), 0700);
#endif
    return QDir::convertSeparators(s);
}

#ifdef WIN32

QWidget *HomeDirPlugin::createConfigWindow(QWidget *parent)
{
    return new HomeDirConfig(parent, this);
}

QString HomeDirPlugin::getConfig()
{
    if (!m_bSave)
        return "";
	QSettings setting( QSettings::Native );
	setting.setPath( "/SIM", "", QSettings::User );

	if (!m_bDefault){
		setting.writeEntry( "Path", m_homeDir );
    }else{
        setting.removeEntry( "Path" );
    }
    return "";
}

#endif

QString HomeDirPlugin::buildFileName(const QString *name)
{
    QString s;
    QString fname = *name;
#ifdef WIN32
    if ((fname[1] != ':') && (fname.left(2) != "\\\\")){
#else
    if (fname[0] != '/'){
#endif
        s += m_homeDir;
        s += '/';
    }
    s += fname;
    return QFile::encodeName(QDir::convertSeparators(s));
}

void *HomeDirPlugin::processEvent(Event *e)
{
    if (e->type() == EventHomeDir){
		QString *cfg = (QString*)(e->param());
		*cfg = buildFileName(cfg);
        return (void*)(!cfg->isEmpty());
    }
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

#endif


