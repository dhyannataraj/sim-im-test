/***************************************************************************
                          plugincfg.h  -  description
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

#ifndef _PLUGINCFG_H
#define _PLUGINCFG_H

#include "simapi.h"
#include "plugincfgbase.h"

class PluginCfg : public PluginCfgBase
{
    Q_OBJECT
public:
    PluginCfg(QWidget *parent, pluginInfo *info);
public slots:
    void apply();
protected:
    pluginInfo *m_info;
};

#endif

