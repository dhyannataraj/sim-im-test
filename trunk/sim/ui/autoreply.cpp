/***************************************************************************
                          autoreply.cpp  -  description
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

#include "autoreply.h"
#include "client.h"
#include "mainwin.h"
#include "icons.h"

#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtimer.h>

AutoReplyDlg::AutoReplyDlg(QWidget *p, unsigned long _status)
        : AutoreplyBase(p), status(_status)
{
    setIcon(Pict(Client::getStatusIcon(status)));
    setWFlags(WDestructiveClose);
    connect(btnOK, SIGNAL(clicked()), this, SLOT(apply()));
    timeLeft = 15;
    tick();
    timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    switch (status){
    case ICQ_STATUS_AWAY:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseAway.c_str()));
        break;
    case ICQ_STATUS_NA:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseNA.c_str()));
        break;
    case ICQ_STATUS_OCCUPIED:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseOccupied.c_str()));
        break;
    case ICQ_STATUS_DND:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseDND.c_str()));
        break;
    case ICQ_STATUS_FREEFORCHAT:
        edtMessage->setText(QString::fromLocal8Bit(pClient->AutoResponseFFC.c_str()));
        break;
    }
    connect(edtMessage, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(chkNoShow, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
}

void AutoReplyDlg::toggled(bool)
{
    textChanged();
}

void AutoReplyDlg::textChanged()
{
    if (timeLeft < 0) return;
    lblTimer->setText("");
    timeLeft = -1;
    timer->stop();
}

void AutoReplyDlg::tick()
{
    if (timeLeft == 0){
        close();
        return;
    }
    if (timeLeft < 0) return;
    lblTimer->setText(i18n("Close in %1 sec") .arg(QString::number(timeLeft)));
    timeLeft--;
}

void AutoReplyDlg::apply()
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
    close();
}

#ifndef _WINDOWS
#include "autoreply.moc"
#endif

