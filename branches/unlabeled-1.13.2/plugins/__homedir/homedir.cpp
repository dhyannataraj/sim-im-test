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

static BOOL (WINAPI *_SHGetSpecialFolderPathA)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
static BOOL (WINAPI *_SHGetSpecialFolderPathW)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;

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
    QString d = QFile::decodeName(m_homeDir.c_str());
#ifdef WIN32
    d = d.replace(QRegExp("/"), "\\");
    if (d.length() && (d[(int)(d.length() - 1)] == '\\'))
        d = d.left(d.length() - 1);
    if (d.length() && d[(int)(d.length() - 1)] == ':')
        d += "\\";
#else
    if (d.length() && (d[(int)(d.length() - 1)] == '/'))
        d = d.left(d.length() - 1);
#endif
    QDir dir(d);
    if (!dir.exists()) {
        m_homeDir  = defaultPath();
#ifdef WIN32
        m_bDefault = true;
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
    if (s[s.size() - 1] != '/')
        s += '/';
#ifdef USE_KDE
    char *kdehome = getenv("KDEHOME");
    if (kdehome){
        s = kdehome;
    }else{
        s += ".kde/";
    }
    if (s.length() == 0) s += '/';
    if (s[s.length()-1] != '/') s += '/';
    s += "share/apps/sim";
#else
    s += ".sim";
#endif
#else
    char szPath[1024];
    szPath[0] = 0;
    HINSTANCE hLib = LoadLibraryA("Shell32.dll");
    QString defPath;
    if (hLib != NULL){
        (DWORD&)_SHGetSpecialFolderPathW = (DWORD)GetProcAddress(hLib,"SHGetSpecialFolderPathW");
        (DWORD&)_SHGetSpecialFolderPathA = (DWORD)GetProcAddress(hLib,"SHGetSpecialFolderPathA");
    }
    if (_SHGetSpecialFolderPathW && _SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, true)){
        for (unsigned short *str = (unsigned short*)szPath; *str; str++)
            defPath += QChar(*str);
    }else if (_SHGetSpecialFolderPathA && _SHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, true)){
        defPath = QFile::decodeName(szPath);
    }
    if (!defPath.isEmpty()){
        if (defPath[(int)(defPath.length() - 1)] != '\\')
            defPath += "\\";
        defPath += "sim";
        string ss;
        ss = QFile::encodeName(defPath);
        ss += "\\";
        makedir((char*)(ss.c_str()));
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
    chmod(s.c_str(), 0700);
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
    QString s;
    QString fname = QFile::decodeName(name);
#ifdef WIN32
    if ((fname[1] != ':') && (fname.left(2) != "\\\\")){
#else
    if (fname[0] != '/'){
#endif
        s += QFile::decodeName(m_homeDir.c_str());
#ifdef WIN32
        s += '\\';
#else
        s += '/';
#endif
    }
    s += fname;
    string res;
    res = QFile::encodeName(s);
    return res;
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


