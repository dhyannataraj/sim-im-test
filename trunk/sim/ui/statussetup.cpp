/***************************************************************************
                          statussetup.cpp  -  description
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
    if (pMain->getAutoAwayTime()){
        chkAway->setChecked(true);
        spnAway->setValue(pMain->getAutoAwayTime() / 60);
    }else{
        spnAway->setValue(5);
        spnAway->setEnabled(false);
    }
    connect(chkAway, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    if (pMain->getAutoNATime()){
        chkNA->setChecked(true);
        spnNA->setValue(pMain->getAutoNATime() / 60);
    }else{
        spnNA->setValue(10);
        spnNA->setEnabled(false);
    }
    connect(chkNA, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    if (pMain->getAutoHideTime()){
        chkAutoHide->setChecked(true);
        spnAutoHide->setValue(pMain->getAutoHideTime());
    }else{
        spnAutoHide->setValue(60);
        spnAutoHide->setEnabled(false);
    }
    connect(chkAutoHide, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    chkAutoHide->setEnabled(pMain->isDock());
    chkAlert->setChecked(pMain->isNoAlertAway());
}

void StatusSetup::changed(bool)
{
    spnAway->setEnabled(chkAway->isChecked());
    spnNA->setEnabled(chkNA->isChecked());
    spnAutoHide->setEnabled(chkAutoHide->isChecked());
}

void StatusSetup::apply(ICQUser*)
{
    pMain->setAutoAwayTime(chkAway->isChecked() ? atol(spnAway->text().latin1()) * 60 : 0);
    pMain->setAutoNATime(chkNA->isChecked() ? atol(spnNA->text().latin1()) * 60 : 0);
    pMain->setAutoHideTime(chkAutoHide->isChecked() ? atol(spnAutoHide->text().latin1()) : 0);
    pMain->setNoAlertAway(chkAlert->isChecked());
}

#ifndef _WINDOWS
#include "statussetup.moc"
#endif

