/***************************************************************************
                          addphone.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADDPHONE_H
#define _ADDPHONE_H

#include "defs.h"
#include "addphonebase.h"

class PhoneDetails;
class PagerDetails;
class PhoneInfo;

typedef struct phoneName
{
    const char *name;
    unsigned type;
    unsigned index;
} phoneName;

extern const phoneName *phoneNames;

class AddPhone : public AddPhoneBase
{
    Q_OBJECT
public:
    AddPhone(QWidget *p, PhoneInfo *info, int country, bool myPhones);
protected slots:
    void ok();
    void nameChanged(const QString&);
    void typeChanged(int);
    void numberChanged(const QString&, bool);
protected:
    bool bMyInfo;
    PhoneDetails *phone;
    PagerDetails *pager;
    PhoneInfo *info;
};

#endif

