/***************************************************************************
                          fontsetup.cpp  -  description
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

#include "fontsetup.h"
#include "fontedit.h"
#include "icons.h"
#include "mainwin.h"
#include "splash.h"
#include "qcolorbutton.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>

FontSetup::FontSetup(QWidget *p)
        : FontSetupBase(p)
{
    edtFont->setWinFont(qApp->font());
    QPopupMenu popup(this);
    edtFontMenu->setWinFont(qApp->font(&popup));
#ifdef USE_KDE
    chkSystem->setChecked(pMain->isUseSystemFonts());
    systemToggled(chkSystem->isChecked());
    connect(chkSystem, SIGNAL(toggled(bool)), this, SLOT(systemToggled(bool)));
#else
    chkSystem->hide();
#endif
    chkOwnColors->setChecked(pMain->isUseOwnColors());
    grpContainer->setButton(pMain->getContainerMode());
    btnSend->setColor(QColor(pMain->getColorSend()));
    btnReceive->setColor(QColor(pMain->getColorReceive()));
    btnSender->setColor(QColor(pMain->getColorSender()));
    btnReceiver->setColor(QColor(pMain->getColorReceiver()));
    grpWndMode->setButton(pMain->isSimpleMode() ? 1 : 0);
    spnCopy->setMinValue(0);
    spnCopy->setMaxValue(9);
    spnCopy->setValue(pMain->getCopyMessages());
    connect(btnModeSplit, SIGNAL(toggled(bool)), this, SLOT(modeChanged(bool)));
    connect(btnModePlain, SIGNAL(toggled(bool)), this, SLOT(modeChanged(bool)));
    connect(chkOwnColors, SIGNAL(toggled(bool)), this, SLOT(useOwnColorsChanged(bool)));
    modeChanged(false);
    useOwnColorsChanged(pMain->isUseOwnColors());
}

void FontSetup::systemToggled(bool)
{
#ifdef USE_KDE
    edtFont->setEnabled(!chkSystem->isChecked());
    edtFontMenu->setEnabled(!chkSystem->isChecked());
#endif
}

void FontSetup::modeChanged(bool)
{
    grpContainer->setEnabled(btnModeSplit->isOn());
    spnCopy->setEnabled(btnModeSplit->isOn());
}

void FontSetup::useOwnColorsChanged(bool AToggled)
{
	btnSend->setEnabled(AToggled);
	btnReceive->setEnabled(AToggled);
	btnSender->setEnabled(AToggled);
	btnReceiver->setEnabled(AToggled);
}

void FontSetup::apply(ICQUser*)
{
    unsigned long colorSend = btnSend->color().rgb() & 0xFFFFFF;
    unsigned long colorReceive = btnReceive->color().rgb() & 0xFFFFFF;
	unsigned long colorSender = btnSender->color().rgb() & 0xFFFFFF;
	unsigned long colorReceiver = btnReceiver->color().rgb() & 0xFFFFFF;
	if ((pMain->getColorSend() != colorSend)     || (pMain->getColorReceive() != colorReceive) ||
		(pMain->getColorSender() != colorSender) || (pMain->getColorReceiver() != colorReceiver))
    {
        pMain->setColorSend(colorSend);
        pMain->setColorReceive(colorReceive);
        pMain->setColorSender(colorSender);
        pMain->setColorReceiver(colorReceiver);
        pMain->changeColors();
    }
#ifdef USE_KDE
    pMain->setUseSystemFonts(chkSystem->isChecked());
    if (!pMain->isUseSystemFonts()){
#endif
        pMain->setFont(pMain->font2str(edtFont->winFont(), false).local8Bit());
        pMain->setFontMenu(pMain->font2str(edtFontMenu->winFont(), false).local8Bit());
#ifdef USE_KDE
    }
#endif
    pMain->changeOwnColors(chkOwnColors->isChecked());
    pMain->setFonts();
    QWidget *w = grpContainer->selected();
    if (w == rbtUser)
        pMain->setContainerMode(ContainerModeUser);
    if (w == rbtGroup)
        pMain->setContainerMode(ContainerModeGroup);
    if (w == rbtAll)
        pMain->setContainerMode(ContainerModeAll);
    pMain->changeMode(grpWndMode->selected() == btnModePlain);
    pMain->setCopyMessages(atol(spnCopy->text().latin1()));
}

#ifndef _WINDOWS
#include "fontsetup.moc"
#endif

