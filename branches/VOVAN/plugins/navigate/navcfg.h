/***************************************************************************
                          navcfg.h  -  description
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

#ifndef _NAVCFG_H
#define _NAVCFG_H

#include "simapi.h"
#include "navcfgbase.h"

class NavigatePlugin;

class NavCfg : public NavCfgBase
{
    Q_OBJECT
public:
    NavCfg(QWidget *w, NavigatePlugin *plugin);
public slots:
    void apply();
protected:
    NavigatePlugin *m_plugin;
};

#endif

