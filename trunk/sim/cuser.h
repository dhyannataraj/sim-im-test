/***************************************************************************
                          cuser.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CUSER_H
#define _CUSER_H

#include "defs.h"

#include <qstring.h>

class ICQUser;
class ICQGroup;

class CUser
{
public:
    CUser(unsigned long uin);
    CUser(ICQUser *u);
    const QString name(bool quoted=false);
    const QString firstName(bool quoted=false);
    const QString lastName(bool quoted=false);
    const QString email(bool quoted=false);
    const QString autoReply(bool quoted=false);
    const QString addr();
    const QString statusTime();
    const QString onlineTime();
    QString client();
    QString toolTip();
    ICQUser *operator -> () { return u; }
protected:
    const QString formatTime(unsigned long time);
    ICQUser *u;
    unsigned long mUIN;
};

class CGroup
{
public:
    CGroup(ICQGroup *g);
    QString name();
    ICQGroup *operator -> () { return g; }
protected:
    ICQGroup *g;
};

#endif

