/***************************************************************************
                          alertdialog.h  -  description
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

#ifndef _ALERTDIALOG_H
#define _ALERTDIALOG_H

#include "defs.h"
#include "alertdialogbase.h"

class ICQUser;

class AlertDialog : public AlertDialogBase
{
    Q_OBJECT
public:
    AlertDialog(QWidget *p, bool bReadOnly=false);
public slots:
    void apply(ICQUser *u);
    void load(ICQUser *u);
    void save(ICQUser *u);
protected slots:
    void overrideChanged(bool bSet);
};

#endif

