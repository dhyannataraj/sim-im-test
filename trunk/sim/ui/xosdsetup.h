/***************************************************************************
                          xosdsetup.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _XOSDSETUP_H
#define _XOSDSETUP_H

#include "defs.h"
#include "xosdbase.h"

class ICQUser;

class XOSDSetup : public XOSDSetupBase
{
    Q_OBJECT
public:
    XOSDSetup(QWidget*);
public slots:
    void apply(ICQUser*);
protected slots:
    void toggledOn(bool);
};

#endif

