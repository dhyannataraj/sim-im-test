/***************************************************************************
                          pageretails.h  -  description
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

#ifndef _PAGERDETAILS_H
#define _PAGERDETAILS_H

#include "defs.h"
#include "pagerbase.h"

class PhoneInfo;

class PagerDetails : public PagerDetailsBase
{
    Q_OBJECT
public:
    PagerDetails(QWidget *p, PhoneInfo *info);
    void getNumber();
    void fillInfo(PhoneInfo *info);
signals:
    void numberChanged(const QString&, bool);
protected slots:
    void providerChanged(const QString&);
    void textChanged(const QString&);
};

#endif

