/***************************************************************************
                          smilecfg.h  -  description
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

#ifndef _SMILECFG_H
#define _SMILECFG_H

#include "simapi.h"
#include "smilecfgbase.h"

class IconsPlugin;

class SmileCfg : public SmileCfgBase
{
    Q_OBJECT
public:
    SmileCfg(QWidget *parent, IconsPlugin *plugin);
public slots:
    void apply();
    void goSmiles();
protected:
    IconsPlugin *m_plugin;
};

#endif

