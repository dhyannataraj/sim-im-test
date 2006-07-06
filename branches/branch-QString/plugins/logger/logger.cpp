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
#include "logconfig.h"

#include <stdio.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>

#ifdef WIN32
#include <windows.h>
#endif

using namespace SIM;

Plugin *createLoggerPlugin(unsigned base, bool, ConfigBuffer *add_info)
{
    LoggerPlugin *plugin = new LoggerPlugin(base, add_info);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Logger"),
        I18N_NOOP("Plugin provides log output\n"
                  "You can set debug level from command line with -d<loglevel>\n"
                  "To log errors, set loglevel to 1, for warnings to 2 and for debug-messages to 4\n"
                  "If you want to log more than one you may add the levels"),
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
        { "LogLevel", DATA_ULONG, 1, DATA(3) },
        { "LogPackets", DATA_STRING, 1, 0 },
        { "File", DATA_STRING, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

LoggerPlugin::LoggerPlugin(unsigned base, ConfigBuffer *add_info)
        : Plugin(base)
{
    m_file = NULL;
    load_data(loggerData, &data, add_info);
    QString value;
    CmdParam p = { "-d:", I18N_NOOP("Set debug level"), value };
    Event e(EventArg, &p);
    if (e.process() && !value.isEmpty())
        setLogLevel(value.toULong());
    if (getLogPackets()){
        QString packets = getLogPackets();
        while (packets.length()){
            QString v = getToken(packets, ',');
            setLogType(v.toULong(), true);
        }
    }
    m_bFilter = false;
    openFile();
}

LoggerPlugin::~LoggerPlugin()
{
    if (m_file)
        delete m_file;
    free_data(loggerData, &data);
}

QString LoggerPlugin::getConfig()
{
    QString packets;
    for (QValueList<unsigned>::iterator it = m_packets.begin(); it != m_packets.end(); ++it){
        if (packets.length())
            packets += ',';
        packets += QString::number(*it);
    }
    setLogPackets(packets);
    return save_data(loggerData, &data);
}

void LoggerPlugin::openFile()
{
/*
    if (m_bFilter){
        if ((getLogLevel() & L_EVENTS) == 0){
            qApp->removeEventFilter(this);
            m_bFilter = false;
        }
    }else{
        if (getLogLevel() & L_EVENTS){
            qApp->installEventFilter(this);
            m_bFilter = true;
        }
    }
*/
    if (m_file){
        delete m_file;
        m_file = NULL;
    }
    QString fname = getFile();
    if (fname.isEmpty())
        return;
    // This is because sim crashes when a logfile is larger than 100MB ...
    QFileInfo fileInfo(fname);
    if (fileInfo.size() > 1024 * 1024 * 50) {	// 50MB ...
        QString desiredFileName = fileInfo.fileName() + ".old";
#ifdef WIN32
        fileInfo.dir().remove(desiredFileName);
#endif
        if (!fileInfo.dir().rename(fileInfo.fileName(), desiredFileName)) {
            // won't work --> simply delete...
            fileInfo.dir().remove(fileInfo.fileName());
        }
    }
    // now open file
    m_file = new QFile(fname);
    if (!m_file->open(IO_Append | IO_ReadWrite)){
        delete m_file;
        m_file = NULL;
        log(L_WARN, "Can't open %s", fname);
    }
}

bool LoggerPlugin::isLogType(unsigned id)
{
    return ( m_packets.find( id ) != m_packets.end() );
}

void LoggerPlugin::setLogType(unsigned id, bool bLog)
{
    QValueList<unsigned>::iterator it;
    it = m_packets.find( id );
    if (bLog){
        if (it == m_packets.end())
            m_packets.push_back(id);
    }else{
        if (it != m_packets.end())
            m_packets.erase(it);
    }
}
/*
bool LoggerPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (strcmp(o->className(), "QTimer"))
        log(L_DEBUG, "Event: %u %s %s", e->type(), o->className(), o->name());
    return QObject::eventFilter(o, e);
}
*/
QWidget *LoggerPlugin::createConfigWindow(QWidget *parent)
{
    return new LogConfig(parent, this);
}

void *LoggerPlugin::processEvent(Event *e)
{
    if (e->type() == EventLog){
        LogInfo *li = (LogInfo*)e->param();
        if (((li->packet_id == 0) && (li->log_level & getLogLevel())) ||
                (li->packet_id && ((getLogLevel() & L_PACKETS) || isLogType(li->packet_id)))){
            QString s;
            s = make_packet_string(li);
            if (m_file){
#ifdef WIN32
                s += "\r\n";
#else
                s += "\n";
#endif
                m_file->writeBlock(s.latin1(), s.length());
            }
#ifdef QT_DLL
            QStringList slist = QStringList::split('\n',s);
            for (unsigned i = 0 ; i < slist.count() ; i++){
                QString out = slist[i];
                if (out.length() > 256){
                    while (!out.isEmpty()){
                        QString l;
                        if (out.length() < 256){
                            l = out;
                            out = "";
                        }else{
                            l = out.left(256);
                            out = out.mid(256);
                        }
                        OutputDebugStringA(l.local8Bit().data());
                    }
                }else{
                    OutputDebugStringA(out.local8Bit().data());
                }
                OutputDebugStringA("\n");
            }
#else
            fprintf(stderr, "%s\n", s.local8Bit().data());
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
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

#endif

#ifndef _MSC_VER
#include "logger.moc"
#endif

