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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qmultilineedit.h>

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
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseAway.c_str()));
        chkNoShow->setChecked(pMain->NoShowAway());
        break;
    case ICQ_STATUS_NA:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseNA.c_str()));
        chkNoShow->setChecked(pMain->NoShowNA());
        break;
    case ICQ_STATUS_DND:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseDND.c_str()));
        chkNoShow->setChecked(pMain->NoShowDND());
        break;
    case ICQ_STATUS_OCCUPIED:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseOccupied.c_str()));
        chkNoShow->setChecked(pMain->NoShowOccupied());
        break;
    case ICQ_STATUS_FREEFORCHAT:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseFFC.c_str()));
        chkNoShow->setChecked(pMain->NoShowFFC());
        break;
    }
    connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
}

void MsgDialog::load(ICQUser *u)
{
    switch (status){
    case ICQ_STATUS_AWAY:
        if (!*u->AutoResponseAway.c_str()) break;
        edtMessage->setText(QString::fromLocal8Bit(u->AutoResponseAway.c_str()));
        chkOverride->setChecked(true);
        break;
    case ICQ_STATUS_NA:
        if (!*u->AutoResponseNA.c_str()) break;
        edtMessage->setText(QString::fromLocal8Bit(u->AutoResponseNA.c_str()));
        chkOverride->setChecked(true);
        break;
    case ICQ_STATUS_DND:
        if (!*u->AutoResponseDND.c_str()) break;
        edtMessage->setText(QString::fromLocal8Bit(u->AutoResponseDND.c_str()));
        chkOverride->setChecked(true);
        break;
    case ICQ_STATUS_OCCUPIED:
        if (!*u->AutoResponseOccupied.c_str()) break;
        edtMessage->setText(QString::fromLocal8Bit(u->AutoResponseOccupied.c_str()));
        chkOverride->setChecked(true);
        break;
    case ICQ_STATUS_FREEFORCHAT:
        if (!*u->AutoResponseFFC.c_str()) break;
        edtMessage->setText(QString::fromLocal8Bit(u->AutoResponseFFC.c_str()));
        chkOverride->setChecked(true);
        break;
    }
    overrideChanged(chkOverride->isChecked());
}

void MsgDialog::save(ICQUser *u)
{
    const char *res = edtMessage->text().local8Bit();
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
        pClient->AutoResponseAway = edtMessage->text().local8Bit();
        pMain->NoShowAway = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_NA:
        pClient->AutoResponseNA = edtMessage->text().local8Bit();
        pMain->NoShowNA = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_DND:
        pClient->AutoResponseDND = edtMessage->text().local8Bit();
        pMain->NoShowDND = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_OCCUPIED:
        pClient->AutoResponseOccupied = edtMessage->text().local8Bit();
        pMain->NoShowOccupied = chkNoShow->isChecked();
        break;
    case ICQ_STATUS_FREEFORCHAT:
        pClient->AutoResponseFFC = edtMessage->text().local8Bit();
        pMain->NoShowFFC = chkNoShow->isChecked();
        break;
    }
}

#ifndef _WINDOWS
#include "msgdialog.moc"
#endif

