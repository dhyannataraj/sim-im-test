/***************************************************************************
                          phonedetails.h  -  description
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

#ifndef _PHONEDETAILS_H
#define _PHONEDETAILS_H

#include "defs.h"
#include "phonebase.h"

class PhoneInfo;

class PhoneDetails : public PhoneDetailsBase
{
    Q_OBJECT
public:
    PhoneDetails(QWidget *p, PhoneInfo *info, unsigned userCountry);
    void setPublishShow(bool bShow);
    void setExtensionShow(bool bShow);
    void setSMSShow(bool bShow);
    void getNumber();
    void fillInfo(PhoneInfo *info);
signals:
    void numberChanged(const QString &str, bool bOK);
protected slots:
    void countryChanged(int);
    void textChanged(const QString&);
};

#endif

