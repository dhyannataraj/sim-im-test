/***************************************************************************
                          workinfo.h  -  description
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

#ifndef _WORKINFO_H
#define _WORKINFO_H

#include "defs.h"
#include "workinfobase.h"

class ICQUser;

class WorkInfo : public WorkInfoBase
{
    Q_OBJECT
public:
    WorkInfo(QWidget *p, bool readOnly=false);
public slots:
    void apply(ICQUser*);
    void load(ICQUser*);
    void save(ICQUser*);
protected slots:
    void goUrl();
};

#endif

