/***************************************************************************
                          aboutinfo.cpp  -  description
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

#include "generalsec.h"
#include "icons.h"
#include "client.h"
#include "ballonmsg.h"
#include "setupdlg.h"
#include "splash.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>

GeneralSecurity::GeneralSecurity(QWidget *p)
        : GeneralSecurityBase(p)
{
    edtCurrent->setEchoMode(QLineEdit::Password);
    edtPasswd1->setEchoMode(QLineEdit::Password);
    edtPasswd2->setEchoMode(QLineEdit::Password);
    chkAuth->setChecked(pClient->Authorize);
    chkHiddenIP->setChecked(pClient->HideIp);
    chkWeb->setChecked(pClient->WebAware);
    chkBypassAuth->setChecked(pClient->BypassAuth);
    chkRejectMsg->setChecked(pClient->RejectMessage);
    chkRejectURL->setChecked(pClient->RejectURL);
    chkRejectWeb->setChecked(pClient->RejectWeb);
    chkRejectEmail->setChecked(pClient->RejectEmail);
    chkRejectOther->setChecked(pClient->RejectOther);
    chkSave->setChecked(pSplash->getSavePassword());
    chkNoShow->setChecked(pSplash->getNoShowLogin());
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
    QString err;
    QWidget *errWidget = NULL;
    if (edtCurrent->text().isEmpty()){
        if (!edtPasswd1->text().isEmpty() || !edtPasswd2->text().isEmpty() ||
                (chkSave->isChecked() != pSplash->getSavePassword()) ||
                (chkNoShow->isChecked() != pSplash->getNoShowLogin())){
            err = i18n("Input current password");
            errWidget = edtCurrent;
        }
    }else if (edtPasswd1->text() != edtPasswd2->text()){
        err = i18n("Confirm password does not match");
        errWidget = edtPasswd2;
    }else{
        string s = ICQClient::cryptPassword(edtCurrent->text().local8Bit());
        if (strcmp(s.c_str(), pClient->EncryptedPassword.c_str())){
            err = i18n("Invalid password");
            errWidget = edtCurrent;
        }
    }
    if (!err.isEmpty()){
        tabWnd->setCurrentPage(3);
        SetupDialog *setup = static_cast<SetupDialog*>(topLevelWidget());
        setup->raiseWidget(SETUP_GENERAL_SEC);
        setup->applyOk = false;
        BalloonMsg::message(err, errWidget, true);
        return;
    }
    if (!edtPasswd1->text().isEmpty())
        pClient->setPassword(edtPasswd1->text().local8Bit());
    pSplash->setSavePassword(chkSave->isChecked());
    pSplash->setNoShowLogin(chkNoShow->isChecked());
    pSplash->save();
}

void GeneralSecurity::rejectToggled(bool bOn)
{
    lblFilter->setEnabled(!bOn);
    edtFilter->setEnabled(!bOn);
}

#ifndef _WINDOWS
#include "generalsec.moc"
#endif

