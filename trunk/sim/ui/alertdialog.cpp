/***************************************************************************
                          alertdialog.cpp  -  description
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

#include "alertdialog.h"
#include "client.h"
#include "icons.h"
#include "fontedit.h"
#include "qcolorbutton.h"
#include "mainwin.h"
#include "xosd.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qcombobox.h>

AlertDialog::AlertDialog(QWidget *p, bool bReadOnly)
        : AlertDialogBase(p)
{
    if (bReadOnly){
        tabWnd->setCurrentPage(1);
        QWidget *page = tabWnd->currentPage();
        tabWnd->setCurrentPage(0);
        tabWnd->removePage(page);
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
        return;
    }
    chkOverride->hide();
    load(pClient->owner);
    chkOn->setChecked(pMain->isXOSD_on());
    spnOffs->setMinValue(0);
    spnOffs->setMaxValue(500);
    spnOffs->setValue(pMain->getXOSD_offset());
    spnTimeout->setMinValue(1);
    spnTimeout->setMaxValue(60);
    spnTimeout->setValue(pMain->getXOSD_timeout());
    btnColor->setColor(pMain->getXOSD_color());
    edtFont->setWinFont(pMain->xosd->font());
    cmbPos->insertItem(i18n("Left-bottom"));
    cmbPos->insertItem(i18n("Left-top"));
    cmbPos->insertItem(i18n("Right-bottom"));
    cmbPos->insertItem(i18n("Right-top"));
    cmbPos->insertItem(i18n("Center-bottom"));
    cmbPos->insertItem(i18n("Center-top"));
    cmbPos->setCurrentItem(pMain->getXOSD_pos());
    chkShadow->setChecked(pMain->isXOSD_Shadow());
    chkBackground->setChecked(pMain->isXOSD_Background());
    btnBgColor->setColor(pMain->getXOSD_BgColor());
    connect(chkOn, SIGNAL(toggled(bool)), this, SLOT(toggledOn(bool)));
    connect(chkBackground, SIGNAL(toggled(bool)), this, SLOT(toggledOn(bool)));
    toggledOn(chkOn->isChecked());
}

void AlertDialog::load(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    chkOverride->setChecked(u->AlertOverride);
    if (!u->AlertOverride) u = static_cast<SIMUser*>(pClient->owner);
    chkOnline->setChecked(u->AlertAway);
    chkBlink->setChecked(u->AlertBlink);
    chkSound->setChecked(u->AlertSound);
    chkOnScreen->setChecked(u->AlertOnScreen);
    chkDialog->setChecked(u->AlertPopup);
    chkFloat->setChecked(u->AlertWindow);
    chkLog->setChecked(u->LogStatus);
    overrideChanged(u->AlertOverride);
}

void AlertDialog::overrideChanged(bool bSet)
{
    if (chkOverride->isHidden()) bSet = true;
    chkOnline->setEnabled(bSet);
    chkBlink->setEnabled(bSet);
    chkSound->setEnabled(bSet);
    chkOnScreen->setEnabled(bSet);
    chkDialog->setEnabled(bSet);
    chkFloat->setEnabled(bSet);
    chkLog->setEnabled(bSet);
}

void AlertDialog::save(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    if (u->Uin != pClient->owner->Uin){
        u->AlertOverride = chkOverride->isChecked();
        if (!u->AlertOverride) return;
    }
    u->AlertAway = chkOnline->isChecked();
    u->AlertBlink = chkBlink->isChecked();
    u->AlertSound = chkSound->isChecked();
    u->AlertOnScreen = chkOnScreen->isChecked();
    u->AlertPopup = chkDialog->isChecked();
    u->AlertWindow = chkFloat->isChecked();
    u->LogStatus = chkLog->isChecked();
}

void AlertDialog::apply(ICQUser*)
{
    save(pClient->owner);
    pMain->setXOSD_on(chkOn->isChecked());
    pMain->setXOSD_offset(atol(spnOffs->text().latin1()));
    pMain->setXOSD_timeout(atol(spnTimeout->text().latin1()));
    pMain->setXOSD_color(btnColor->color().rgb() & 0xFFFFFF);
    pMain->setXOSD_pos(cmbPos->currentItem());
    pMain->setXOSD_Shadow(chkShadow->isChecked());
    pMain->setXOSD_Background(chkBackground->isChecked());
    pMain->setXOSD_BgColor(btnBgColor->color().rgb() & 0xFFFFFF);
    pMain->setXOSD_Font(pMain->font2str(edtFont->winFont(), false).local8Bit());
    pMain->xosd->init();
}

void AlertDialog::toggledOn(bool)
{
    bool bOn = chkOn->isChecked();
    bool bBg = chkBackground->isChecked();
    spnOffs->setEnabled(bOn);
    spnTimeout->setEnabled(bOn);
    btnColor->setEnabled(bOn);
    edtFont->setEnabled(bOn);
    cmbPos->setEnabled(bOn);
    chkShadow->setEnabled(bOn);
    chkBackground->setEnabled(bOn);
    btnBgColor->setEnabled(bOn & bBg);
}

#ifndef _WINDOWS
#include "alertdialog.moc"
#endif

