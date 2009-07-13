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

#include <QObject>
#include <QSet>

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

class QFile;
const unsigned short L_PACKETS = 0x08;
// const unsigned short L_EVENTS  = 0x10;

class QFile;

class LoggerPlugin : virtual public QObject, public SIM::Plugin, public SIM::EventReceiver, public SIM::PropertyHub
{
    Q_OBJECT
public:
    LoggerPlugin(unsigned, Buffer*);
    virtual ~LoggerPlugin();
    bool isLogType(unsigned id);
    void setLogType(unsigned id, bool bLog);
protected:
//    bool eventFilter(QObject *o, QEvent *e);
    QSet<unsigned> m_packets;
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QByteArray getConfig();
    virtual bool processEvent(SIM::Event *e);
    void openFile();
    QFile *m_file;
    bool m_bFilter;
    friend class LogConfig;
};

#endif

