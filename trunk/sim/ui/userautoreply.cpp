/***************************************************************************
                          userautoreply.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "userautoreply.h"
#include "client.h"
#include "mainwin.h"
#include "icons.h"
#include "cuser.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qmultilineedit.h>

UserAutoReplyDlg::UserAutoReplyDlg(unsigned long _uin)
        : AutoreplyBase(NULL), uin(_uin)
{
    setWFlags(WDestructiveClose);
    lblTimer->hide();
    chkNoShow->hide();
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    edtMessage->setReadOnly(true);
    edtMessage->setText(pClient->from8Bit(uin, u->AutoReply.c_str()));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(click()));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    setStatus();
    bProcess = false;
    start();
}

void UserAutoReplyDlg::setStatus()
{
    ICQUser *u = pClient->getUser(uin);
    if (u == NULL) return;
    CUser usr(u);
    QString c = i18n("%1 autoresponse for %2")
                .arg(SIMClient::getStatusText(u->uStatus))
                .arg(usr.name());

    setCaption(c);

    setIcon(Pict(SIMClient::getStatusIcon(u->uStatus)));
}

void UserAutoReplyDlg::click()
{
    if (bProcess){
        close();
        return;
    }
    start();
}

void UserAutoReplyDlg::start()
{
    if (bProcess){
        close();
        return;
    }
    edtMessage->setEnabled(false);
    btnOK->setText(i18n("&Close"));
    pClient->addResponseRequest(uin, true);
    bProcess = true;
}

void UserAutoReplyDlg::processEvent(ICQEvent *e)
{
    if (!bProcess) return;
    if (e->Uin() != uin) return;
    if ((e->type() == EVENT_INFO_CHANGED) && (e->subType() == EVENT_SUBTYPE_AUTOREPLY)){
        ICQUser *u = pClient->getUser(uin);
        if (u == NULL) return;
        edtMessage->setText(pClient->from8Bit(uin, u->AutoReply.c_str()));
        edtMessage->setEnabled(true);
        btnOK->setText(i18n("&Check again"));
        bProcess = false;
        return;
    }
    if (e->type() == EVENT_STATUS_CHANGED)
        setStatus();
}

#ifndef _WINDOWS
#include "userautoreply.moc"
#endif

