/***************************************************************************
                          keysetup.h  -  description
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

#ifndef _KEYSETUP_H
#define _KEYSETUP_H

#include "defs.h"
#include "keysetupbase.h"

class QWidget;
class ICQUser;
class QListViewItem;

class KeySetup : public KeySetupBase
{
    Q_OBJECT
public:
    KeySetup(QWidget*);
public slots:
    void apply(ICQUser*);
protected slots:
    void selectionChanged(QListViewItem*);
    void enableToggled(bool);
    void keyChanged();
};

#endif

