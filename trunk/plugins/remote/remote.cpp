/***************************************************************************
                          remote.cpp  -  description
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

#include "remote.h"
#include "remotecfg.h"
#include "simapi.h"

#include "core.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qregexp.h>

Plugin *createRemotePlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new RemotePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Remote control"),
        I18N_NOOP("Plugin provides remote control"),
        VERSION,
        createRemotePlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef remoteData[] =
    {
#ifdef WIN32
        { "Path", DATA_STRING, 1, (unsigned)"tcp:3000" },
#else
        { "Path", DATA_STRING, 1, (unsigned)"/tcp/sim.%user%" },
#endif
        { NULL, 0, 0, 0 }
    };

RemotePlugin::RemotePlugin(unsigned base, const char *config)
        : Plugin(base)
{
    load_data(remoteData, &data, config);
    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
    bind();
}

RemotePlugin::~RemotePlugin()
{
    while (!m_sockets.empty())
        delete m_sockets.front();
    free_data(remoteData, &data);
}

string RemotePlugin::getConfig()
{
    return save_data(remoteData, &data);
}

QWidget *RemotePlugin::createConfigWindow(QWidget *parent)
{
    return new RemoteConfig(parent, this);
}

void *RemotePlugin::processEvent(Event*)
{
    return NULL;
}

static char TCP[] = "tcp:";


void RemotePlugin::bind()
{
    const char *path = getPath();
    if ((strlen(path) > strlen(TCP)) && !memcmp(path, TCP, strlen(TCP))){
        unsigned short port = (unsigned short)atol(path + strlen(TCP));
        ServerSocketNotify::bind(port, port, NULL);
#ifndef WIN32
    }else{
        ServerSocketNotify::bind(path);
#endif
    }
}

bool RemotePlugin::accept(Socket *s, unsigned long)
{
    log(L_DEBUG, "Accept remote control");
    new ControlSocket(this, s);
    return false;
}

void RemotePlugin::bind_ready(unsigned short)
{
}

bool RemotePlugin::error(const char *err)
{
    if (*err)
        log(L_DEBUG, "Remote: %s", err);
    return true;
}

static char CRLF[] = "\r\n>";

ControlSocket::ControlSocket(RemotePlugin *plugin, Socket *socket)
{
    m_plugin = plugin;
    m_plugin->m_sockets.push_back(this);
    m_socket = new ClientSocket(this);
    m_socket->setSocket(socket);
    m_socket->setRaw(true);
    m_socket->readBuffer.init(0);
    m_socket->readBuffer.packetStart();
    write(CRLF);
}

ControlSocket::~ControlSocket()
{
    for (list<ControlSocket*>::iterator it = m_plugin->m_sockets.begin(); it != m_plugin->m_sockets.end(); ++it){
        if ((*it) == this){
            m_plugin->m_sockets.erase(it);
            break;
        }
    }
    delete m_socket;
}

void ControlSocket::write(const char *msg)
{
    log(L_DEBUG, "Remote write %s", msg);
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer.pack(msg, strlen(msg));
    m_socket->write();
}

bool ControlSocket::error_state(const char *err, unsigned)
{
    if (err && *err)
        log(L_WARN, "ControlSocket error %s", err);
    return true;
}

void ControlSocket::connect_ready()
{
}

const unsigned CMD_STATUS		= 0;
const unsigned CMD_INVISIBLE	= 1;
const unsigned CMD_MAINWND		= 2;
const unsigned CMD_SEARCHWND	= 3;
const unsigned CMD_QUIT			= 4;
const unsigned CMD_CLOSE		= 5;
const unsigned CMD_HELP			= 6;
// const unsigned CMD_MESSAGE		= 2;
// const unsigned CMD_SMS			= 3;
// const unsigned CMD_DOCK			= 6;
// const unsigned CMD_NOTIFY		= 7;
// const unsigned CMD_ICON			= 8;
// const unsigned CMD_OPEN			= 9;
// const unsigned CMD_POPUP		= 10;

typedef struct cmdDef
{
    const char *cmd;
    const char *shortDescr;
    const char *longDescr;
    unsigned minArgs;
    unsigned maxArgs;
} cmdDef;

static cmdDef cmds[] =
    {
        { "STATUS", "set status", "STATUS [status]", 0, 1 },
        { "INVISIBLE", "set invisible mode", "INVISIBLE [on|off]", 0, 1 },
        { "MAINWINDOW", "show/hide main window", "MAINWINDOW [on|off|toggle]", 0, 1 },
        { "SEARCHWINDOW", "show/hide search window", "SEARCHWINDOW [on|off]", 0, 1 },
        { "QUIT", "quit SIM", "QUIT", 0, 0 },
        { "CLOSE", "close session", "CLOSE", 0, 0 },
        { "HELP", "command help information", "HELP [<cmd>]", 0, 1 },
        { NULL, NULL, NULL, 0, 0 }
    };

#if 0
{ "MESSAGE", "send message", "MESSAGE <UIN|Name> <message>", 2, 2 },
{ "SMS", "send SMS", "SMS <phone> <message>", 2, 2 },
{ "DOCK", "show/hide dock", "DOCK [on|off]", 0, 1 },
{ "NOTIFY", "set notify mode", "NOTIFY [on|off]", 0, 1 },
{ "ICON", "get icon in xpm format", "ICON nIcon", 1, 1 },
{ "OPEN", "open unread message", "OPEN", 0, 0 },
{ "POPUP", "show popup", "POPUP x y", 2, 2 },
#endif

static bool isOn(string &s)
{
    return (s == "1") || (s == "on") || (s == "ON");
}

static bool cmpStatus(const char *s1, const char *s2)
{
    QString ss1 = s1;
    QString ss2 = s2;
    ss1 = ss1.replace(QRegExp("\\&"), "");
    ss2 = ss2.replace(QRegExp("\\&"), "");
    return ss1.lower() == ss2.lower();
}

static QWidget *findWidget(const char *className)
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget *w;
    while ((w = it.current()) != NULL){
        if (w->inherits(className))
            break;
        ++it;
    }
    return w;
}

void ControlSocket::packet_ready()
{
    string line;
    if (!m_socket->readBuffer.scan("\n", line))
        return;
    if (line.empty())
        return;
    if (line[(int)line.size() - 1] == '\r')
        line = line.substr(0, line.size() - 1);
    log(L_DEBUG, "Remote read: %s", line.c_str());
    string cmd;
    vector<string> args;
    const char *p = line.c_str();
    for (; *p; p++)
        if (*p != ' ')
            break;
    for (; *p; p++){
        if (*p == ' ')
            break;
        cmd += *p;
    }
    for (; *p; ){
        for (; *p; p++)
            if (*p != ' ')
                break;
        if (*p == 0)
            break;
        string arg;
        if ((*p == '\'') || (*p == '\"')){
            char c = *p;
            for (p++; *p; p++){
                if (*p == c){
                    break;
                }
                arg += *p;
            }
        }else{
            for (; *p; p++){
                if (*p == '\\'){
                    p++;
                    if (*p == 0)
                        break;
                }
                arg += *p;
            }
        }
        args.push_back(arg);
    }
    unsigned nCmd = 0;
    const cmdDef *c;
    for (c = cmds; c->cmd; c++, nCmd++)
        if (cmd == c->cmd)
            break;
    if (c->cmd == NULL){
        string msg = "? Unknown command ";
        msg += cmd;
        write(msg.c_str());
        write(CRLF);
    }
    if ((args.size() < c->minArgs) || (args.size() > c->maxArgs)){
        string msg = "? Bad arguments number. Try help ";
        msg += cmd;
        write(msg.c_str());
        write(CRLF);
        return;
    }
    string msg;
    QWidget *w;
    switch (nCmd){
    case CMD_STATUS:
        if (args.size()){
            unsigned status = STATUS_UNKNOWN;
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                for (const CommandDef *d = client->protocol()->statusList(); d->text; d++){
                    if (cmpStatus(d->text, args[0].c_str())){
                        status = d->id;
                        break;
                    }
                }
                if (status != STATUS_UNKNOWN)
                    break;
            }
            if (status == STATUS_UNKNOWN){
                msg = "? Unknown status ";
                msg += args[0];
            }else{
                for (unsigned i = 0; i < getContacts()->nClients(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->getCommonStatus())
                        client->setStatus(status, true);
                }
                m_plugin->core->setManualStatus(status);
            }
        }else{
            for (unsigned i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (client->getCommonStatus()){
                    const CommandDef *d = NULL;
                    for (d = client->protocol()->statusList(); d->text; d++){
                        if (d->id == m_plugin->core->getManualStatus())
                            break;
                    }
                    if (d){
                        msg = "STATUS ";
                        for (const char *p = d->text; *p; p++){
                            if (*p == '&')
                                continue;
                            msg += *p;
                        }
                        write(msg.c_str());
                        break;
                    }
                }
            }
        }
        break;
    case CMD_INVISIBLE:
        if (args.size()){
            bool bInvisible = isOn(args[0]);
            if (m_plugin->core->getInvisible() != bInvisible){
                m_plugin->core->setInvisible(bInvisible);
                for (unsigned i = 0; i < getContacts()->nClients(); i++)
                    getContacts()->getClient(i)->setInvisible(bInvisible);
            }
        }else{
            msg  = "INVISIBLE ";
            msg += m_plugin->core->getInvisible() ? "on" : "off";
            write(msg.c_str());
        }
        break;
    case CMD_MAINWND:
        w = findWidget("SearchDialog");
        if (args.size()){
            if (isOn(args[0])){
                if (w)
                    raiseWindow(w);
            }else{
                if (w)
                    w->hide();
            }
        }else if ((args[0] == "toggle") || (args[0] == "TOGGLE")){
            if (w){
                if (w->isVisible()){
                    w->hide();
                }else{
                    w->show();
                }
            }
        }else{
            write("MAINWINDOW ");
            write(w ? "on" : "off");
        }
        break;
    case CMD_SEARCHWND:
        w = findWidget("SearchDialog");
        if (args.size()){
            if (isOn(args[0])){
                if (w){
                    raiseWindow(w);
                }else{
                    Command cc;
                    cc->id = CmdSearch;
                    Event e(EventCommandExec, cc);
                    e.process();
                }
            }else{
                if (w)
                    w->close();
            }
        }else{
            write("SEARCHWINDOW ");
            write(w ? "on" : "off");
        }
        break;
    case CMD_QUIT:{
            Command cc;
            cc->id = CmdQuit;
            Event e(EventCommandExec, cc);
            e.process();
            break;
        }
    case CMD_CLOSE:
        m_socket->error_state("");
        return;
    case CMD_HELP:
        if (args.size() == 0){
            for (c = cmds; c->cmd; c++){
                msg = c->cmd;
                msg += "\t";
                msg += c->shortDescr;
                msg += "\r\n";
                write(msg.c_str());
            }
        }else{
            for (c = cmds; c->cmd; c++)
                if (args[0] == c->cmd)
                    break;
            if (c->cmd == NULL){
                msg = "? Unknown command ";
                msg += args[0];
                write(msg.c_str());
                break;
            }
            msg = c->cmd;
            msg += "\t";
            msg += c->shortDescr;
            msg += "\r\n";
            msg += c->longDescr;
            write(msg.c_str());
        }
        break;
    }
    write(CRLF);
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

