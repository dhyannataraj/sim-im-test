/***************************************************************************
                          aboutinfo.cpp  -  description
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

#include "generalsec.h"
#include "icons.h"
#include "client.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpixmap.h>

GeneralSecurity::GeneralSecurity(QWidget *p)
        : GeneralSecurityBase(p)
{
    lblPict->setPixmap(Pict("security"));
    chkAuth->setChecked(pClient->Authorize());
    chkWebAware->setChecked(pClient->WebAware());
    chkHiddenIP->setChecked(pClient->HideIp());
    chkRejectMsg->setChecked(pClient->RejectMessage());
    chkRejectURL->setChecked(pClient->RejectURL());
    chkRejectWeb->setChecked(pClient->RejectWeb());
    chkRejectEmail->setChecked(pClient->RejectEmail());
    chkRejectOther->setChecked(pClient->RejectOther());
    grpDirect->setButton(pClient->DirectMode());
}

void GeneralSecurity::apply(ICQUser*)
{
    pClient->setSecurityInfo(chkAuth->isChecked(), chkWebAware->isChecked());
    pClient->HideIp = chkHiddenIP->isChecked();
    pClient->RejectMessage = chkRejectMsg->isChecked();
    pClient->RejectURL = chkRejectURL->isChecked();
    pClient->RejectWeb = chkRejectWeb->isChecked();
    pClient->RejectEmail = chkRejectEmail->isChecked();
    pClient->RejectOther = chkRejectOther->isChecked();
    if (grpDirect->selected())
        pClient->DirectMode = grpDirect->id(grpDirect->selected());
    if (pClient->m_state == ICQClient::Logged)
        pClient->setStatus(pClient->uStatus);
}

#ifndef _WINDOWS
#include "generalsec.moc"
#endif

