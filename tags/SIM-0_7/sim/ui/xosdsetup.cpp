/***************************************************************************
                          xosdsetup.cpp  -  description
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

#include "xosdsetup.h"
#include "icons.h"
#include "mainwin.h"
#include "xosd.h"
#include "qcolorbutton.h"
#include "fontedit.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qcombobox.h>

XOSDSetup::XOSDSetup(QWidget *p)
        : XOSDSetupBase(p)
{
    lblPict->setPixmap(Pict("screen"));
    chkOn->setChecked(pMain->XOSD_on);
    spnOffs->setMinValue(0);
    spnOffs->setMaxValue(500);
    spnOffs->setValue(pMain->XOSD_offset());
    spnTimeout->setMinValue(1);
    spnTimeout->setMaxValue(60);
    spnTimeout->setValue(pMain->XOSD_timeout());
    btnColor->setColor(pMain->XOSD_color());
    edtFont->setWinFont(pMain->xosd->font());
    cmbPos->insertItem(i18n("Left-bottom"));
    cmbPos->insertItem(i18n("Left-top"));
    cmbPos->insertItem(i18n("Right-bottom"));
    cmbPos->insertItem(i18n("Right-top"));
    cmbPos->insertItem(i18n("Center-bottom"));
    cmbPos->insertItem(i18n("Center-top"));
    cmbPos->setCurrentItem(pMain->XOSD_pos());
    chkShadow->setChecked(pMain->XOSD_Shadow());
    chkBackground->setChecked(pMain->XOSD_Background());
    btnBgColor->setColor(pMain->XOSD_BgColor());
    connect(chkOn, SIGNAL(toggled(bool)), this, SLOT(toggledOn(bool)));
    connect(chkBackground, SIGNAL(toggled(bool)), this, SLOT(toggledOn(bool)));
    toggledOn(chkOn->isChecked());
}

void XOSDSetup::apply(ICQUser*)
{
    pMain->XOSD_on = chkOn->isChecked();
    pMain->XOSD_offset = spnOffs->value();
    pMain->XOSD_timeout = spnTimeout->value();
    pMain->XOSD_color = btnColor->color().rgb() & 0xFFFFFF;
    pMain->XOSD_pos = cmbPos->currentItem();
    pMain->XOSD_Shadow = chkShadow->isChecked();
    pMain->XOSD_Background = chkBackground->isChecked();
    pMain->XOSD_BgColor = btnBgColor->color().rgb() & 0xFFFFFF;
    const QFont &f = edtFont->winFont();
    pMain->XOSD_FontFamily = f.family();
    pMain->XOSD_FontSize = f.pointSize();
    pMain->XOSD_FontWeight = f.weight();
    pMain->XOSD_FontItalic = f.italic();
    pMain->xosd->init();
}

void XOSDSetup::toggledOn(bool)
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
#include "xosdsetup.moc"
#endif

