/***************************************************************************
                          cuser.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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
    QString name(bool quoted=false);
    QString firstName(bool quoted=false);
    QString lastName(bool quoted=false);
    QString email(bool quoted=false);
    QString autoReply(bool quoted=false);
    QString addr();
    QString realAddr();
    QString statusTime();
    QString onlineTime();
    QString client();
    QString toolTip();
    ICQUser *operator -> () { return u; }
protected:
    QString formatTime(unsigned long time);
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

