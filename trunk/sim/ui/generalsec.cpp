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
#include "helpedit.h"
#include "icons.h"
#include "client.h"
#include "ballonmsg.h"
#include "setupdlg.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qtabwidget.h>

GeneralSecurity::GeneralSecurity(QWidget *p)
        : GeneralSecurityBase(p)
{
    chkAuth->setChecked(pClient->Authorize);
    chkHiddenIP->setChecked(pClient->HideIp);
    chkWeb->setChecked(pClient->WebAware);
    chkBypassAuth->setChecked(pClient->BypassAuth);
    chkRejectMsg->setChecked(pClient->RejectMessage);
    chkRejectURL->setChecked(pClient->RejectURL);
    chkRejectWeb->setChecked(pClient->RejectWeb);
    chkRejectEmail->setChecked(pClient->RejectEmail);
    chkRejectOther->setChecked(pClient->RejectOther);
    edtFilter->setText(QString::fromLocal8Bit(pClient->RejectFilter.c_str()));
    grpDirect->setButton(pClient->DirectMode);
    rejectToggled(chkRejectMsg->isChecked());
    connect(chkRejectMsg, SIGNAL(toggled(bool)), this, SLOT(rejectToggled(bool)));
}

void GeneralSecurity::apply(ICQUser*)
{
    pClient->HideIp = chkHiddenIP->isChecked();
    pClient->BypassAuth = chkBypassAuth->isChecked();
    pClient->RejectMessage = chkRejectMsg->isChecked();
    pClient->RejectURL = chkRejectURL->isChecked();
    pClient->RejectWeb = chkRejectWeb->isChecked();
    pClient->RejectEmail = chkRejectEmail->isChecked();
    pClient->RejectOther = chkRejectOther->isChecked();
    pClient->setRejectFilter(edtFilter->text().local8Bit());
    edtFilter->setText(QString::fromLocal8Bit(pClient->RejectFilter.c_str()));
    if (grpDirect->selected())
        pClient->DirectMode = grpDirect->id(grpDirect->selected());
    if (pClient->isLogged())
        pClient->setStatus(pClient->owner->uStatus);
    pClient->setSecurityInfo(chkAuth->isChecked(), chkWeb->isChecked());
    if (edtPasswd1->text().length() == 0) return;
    if (edtPasswd1->text() != edtPasswd2->text()){
        tabWnd->setCurrentPage(3);
        SetupDialog *setup = static_cast<SetupDialog*>(topLevelWidget());
        setup->applyOk = false;
        BalloonMsg::message(i18n("Confirm password does not match"), edtPasswd2, true);
        return;
    }
    pClient->setPassword(edtPasswd1->text().local8Bit());
}

void GeneralSecurity::rejectToggled(bool bOn)
{
    lblFilter->setEnabled(!bOn);
    edtFilter->setEnabled(!bOn);
}

#ifndef _WINDOWS
#include "generalsec.moc"
#endif

