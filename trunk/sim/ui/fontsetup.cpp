/***************************************************************************
                          fontsetup.cpp  -  description
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

FontSetup::FontSetup(QWidget *p)
        : FontSetupBase(p)
{
    lblPict->setPixmap(Pict("text"));
    edtFont->setWinFont(qApp->font());
    QPopupMenu popup(this);
    edtFontMenu->setWinFont(qApp->font(&popup));
#ifdef USE_KDE
    chkSystem->setChecked(pMain->UseSystemFonts());
    systemToggled(chkSystem->isChecked());
    connect(chkSystem, SIGNAL(toggled(bool)), this, SLOT(systemToggled(bool)));
#else
    chkSystem->hide();
#endif
    chkOwnColors->setChecked(pMain->UseOwnColors());
    grpContainer->setButton(pMain->ContainerMode());
    btnSend->setColor(QColor(pMain->ColorSend));
    btnReceive->setColor(QColor(pMain->ColorReceive));
    grpWndMode->setButton(pMain->SimpleMode() ? 1 : 0);
	chkSplash->setChecked(pSplash->Show());
}

void FontSetup::systemToggled(bool)
{
#ifdef USE_KDE
    edtFont->setEnabled(!chkSystem->isChecked());
    edtFontMenu->setEnabled(!chkSystem->isChecked());
#endif
}

void FontSetup::apply(ICQUser*)
{
    unsigned long colorSend = btnSend->color().rgb() & 0xFFFFFF;
    unsigned long colorReceive = btnReceive->color().rgb() & 0xFFFFFF;
    if ((pMain->ColorSend() != colorSend) || (pMain->ColorReceive() != colorReceive)){
        pMain->ColorSend = colorSend;
        pMain->ColorReceive = colorReceive;
        pMain->changeColors();
    }
#ifdef USE_KDE
    pMain->UseSystemFonts = chkSystem->isChecked();
    if (!pMain->UseSystemFonts()){
#endif
        const QFont &fontWin = edtFont->winFont();
        const QFont &fontMenu = edtFontMenu->winFont();
        pMain->FontFamily = fontWin.family();
        pMain->FontSize = fontWin.pointSize();
        pMain->FontWeight = fontWin.weight();
        pMain->FontItalic = fontWin.italic();
        pMain->FontMenuFamily = fontMenu.family();
        pMain->FontMenuSize = fontMenu.pointSize();
        pMain->FontMenuWeight = fontMenu.weight();
        pMain->FontMenuItalic = fontMenu.italic();
#ifdef USE_KDE
    }
#endif
    pMain->changeOwnColors(chkOwnColors->isChecked());
    pMain->setFonts();
    QWidget *w = grpContainer->selected();
    if (w == rbtUser)
        pMain->ContainerMode = ContainerModeUser;
    if (w == rbtGroup)
        pMain->ContainerMode = ContainerModeGroup;
    if (w == rbtAll)
        pMain->ContainerMode = ContainerModeAll;
    pMain->changeMode(grpWndMode->selected() == btnModePlain);
	pSplash->Show = chkSplash->isChecked();
}

#ifndef _WINDOWS
#include "fontsetup.moc"
#endif

