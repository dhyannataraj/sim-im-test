/***************************************************************************
                          statussetup.cpp  -  description
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

#include "statussetup.h"
#include "mainwin.h"
#include "icons.h"

#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qpixmap.h>

StatusSetup::StatusSetup(QWidget *p)
        : StatusSetupBase(p)
{
    lblPict->setPixmap(Pict("status"));
    if (pMain->AutoAwayTime){
        chkAway->setChecked(true);
        spnAway->setValue(pMain->AutoAwayTime / 60);
    }else{
        spnAway->setValue(5);
        spnAway->setEnabled(false);
    }
    connect(chkAway, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    if (pMain->AutoNATime){
        chkNA->setChecked(true);
        spnNA->setValue(pMain->AutoNATime / 60);
    }else{
        spnNA->setValue(10);
        spnNA->setEnabled(false);
    }
    connect(chkNA, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
}

void StatusSetup::changed(bool)
{
    spnAway->setEnabled(chkAway->isChecked());
    spnNA->setEnabled(chkNA->isChecked());
}

void StatusSetup::apply(ICQUser*)
{
    pMain->AutoAwayTime = chkAway->isChecked() ? spnAway->value() * 60 : 0;
    pMain->AutoNATime = chkNA->isChecked() ? spnNA->value() * 60 : 0;
}

#ifndef _WINDOWS
#include "statussetup.moc"
#endif

