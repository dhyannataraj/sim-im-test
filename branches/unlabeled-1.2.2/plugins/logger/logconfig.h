/***************************************************************************
                          logconfig.h  -  description
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

#ifndef _LOGCONFIG_H
#define _LOGCONFIG_H

#include "simapi.h"
#include "logconfigbase.h"

#include <map>
using namespace std;

class QCheckBox;
typedef map<unsigned, QCheckBox*> BOX_MAP;

class LoggerPlugin;

class LogConfig : public LogConfigBase, public EventReceiver
{
    Q_OBJECT
public:
    LogConfig(QWidget *parent, LoggerPlugin *plugin);
public slots:
    void apply();
protected:
    void *processEvent(Event*);
    void fill();
    LoggerPlugin	*m_plugin;
    BOX_MAP			m_boxes;
};

#endif

