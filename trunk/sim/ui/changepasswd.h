/***************************************************************************
                          changepasswd.h  -  description
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

#ifndef _CHANGEPASSWD_H
#define _CHANGEPASSWD_H

#include "defs.h"
#include "changepasswdbase.h"

class ICQUser;

class ChangePasswd : public ChangePasswdBase
{
    Q_OBJECT
public:
    ChangePasswd(QWidget*);
public slots:
    void apply(ICQUser*);
};

#endif

