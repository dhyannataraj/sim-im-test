/***************************************************************************
                          soundsetup.h  -  description
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

#ifndef _SOUNDSETUP_H
#define _SOUNDSETUP_H

#include "defs.h"
#include "soundsetupbase.h"

#include <string>
using namespace std;

class ICQUser;
class EditSound;

class SoundSetup : public SoundSetupBase
{
    Q_OBJECT
public:
    SoundSetup(QWidget *p, bool bUser=false);
public slots:
    void load(ICQUser*);
    void save(ICQUser*);
    void apply(ICQUser*);
protected slots:
    void overrideToggled(bool);
    void artsToggled(bool);
    void disableToggled(bool);
protected:
    string sound(EditSound *edt);
};

#endif

