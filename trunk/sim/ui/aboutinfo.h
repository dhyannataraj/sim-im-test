/***************************************************************************
                          aboutinfo.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _ABOUTINFO_H
#define _ABOUTINFO_H

#include "defs.h"
#include "aboutinfobase.h"

class ICQUser;

class AboutInfo : public AboutInfoBase
{
    Q_OBJECT
public:
    AboutInfo(QWidget *p, bool readOnly=false);
public slots:
    void apply(ICQUser *u);
    void load(ICQUser *u);
    void save(ICQUser *u);
};

#endif

