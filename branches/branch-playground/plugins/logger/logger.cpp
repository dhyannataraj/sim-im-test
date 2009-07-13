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

#include <stdio.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
//Added by qt3to4:
#include <QByteArray>

#ifdef Q_OS_WIN
# include <windows.h>
#endif

#include "log.h"
#include "misc.h"

#include "logger.h"
#include "logconfig.h"

using namespace std;
using namespace SIM;

Plugin *createLoggerPlugin(unsigned base, bool, Buffer *add_info)
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
#if defined(Q_OS_WIN) || defined(__OS2__)
        PLUGIN_NOLOAD_DEFAULT
#else
        PLUGIN_DEFAULT
#endif
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

LoggerPlugin::LoggerPlugin(unsigned base, Buffer *add_info)
        : Plugin(base),
        PropertyHub("logger"),
        m_file(NULL)
{

    EventArg e("-d:", I18N_NOOP("Set debug level"));
    if (e.process())
        setProperty("LogLevel", e.value().toUInt());
    const QStringList packets = property("LogPackets").toString().split(',');
    Q_FOREACH (const QString &v, packets) {
        setLogType(v.toULong(), true);
    }
    m_bFilter = false;
    openFile();
}

LoggerPlugin::~LoggerPlugin()
{
    delete m_file;
    PropertyHub::save();
}

QByteArray LoggerPlugin::getConfig()
{
    /*
    QByteArray packets;
    QSetIterator<unsigned> setIt(m_packets);
    while(setIt.hasNext()) {
        if (packets.length())
            packets += ',';
        packets += QByteArray::number(setIt.next());
    }
    setProperty("LogPackets", packets);
    */
    return QByteArray();
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
    delete m_file;
    m_file = NULL;
    QString fname = property("File").toString();
    if (fname.isEmpty())
        return;
    // This is because sim crashes when a logfile is larger than 100MB ...
    QFileInfo fileInfo(fname);
    if (fileInfo.size() > 1024 * 1024 * 50) {	// 50MB ...
        QString desiredFileName = fileInfo.fileName() + ".old";
#if defined(Q_OS_WIN) || defined(__OS2__)
        fileInfo.dir().remove(desiredFileName);
#endif
        if (!fileInfo.dir().rename(fileInfo.fileName(), desiredFileName)) {
            // won't work --> simply delete...
            fileInfo.dir().remove(fileInfo.fileName());
        }
    }
    // now open file
    m_file = new QFile(fname);
    if (!m_file->open(QIODevice::Append | QIODevice::ReadWrite)){
        delete m_file;
        m_file = NULL;
        log(L_WARN, "Can't open %s", qPrintable(fname));
    }
}

bool LoggerPlugin::isLogType(unsigned id)
{
    return m_packets.contains(id);
}

void LoggerPlugin::setLogType(unsigned id, bool bLog)
{
    if(bLog)
      m_packets.insert(id);
    else
      m_packets.remove(id);
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

bool LoggerPlugin::processEvent(Event *e)
{
    if(e->type() == eEventLog)
    {
        EventLog *l = static_cast<EventLog*>(e);
        if (((l->packetID() == 0) && (l->logLevel() & property("LogLevel").toUInt())) ||
                (l->packetID() && ((property("LogLevel").toUInt() & L_PACKETS) || isLogType(l->packetID()))))
        {
            QString s;
            s = EventLog::make_packet_string(*l);
            if (m_file) {
#if defined(Q_OS_WIN) || defined(__OS2__)
                s += "\r\n";
#else
                s += "\n";
#endif
                m_file->write(s.toLocal8Bit());
            }
#ifdef Q_OS_WIN
            const QStringList slist = s.split('\n');
            for(int i = 0 ; i < slist.count() ; i++)
            {
                QString out = slist[i];
                if (out.length() > 256){
                    while (!out.isEmpty()){
                        QString l;
                        if (out.length() < 256){
                            l = out;
                            out.clear();
                        }else{
                            l = out.left(256);
                            out = out.mid(256);
                        }
                        OutputDebugStringW((LPCWSTR)l.utf16());
                    }
                }
                else
                {
                    OutputDebugStringW((LPCWSTR)out.utf16());
                }
                OutputDebugStringW(L"\n");
            }
#else
            fprintf(stderr, "%s\n", qPrintable(s));
#endif
        }
    }
    else if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHub::load();
    }
    return false;
}
