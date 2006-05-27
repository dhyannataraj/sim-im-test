/***************************************************************************
                          wifacecfg.h  -  description
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

#ifndef _WIFACECFG_H
#define _WIFACECFG_H

#include "simapi.h"
#include "wifacecfgbase.h"

class WeatherPlugin;

class WIfaceCfg : public WIfaceCfgBase
{
    Q_OBJECT
public:
    WIfaceCfg(QWidget *parent, WeatherPlugin*);
public slots:
    void apply();
    void help();
protected:
    WeatherPlugin *m_plugin;
};

#endif

