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
#include "fetch.h"
#include "core.h"
#include "ballonmsg.h"

#include <time.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qregexp.h>

const unsigned CHECK_INTERVAL = 60 * 60 * 24;

Plugin *createUpdatePlugin(unsigned base, bool, const char *config)
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

UpdatePlugin::UpdatePlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(updateData, &data, config);
    m_fetch_id = 0;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(60000);
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
    if (!getSocketFactory()->isActive() || m_fetch_id)
        return;
    time_t now;
    time(&now);
    if ((unsigned)now >= getTime() + CHECK_INTERVAL){
        string url = "http://sim.shutoff.ru/cgi-bin/update.pl?v=" VERSION;
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
#endif
        url += "&l=";
        QString s = i18n("Message", "%n messages", 1);
        s = s.replace(QRegExp("1 "), "");
        for (int i = 0; i < (int)(s.length()); i++){
            unsigned short c = s[i].unicode();
            if ((c == ' ') || (c == '%')){
                char b[5];
                sprintf(b, "%02X", c);
                url += b;
            }else if (c > 0x77){
                char b[10];
                sprintf(b, "%#04X", c);
                url += b;
            }else{
                url += (char)c;
            }
        }
        TCPClient *client = NULL;
        if (getContacts()->nClients())
            client = static_cast<TCPClient*>(getContacts()->getClient(0));
        m_fetch_id = fetch(NULL, url.c_str());
    }
}

void *UpdatePlugin::processEvent(Event *e)
{
    if (e->type() == EventFetchDone){
        fetchData *data = (fetchData*)(e->param());
        if (data->req_id != m_fetch_id)
            return NULL;
        string h = getHeader("Location", data->headers);
        if (h.empty()){
            if (data->result == 200){
                time_t now;
                time(&now);
                setTime(now);
            }
        }else{
            QWidget *main = getMainWindow();
            if (main == NULL)
                return NULL;
            Command cmd;
            cmd->id		= CmdStatusBar;
            Event eWidget(EventCommandWidget, cmd);
            QWidget *statusWidget = (QWidget*)(eWidget.process());
            if (statusWidget == NULL)
                return NULL;
            m_url = h;
            QStringList l;
            l.append(i18n("Show details"));
            l.append(i18n("Remind later"));
            raiseWindow(main);
            BalloonMsg *msg = new BalloonMsg(NULL, i18n("New version SIM is released"), l, statusWidget);
            connect(msg, SIGNAL(action(int, void*)), this, SLOT(showDetails(int, void*)));
            connect(msg, SIGNAL(destroyed()), this, SLOT(msgDestroyed()));
            msg->show();
            return NULL;
        }
        m_fetch_id = 0;
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

QWidget *UpdatePlugin::getMainWindow()
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget *w;
    while ( (w=it.current()) != 0 ) {
        ++it;
        if (w->inherits("MainWindow")){
            delete list;
            return w;
        }
    }
    delete list;
    return NULL;
}

void UpdatePlugin::showDetails(int n, void*)
{
    if (n == 0){
        Event e(EventGoURL, (void*)(m_url.c_str()));
        e.process();
    }
    time_t now;
    time(&now);
    setTime(now);
    m_url = "";
    m_fetch_id = 0;
}

void UpdatePlugin::msgDestroyed()
{
    time_t now;
    time(&now);
    setTime(now);
    m_fetch_id = 0;
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


