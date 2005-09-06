/***************************************************************************
                          update.cpp  -  description
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

#include "update.h"
#include "socket.h"
#include "core.h"
#include "ballonmsg.h"

#include <time.h>
#include <stdio.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qregexp.h>

const unsigned CHECK_INTERVAL = 60 * 60 * 24;

Plugin *createUpdatePlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new UpdatePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Update"),
        I18N_NOOP("Plugin provides notifications about update SIM software"),
        VERSION,
        createUpdatePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef updateData[] =
    {
        { "Time", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

UpdatePlugin::UpdatePlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    load_data(updateData, &data, config);
    CmdGo = registerType();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(15000);
}

UpdatePlugin::~UpdatePlugin()
{
    free_data(updateData, &data);
}

string UpdatePlugin::getConfig()
{
    return save_data(updateData, &data);
}

void UpdatePlugin::timeout()
{
    if (!getSocketFactory()->isActive() || !isDone())
        return;
    time_t now;
    time(&now);
    if ((unsigned)now >= getTime() + CHECK_INTERVAL){
        string url = "http://sim.shutoff.ru/cgi-bin/update1.pl?v=" VERSION;
#ifdef WIN32
        url += "&os=1";
#else
#ifdef QT_MACOSX_VERSION
        url += "&os=2";
#endif
#endif
#ifdef CVS_BUILD
        url += "&cvs=";
        for (const char *p = __DATE__; *p; p++){
            if (*p == ' '){
                url += "%20";
                continue;
            }
            url += *p;
        }
#else
        url += "&release";
#endif
        url += "&l=";
        QString s = i18n("Message", "%n messages", 1);
        s = s.replace(QRegExp("1 "), "");
        for (int i = 0; i < (int)(s.length()); i++){
            unsigned short c = s[i].unicode();
            if ((c == ' ') || (c == '%') || (c == '=') || (c == '&')){
                char b[5];
                sprintf(b, "%02X", c);
                url += b;
            }else if (c > 0x77){
                char b[10];
                sprintf(b, "#%04X", c);
                url += b;
            }else{
                url += (char)c;
            }
        }
        fetch(url.c_str(), NULL, NULL, false);
    }
}

#if 0
I18N_NOOP("Show details")
I18N_NOOP("Remind later")
#endif

bool UpdatePlugin::done(unsigned, Buffer&, const char *headers)
{
    string h = getHeader("Location", headers);
    if (!h.empty()){
        Command cmd;
        cmd->id		= CmdStatusBar;
        Event eWidget(EventCommandWidget, cmd);
        QWidget *statusWidget = (QWidget*)(eWidget.process());
        if (statusWidget == NULL)
            return false;
        m_url = h;
        clientErrorData d;
        d.client  = NULL;
        d.err_str = I18N_NOOP("New version SIM is released");
        d.code	  = 0;
        d.args    = NULL;
        d.flags	  = ERR_INFO;
        d.options = "Show details\x00Remind later\x00\x00";
        d.id	  = CmdGo;
        Event e(EventShowError, &d);
        e.process();
    }
    time_t now;
    time(&now);
    setTime(now);
    Event e(EventSaveState);
    e.process();
    return false;
}

void *UpdatePlugin::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdGo){
            Event eGo(EventGoURL, (void*)(m_url.c_str()));
            eGo.process();
            time_t now;
            time(&now);
            setTime(now);
            m_url = "";
            Event eSave(EventSaveState);
            eSave.process();
            return e->param();
        }
    }
    return NULL;
}

string UpdatePlugin::getHeader(const char *name, const char *headers)
{
    for (const char *h = headers; *h; h += strlen(h) + 1){
        string header = h;
        string key = getToken(header, ':');
        if (key != name)
            continue;
        const char *p;
        for (p = header.c_str(); *p; p++)
            if (*p != ' ')
                break;
        return p;
    }
    return "";
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
#include "update.moc"
#endif


