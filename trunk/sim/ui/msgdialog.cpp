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

#include "msgdialog.h"
#include "client.h"
#include "mainwin.h"
#include "icons.h"
#include "enable.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qmultilineedit.h>

static void set(QMultiLineEdit *edit, const string &s, ICQUser *u)
{
    edit->setText(pClient->from8Bit(u->Uin, s.c_str()));
}

MsgDialog::MsgDialog(QWidget *p, unsigned long _status, bool bReadOnly)
        : MsgDialogBase(p), status(_status)
{
    lblPict->setPixmap(Pict(Client::getStatusIcon(status)));
    lblHeader->setText(i18n("Autoreply message for status \"%1\"") .arg(Client::getStatusText(status)));
    if (bReadOnly){
        chkNoShow->hide();
    }else{
        chkOverride->hide();
    }
    switch (status){
    case ICQ_STATUS_AWAY:
        set(edtMessage, pClient->owner->AutoResponseAway, pClient->owner);
        chkNoShow->setChecked(pMain->NoShowAway);
        break;
    case ICQ_STATUS_NA:
        set(edtMessage, pClient->owner->AutoResponseNA, pClient->owner);
        chkNoShow->setChecked(pMain->NoShowNA);
        break;
    case ICQ_STATUS_DND:
        set(edtMessage, pClient->owner->AutoResponseDND, pClient->owner);
        chkNoShow->setChecked(pMain->NoShowDND);
        break;
    case ICQ_STATUS_OCCUPIED:
        set(edtMessage, pClient->owner->AutoResponseOccupied, pClient->owner);
        chkNoShow->setChecked(pMain->NoShowOccupied);
        break;
    case ICQ_STATUS_FREEFORCHAT:
        set(edtMessage, pClient->owner->AutoResponseFFC, pClient->owner);
        chkNoShow->setChecked(pMain->NoShowFFC);
        break;
    }
    connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
}

void MsgDialog::setup(ICQUser *u, const string &str1, const string &str2)
{
    string str;
    ICQUser *user;
    if (*(str1.c_str()) == 0){
        chkOverride->setChecked(false);
        str = str2;
	user = pClient->owner;
    }else{
        chkOverride->setChecked(true);
        str = str1;
	user = u;
    }
    set(edtMessage, str, user);
}

void MsgDialog::load(ICQUser *u)
{
    switch (status){
    case ICQ_STATUS_AWAY:
        setup(u, u->AutoResponseAway, pClient->owner->AutoResponseAway);
        break;
    case ICQ_STATUS_NA:
        setup(u, u->AutoResponseNA, pClient->owner->AutoResponseNA);
        break;
    case ICQ_STATUS_DND:
        setup(u, u->AutoResponseDND, pClient->owner->AutoResponseDND);
        break;
    case ICQ_STATUS_OCCUPIED:
        setup(u, u->AutoResponseOccupied, pClient->owner->AutoResponseOccupied);
        break;
    case ICQ_STATUS_FREEFORCHAT:
        setup(u, u->AutoResponseFFC, pClient->owner->AutoResponseFFC);
        break;
    }
    overrideChanged(chkOverride->isChecked());
}

void MsgDialog::save(ICQUser *u)
{
    string res;
    set(res, edtMessage->text());
    if (!chkOverride->isChecked()) res = "";
    switch (status){
    case ICQ_STATUS_AWAY:
        u->AutoResponseAway = res;
        break;
    case ICQ_STATUS_NA:
        u->AutoResponseNA = res;
        break;
    case ICQ_STATUS_DND:
        u->AutoResponseDND = res;
        break;
    case ICQ_STATUS_OCCUPIED:
        u->AutoResponseOccupied = res;
        break;
    case ICQ_STATUS_FREEFORCHAT:
        u->AutoResponseFFC = res;
        break;
    }
}

void MsgDialog::overrideChanged(bool bOverride)
{
    edtMessage->setReadOnly(!bOverride);
}

void MsgDialog::apply(ICQUser*)
{
    switch (status){
    case ICQ_STATUS_AWAY:
        set(pClient->owner->AutoResponseAway, edtMessage->text());
        pMain->NoShowAway = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_NA:
        set(pClient->owner->AutoResponseNA, edtMessage->text());
        pMain->NoShowNA = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_DND:
        set(pClient->owner->AutoResponseDND, edtMessage->text());
        pMain->NoShowDND = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_OCCUPIED:
        set(pClient->owner->AutoResponseOccupied, edtMessage->text());
        pMain->NoShowOccupied = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_FREEFORCHAT:
        set(pClient->owner->AutoResponseFFC, edtMessage->text());
        pMain->NoShowFFC = chkNoShow->isChecked();
        break;
    }
}

#ifndef _WINDOWS
#include "msgdialog.moc"
#endif

