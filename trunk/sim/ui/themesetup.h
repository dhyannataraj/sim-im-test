/***************************************************************************
                          themesetup.h  -  description
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

#ifndef _THEMESETUP_H
#define _THEMESETUP_H

#include "defs.h"
#include "themesetupbase.h"

class QListBoxItem;
class ICQUser;

class ThemeSetup : public ThemeSetupBase
{
    Q_OBJECT
public:
    ThemeSetup(QWidget*);
public slots:
    void apply(ICQUser*);
protected slots:
    void addIcons();
    void checkedTransparent(bool);
    void apply(QListBoxItem*);
    void setupInit();
	void langChanged(int);
protected:
	bool bLangChanged;
};

#endif

