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

#include "homedircfg.h"

static BOOL (WINAPI *_SHGetSpecialFolderPath)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;

#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

#include <qdir.h>

Plugin *createHomeDirPlugin(unsigned base, bool, const char*)
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
        PLUGIN_NO_CONFIG_PATH
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
    m_bDefault = true;
    m_bSave    = true;
    HKEY subKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, key_name, 0,
                      KEY_READ | KEY_QUERY_VALUE, &subKey) == ERROR_SUCCESS){
        DWORD vType = REG_SZ;
        DWORD vCount = 0;
        if (RegQueryValueExA(subKey, path_value, NULL, &vType, NULL, &vCount) == ERROR_SUCCESS){
            m_homeDir = "";
            m_homeDir.append(vCount, '\x00');
            RegQueryValueExA(subKey, path_value, NULL, &vType, (unsigned char*)m_homeDir.c_str(), &vCount);
            m_bDefault = false;
        }
        RegCloseKey(subKey);
    }
#endif
    string value;
    CmdParam p = { "-b:", I18N_NOOP("Set home directory"), &value };
    Event e(EventArg, &p);
    if (e.process()){
        m_homeDir = value;
#ifdef WIN32
        m_bSave   = false;
#endif
    }
    if (m_homeDir.empty())
         m_homeDir = defaultPath();
    QDir dir(m_homeDir.c_str());
    if (dir.exists()) {
#ifdef WIN32
    QString directory = dir.path();
    directory.replace(QRegExp("/"),"\\");
    m_homeDir = directory.latin1();
#else
    m_homeDir = dir.path()+"/";
#endif
    } else {
        m_homeDir = defaultPath();
#ifdef WIN32
        m_bSave   = false;
#endif
    }
}

string HomeDirPlugin::defaultPath()
{
    string s;
#ifndef WIN32
    struct passwd *pwd = getpwuid(getuid());
    if (pwd){
        s = pwd->pw_dir;
    }else{
        log(L_ERROR, "Can't get pwd");
    }
    if (s[s.size() - 1] != '/') s += '/';
#ifdef USE_KDE
    char *kdehome = getenv("KDEHOME");
    if (kdehome){
        s = kdehome;
    }else{
        s += ".kde/";
    }
    if (s.length() == 0) s += '/';
    if (s[s.length()-1] != '/') s += '/';
    s += "share/apps/sim/";
#else
    s += ".sim/";
#endif
#else
    char szPath[512];
    HINSTANCE hLib = LoadLibraryA("Shell32.dll");
    if (hLib != NULL)
        (DWORD&)_SHGetSpecialFolderPath = (DWORD)GetProcAddress(hLib,"SHGetSpecialFolderPathA");
    if (_SHGetSpecialFolderPath && _SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, true)){
        s = szPath;
        if (s.length()  == 0) s = "c:\\";
        if (s[s.length() - 1] != '\\') s += '\\';
        s += "sim\\";
    }else{
        s = app_file("");
    }
#endif
#ifndef _WINDOWS
    if (s[s.length() - 1] != '/') s += '/';
#else
    if (s[s.length() - 1] != '\\') s += '\\';
#endif
    return s;
}

#ifdef WIN32

QWidget *HomeDirPlugin::createConfigWindow(QWidget *parent)
{
    return new HomeDirConfig(parent, this);
}

string HomeDirPlugin::getConfig()
{
    if (!m_bSave)
        return "";
    HKEY subKey;
    DWORD disposition;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, key_name, 0, "",
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &subKey, &disposition) != ERROR_SUCCESS)
        return "";
    if (!m_bDefault){
        RegSetValueExA(subKey, path_value, 0, REG_SZ, (const unsigned char*)m_homeDir.c_str(), m_homeDir.length());
    }else{
        RegDeleteValueA(subKey, path_value);
    }
    RegCloseKey(subKey);
    return "";
}

#endif

string HomeDirPlugin::buildFileName(const char *name)
{
    string s = m_homeDir;
#ifdef WIN32
    if (s[s.size() - 1] != '/') s += '/';
#endif
    s += name;
    makedir((char*)s.c_str());
    return s;
}

void *HomeDirPlugin::processEvent(Event *e)
{
    if (e->type() == EventHomeDir){
        string *cfg = (string*)(e->param());
        *cfg = buildFileName(cfg->c_str());
        return (void*)(cfg->c_str());
    }
    return NULL;
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


