/***************************************************************************
                          logger.cpp  -  description
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

#include "logger.h"
#include "simapi.h"
#include "logconfig.h"

#include <stdio.h>
#include <qapplication.h>

Plugin *createLoggerPlugin(unsigned base, bool, const char *add_info)
{
    LoggerPlugin *plugin = new LoggerPlugin(base, add_info);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Logger"),
        I18N_NOOP("Plugin provides log output\n"
                  "You can set debug level from command line with -d <loglevel>"),
        VERSION,
        createLoggerPlugin,
#ifdef WIN32
        PLUGIN_NOLOAD_DEFAULT
#else
        PLUGIN_DEFAULT
#endif
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*
typedef struct LoggerData
{
    unsigned LogLevel;
	char	 *LogPackets;
} LoggerData;
*/
static DataDef loggerData[] =
    {
        { "LogLevel", DATA_ULONG, 1, L_ERROR | L_WARN },
        { "LogPackets", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

LoggerPlugin::LoggerPlugin(unsigned base, const char *add_info)
        : Plugin(base)
{
    load_data(loggerData, &data, add_info);
    string value;
    CmdParam p = { "-d:", I18N_NOOP("Set debug level"), &value };
    Event e(EventArg, &p);
    if (e.process())
        setLogLevel(atol(value.c_str()));
    if (getLogPackets()){
        string packets = getLogPackets();
        while (packets.length()){
            string v = getToken(packets, ',');
            setLogType(atol(v.c_str()), true);
        }
    }
}

LoggerPlugin::~LoggerPlugin()
{
    free_data(loggerData, &data);
}

string LoggerPlugin::getConfig()
{
    string packets;
    for (list<unsigned>::iterator it = m_packets.begin(); it != m_packets.end(); ++it){
        if (packets.length())
            packets += ',';
        packets += number(*it);
    }
    setLogPackets(packets.c_str());
    return save_data(loggerData, &data);
}

bool LoggerPlugin::isLogType(unsigned id)
{
    for (list<unsigned>::iterator it = m_packets.begin(); it != m_packets.end(); ++it){
        if ((*it) == id)
            return true;
    }
    return false;
}

void LoggerPlugin::setLogType(unsigned id, bool bLog)
{
    list<unsigned>::iterator it;
    for (it = m_packets.begin(); it != m_packets.end(); ++it){
        if ((*it) == id)
            break;
    }
    if (bLog){
        if (it == m_packets.end())
            m_packets.push_back(id);
    }else{
        if (it != m_packets.end())
            m_packets.erase(it);
    }
}

QWidget *LoggerPlugin::createConfigWindow(QWidget *parent)
{
    return new LogConfig(parent, this);
}

void *LoggerPlugin::processEvent(Event *e)
{
    if (e->type() == EventLog){
        LogInfo *li = (LogInfo*)e->param();
        if (((li->packet_id == 0) && (li->log_level & getLogLevel())) ||
                (li->packet_id && isLogType(li->packet_id))){
            string s;
            s = make_packet_string(li);
#ifdef QT_DLL
            for (char *p = (char*)(s.c_str()); *p; ){
                char *r = strchr(p, '\n');
                if (r) *r = 0;
                qWarning("%s", p);
                if (r == NULL) break;
                p = r + 1;
            }
#else
            fprintf(stderr, "%s", s.c_str());
            fprintf(stderr, "\n");
#endif
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


