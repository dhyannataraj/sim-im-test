/***************************************************************************
                          fontsetup.h  -  description
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

#ifndef _FONTSETUP_H
#define _FONTSETUP_H

#include "defs.h"
#include "fontsetupbase.h"

class ICQUser;

class FontSetup : public FontSetupBase
{
    Q_OBJECT
public:
    FontSetup(QWidget *p);
public slots:
    void apply(ICQUser*);
protected slots:
    void systemToggled(bool);
    void modeChanged(bool);
    void useOwnColorsChanged(bool);
};

#endif

