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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>

AlertDialog::AlertDialog(QWidget *p, bool bReadOnly)
        : AlertDialogBase(p)
{
    lblPict->setPixmap(Pict("alert"));
    if (bReadOnly){
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
        return;
    }
    chkOverride->hide();
    load(pClient->owner);
}

void AlertDialog::load(ICQUser *u)
{
    chkOverride->setChecked(u->AlertOverride);
    if (!u->AlertOverride) u = pClient->owner;
    chkOnline->setChecked(u->AlertAway);
    chkBlink->setChecked(u->AlertBlink);
    chkSound->setChecked(u->AlertSound);
    chkOnScreen->setChecked(u->AlertOnScreen);
    chkDialog->setChecked(u->AlertPopup);
    chkFloat->setChecked(u->AlertWindow);
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
}

void AlertDialog::save(ICQUser *u)
{    if (u->Uin != pClient->owner->Uin){
        u->AlertOverride = chkOverride->isChecked();
        if (!u->AlertOverride) return;
    }
    u->AlertAway = chkOnline->isChecked();
    u->AlertBlink = chkBlink->isChecked();
    u->AlertSound = chkSound->isChecked();
    u->AlertOnScreen = chkOnScreen->isChecked();
    u->AlertPopup = chkDialog->isChecked();
    u->AlertWindow = chkFloat->isChecked();
}

void AlertDialog::apply(ICQUser*)
{
    save(pClient->owner);
}

#ifndef _WINDOWS
#include "alertdialog.moc"
#endif

