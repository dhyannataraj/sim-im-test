/***************************************************************************
                          userautoreply.h  -  description
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

#ifndef _USERAUTOREPLY_H
#define _USERAUTOREPLY_H

#include "defs.h"
#include "autoreplybase.h"

class ICQEvent;

class UserAutoReplyDlg : public AutoreplyBase
{
    Q_OBJECT
public:
    UserAutoReplyDlg(unsigned long uin);
    unsigned long uin;
protected slots:
    void click();
    void start();
    void processEvent(ICQEvent*);
protected:
    void setStatus();
    bool bProcess;
};

#endif

