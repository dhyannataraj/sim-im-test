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
#include "smiles.h"
#include "core.h"

bool my_string::operator < (const my_string &a) const
{
    return strcmp(c_str(), a.c_str()) < 0;
}

Plugin *createIconsPlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new IconsPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Icons"),
        I18N_NOOP("Plugin provides Miranda icons themes and emoticons"),
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
        { "Smiles", DATA_UTF, 1, 0 },
        { NULL, 0, 0, 0 }
    };

IconsPlugin::IconsPlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(iconsData, &data, config);
    smiles = NULL;
    if (!getSmiles().isEmpty()){
        smiles = new Smiles;
        if (!smiles->load(getSmiles())){
            delete smiles;
            smiles = NULL;
            setSmiles(NULL);
        }
    }
    setIcons();
}

IconsPlugin::~IconsPlugin()
{
    for (ICONS_MAP::iterator it = dlls.begin(); it != dlls.end(); ++it)
        delete (*it).second;
    dlls.clear();
    if (smiles){
        delete smiles;
        smiles = NULL;
    }
    setSmiles(NULL);
    Event e(EventIconChanged);
    e.process();
    free_data(iconsData, &data);
}

void IconsPlugin::setIcons()
{
    for (ICONS_MAP::iterator it = dlls.begin(); it != dlls.end(); ++it)
        delete (*it).second;
    dlls.clear();
    for (unsigned n = 1; ; n++){
        const char *cfg = getIconDLLs(n);
        if ((cfg == NULL) || (*cfg == 0))
            break;
        string v = cfg;
        string name = getToken(v, ',');
        if (v.length() == 0)
            continue;
        IconDLL *dll = new IconDLL;
        if (!dll->load(QString::fromUtf8(v.c_str())))
            continue;
        dlls.insert(ICONS_MAP::value_type(name.c_str(), dll));
    }
    if (smiles){
        string s;
        for (unsigned i = 0; i < smiles->count(); i++){
            const QIconSet *is = smiles->get(i);
            if (is == NULL){
                log(L_DEBUG, "Skip %u", i);
                s += '-';
                s += '\x00';
                s += '\x00';
                s += '\x00';
                continue;
            }
            QString pat = smiles->pattern(i);
            while (!pat.isEmpty()){
                QString p = getToken(pat, ' ', false);
                if (p.isEmpty())
                    continue;
                s += p.latin1();
                s += '\x00';
                log(L_DEBUG, "%u > %s", i, (const char*)(p.latin1()));
            }
            s += '\x00';
            QString tip = smiles->tip(i);
            if (!tip.isEmpty()){
                s += tip.local8Bit();
                log(L_DEBUG, "%u Tip %s", i, (const char*)(tip.local8Bit()));
            }
            s += '\x00';
            QString url = "smile";
            url += QString::number(i).upper();
            url = QString("icon:") + url;
            QMimeSourceFactory::defaultFactory()->setPixmap(url, is->pixmap(QIconSet::Small, QIconSet::Normal));
        }
        s += '\x00';
        s += '\x00';
        s += '\x00';
        SIM::setSmiles(s.c_str());
    }else{
        for (unsigned i = 0; i < 16; i++){
            QString url = "icon:smile";
            url += QString::number(i).upper();
            const QIconSet *is = Icon((const char*)(url.latin1()));
            if (is == NULL)
                continue;
            QMimeSourceFactory::defaultFactory()->setPixmap(url, is->pixmap(QIconSet::Small, QIconSet::Normal));
        }
        SIM::setSmiles(NULL);
    }
    Event eIcon(EventIconChanged);
    eIcon.process();
    Event eHistory(EventHistoryConfig);
    eHistory.process();
}

string IconsPlugin::getConfig()
{
    return save_data(iconsData, &data);
}

QWidget *IconsPlugin::createConfigWindow(QWidget *parent)
{
    return new IconCfg(parent, this);
}

typedef struct IconID
{
    const char *name;
    unsigned   id;
} IconID;


static IconID icons_def[] =
    {
        { "licq", 102 },
        { "message", 136 },
        { "url", 135 },
        { "sms", 103 },
        { "file", 207 },
        { "online", 104 },
        { "offline", 105 },
        { "away", 128 },
        { "na", 131 },
        { "dnd", 158 },
        { "occupied", 159 },
        { "ffc", 129 },
        { "invisible", 130 },
        { "info", 160 },
        { "history", 174 },
        { "mail_generic", 193 },
        { "find", 161 },
        { NULL, 0 }
    };

void *IconsPlugin::processEvent(Event *e)
{
    if (e->type() == EventGetIcon){
        const char *name = (const char*)(e->param());
        const char *p = strchr(name, '_');
        if (p){
            string s;
            s.append(name, (unsigned)(p - name));
            ICONS_MAP::iterator it = dlls.find(s.c_str());
            if (it == dlls.end())
                return NULL;
            s = p + 1;
            for (const IconID *d = icons_def; d->name; d++){
                if (s == d->name)
                    return (void*)((*it).second->get(d->id));
            }
            return NULL;
        }
        char _SMILE[] = "smile";
        if (smiles && (strlen(name) > strlen(_SMILE)) && (memcmp(name, _SMILE, strlen(_SMILE)) == 0)){
            unsigned nIcon = 0;
            p = name + strlen(_SMILE);
            for (; *p; p++){
                if ((*p >= '0') && (*p <= '9')){
                    nIcon = (nIcon * 10) + (*p - '0');
                    continue;
                }
                return NULL;
            }
            const QIconSet *is = smiles->get(nIcon);
            if (is)
                return (void*)is;
            if (nIcon < 16)
                return (void*)(-1);
            return NULL;
        }
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


