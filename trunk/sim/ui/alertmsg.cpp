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

#include "alertmsg.h"
#include "client.h"
#include "cuser.h"
#include "icons.h"
#include "enable.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>

AlertMsgDlg::AlertMsgDlg(QWidget *p, unsigned long uin)
        : AlertMsgBase(p)
{
    setButtonsPict(this);
    setIcon(Pict(SIMClient::getStatusIcon(ICQ_STATUS_OFFLINE)));
    setWFlags(WDestructiveClose);
    connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
    timeLeft = 15;
    tick();
    timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    CUser u(uin);
    lblInfo->setText(i18n("User ") + u.name() + i18n(" is online"));
}

void AlertMsgDlg::tick()
{
    if (timeLeft == 0){
        close();
        return;
    }
    if (timeLeft < 0) return;
    btnOK->setText(i18n("Close [%1 sec]") .arg(QString::number(timeLeft)));
    timeLeft--;
}

#ifndef _WINDOWS
#include "alertmsg.moc"
#endif

