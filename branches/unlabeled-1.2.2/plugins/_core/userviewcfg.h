/***************************************************************************
                          userviewcfg.h  -  description
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

#ifndef _USERVIEWCFG_H
#define _USERVIEWCFG_H

#include "simapi.h"
#include "qcolorbutton.h"
#include "userviewcfgbase.h"

class CorePlugin;

class UserViewConfig : public UserViewConfigBase
{
    Q_OBJECT
public:
    UserViewConfig(QWidget *parent);
    ~UserViewConfig();
public slots:
    void apply();
    void colorsToggled(bool);
};

#endif

