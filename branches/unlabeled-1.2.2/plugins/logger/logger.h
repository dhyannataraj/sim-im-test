/***************************************************************************
                          logger.h  -  description
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

#ifndef _LOGGER_H
#define _LOGGER_H

#include "simapi.h"

#include <list>
using namespace std;

typedef struct LoggerData
{
    unsigned LogLevel;
    char	 *LogPackets;
	char	 *File;
} LoggerData;

class QFile;

class LoggerPlugin : public Plugin, public EventReceiver
{
public:
    LoggerPlugin(unsigned, const char*);
    virtual ~LoggerPlugin();
    PROP_ULONG(LogLevel);
    PROP_STR(LogPackets);
	PROP_STR(File);
    bool isLogType(unsigned id);
    void setLogType(unsigned id, bool bLog);
protected:
    list<unsigned> m_packets;
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    void *processEvent(Event*);
	void openFile();
	QFile *m_file;
    LoggerData data;
    friend class LogConfig;
};

#endif

