/***************************************************************************
                          weathercfg.h  -  description
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

#ifndef _WEATHERCFG_H
#define _WEATHERCFG_H

#include "simapi.h"
#include "weathercfgbase.h"

class WeatherPlugin;

class WeatherCfg : public WeatherCfgBase, public EventReceiver
{
    Q_OBJECT
public:
    WeatherCfg(QWidget *parent, WeatherPlugin*);
public slots:
    void apply();
protected:
    void *processEvent(Event*);
    void fill();
    WeatherPlugin *m_plugin;
};

#endif

