/***************************************************************************
                          moreinfo.h  -  description
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

#ifndef _MOREINFO_H
#define _MOREINFO_H

#include "defs.h"
#include "moreinfobase.h"

class ICQUser;
class QComboBox;

class MoreInfo : public MoreInfoBase
{
    Q_OBJECT
public:
    MoreInfo(QWidget *p, bool readOnly=false);
public slots:
    void apply(ICQUser *u);
    void load(ICQUser *u);
    void save(ICQUser *u);
protected slots:
    void birthDayChanged(int);
    void setLang(int);
    void goUrl();
};

#endif

