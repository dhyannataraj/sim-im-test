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

void AlertDialog::load(ICQUser *u)
{
    load(pClient->getSettings(u, offsetof(UserSettings, AlertOverride)));
}

void AlertDialog::load(ICQGroup *g)
{
    load(pClient->getSettings(g, offsetof(UserSettings, AlertOverride)));
}

void AlertDialog::load(UserSettings *settings)
{
    chkOverride->setChecked(settings->AlertOverride);
    chkOnline->setChecked(settings->AlertAway);
    chkBlink->setChecked(settings->AlertBlink);
    chkSound->setChecked(settings->AlertSound);
    chkOnScreen->setChecked(settings->AlertOnScreen);
    chkDialog->setChecked(settings->AlertPopup);
    chkFloat->setChecked(settings->AlertWindow);
    chkLog->setChecked(settings->LogStatus);
    overrideChanged(settings->AlertOverride);
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
    if (_u->Uin != pClient->owner->Uin){
        SIMUser *u = static_cast<SIMUser*>(_u);
        u->settings.AlertOverride = chkOverride->isChecked();
        if (!u->settings.AlertOverride) return;
    }
    save(pClient->getSettings(_u, offsetof(UserSettings, AlertOverride)));
}

void AlertDialog::save(ICQGroup *_g)
{
    SIMGroup *g = static_cast<SIMGroup*>(_g);
    g->settings.AlertOverride = chkOverride->isChecked();
    if (!g->settings.AlertOverride) return;
    save(&g->settings);
}

void AlertDialog::save(UserSettings *settings)
{
    settings->AlertAway = chkOnline->isChecked();
    settings->AlertBlink = chkBlink->isChecked();
    settings->AlertSound = chkSound->isChecked();
    settings->AlertOnScreen = chkOnScreen->isChecked();
    settings->AlertPopup = chkDialog->isChecked();
    settings->AlertWindow = chkFloat->isChecked();
    settings->LogStatus = chkLog->isChecked();
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

