/***************************************************************************
                          securedlg.cpp  -  description
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

#include "securedlg.h"
#include "icons.h"
#include "client.h"
#include "enable.h"
#include "cuser.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtimer.h>

SecureDlg::SecureDlg(QWidget *parent, unsigned long uin)
        : SecureDlgBase(parent, "sceuredlg", false, WDestructiveClose)
{
    SET_WNDPROC
    setButtonsPict(this);
    Uin = uin;
    CUser user(Uin);
    setCaption(user.name() + " - " + caption());
    setIcon(Pict("encrypted"));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QTimer::singleShot(10, this, SLOT(sendRequest()));
}

void SecureDlg::sendRequest()
{
    ICQUser *u = pClient->getUser(Uin);
    if (u) pClient->requestSecureChannel(u);
}

void SecureDlg::processEvent(ICQEvent *e)
{
    if ((e->type() == EVENT_STATUS_CHANGED) && (e->Uin() == Uin)){
        ICQUser *u = pClient->getUser(Uin);
        if (u && u->isSecure())
            close();
        return;
    }
    if (e->type() != EVENT_MESSAGE_SEND) return;
    if ((e->message() == NULL) ||
            (e->message()->Type() != ICQ_MSGxSECURExOPEN) ||
            (e->message()->getUin() != Uin)) return;
    if (e->state == ICQEvent::Fail){
        setCaption(caption() + " " + i18n("[Failed]"));
        btnCancel->setText(i18n("&Close"));
        return;
    }
}

#ifndef _WINDOWS
#include "securedlg.moc"
#endif


