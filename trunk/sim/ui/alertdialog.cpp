/***************************************************************************
                          alertdialog.cpp  -  description
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
    chkOn->setChecked(pMain->XOSD_on);
    spnOffs->setMinValue(0);
    spnOffs->setMaxValue(500);
    spnOffs->setValue(pMain->XOSD_offset);
    spnTimeout->setMinValue(1);
    spnTimeout->setMaxValue(60);
    spnTimeout->setValue(pMain->XOSD_timeout);
    btnColor->setColor(pMain->XOSD_color);
    edtFont->setWinFont(pMain->xosd->font());
    cmbPos->insertItem(i18n("Left-bottom"));
    cmbPos->insertItem(i18n("Left-top"));
    cmbPos->insertItem(i18n("Right-bottom"));
    cmbPos->insertItem(i18n("Right-top"));
    cmbPos->insertItem(i18n("Center-bottom"));
    cmbPos->insertItem(i18n("Center-top"));
    cmbPos->setCurrentItem(pMain->XOSD_pos);
    chkShadow->setChecked(pMain->XOSD_Shadow);
    chkBackground->setChecked(pMain->XOSD_Background);
    btnBgColor->setColor(pMain->XOSD_BgColor);
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
    pMain->XOSD_on = chkOn->isChecked();
    pMain->XOSD_offset = atol(spnOffs->text().latin1());
    pMain->XOSD_timeout = atol(spnTimeout->text().latin1());
    pMain->XOSD_color = btnColor->color().rgb() & 0xFFFFFF;
    pMain->XOSD_pos = cmbPos->currentItem();
    pMain->XOSD_Shadow = chkShadow->isChecked();
    pMain->XOSD_Background = chkBackground->isChecked();
    pMain->XOSD_BgColor = btnBgColor->color().rgb() & 0xFFFFFF;
    pMain->XOSD_Font = pMain->font2str(edtFont->winFont(), false).local8Bit();
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

