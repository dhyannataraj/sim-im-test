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

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpixmap.h>
#include <qlineedit.h>

GeneralSecurity::GeneralSecurity(QWidget *p)
        : GeneralSecurityBase(p)
{
    lblPict->setPixmap(Pict("security"));
    chkAuth->setChecked(pClient->Authorize());
    chkHiddenIP->setChecked(pClient->HideIp());
    chkRejectMsg->setChecked(pClient->RejectMessage());
    chkRejectURL->setChecked(pClient->RejectURL());
    chkRejectWeb->setChecked(pClient->RejectWeb());
    chkRejectEmail->setChecked(pClient->RejectEmail());
    chkRejectOther->setChecked(pClient->RejectOther());
    edtFilter->setText(QString::fromLocal8Bit(pClient->RejectFilter.c_str()));
    grpDirect->setButton(pClient->DirectMode());
    rejectToggled(chkRejectMsg->isChecked());
    connect(chkRejectMsg, SIGNAL(toggled(bool)), this, SLOT(rejectToggled(bool)));
    edtFilter->helpText = i18n(
                              "Words are divided by any separators (space, comma, i.e.)\n"
                              "Phrase (sequence of words) consists in quotas\n"
                              "Words can contain wildcards:\n"
                              "* - any amount of symbols (or is empty)\n"
                              "? - any symbol\n");
    connect(chkAuth, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(chkHiddenIP, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(chkRejectMsg, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(chkRejectURL, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(chkRejectWeb, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(chkRejectEmail, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(chkRejectOther, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(btnDAll, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(btnDContact, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
    connect(btnDAuth, SIGNAL(toggled(bool)), this, SLOT(hideHelp(bool)));
}

void GeneralSecurity::apply(ICQUser*)
{
    hideHelp(false);
    pClient->HideIp = chkHiddenIP->isChecked();
    pClient->RejectMessage = chkRejectMsg->isChecked();
    pClient->RejectURL = chkRejectURL->isChecked();
    pClient->RejectWeb = chkRejectWeb->isChecked();
    pClient->RejectEmail = chkRejectEmail->isChecked();
    pClient->RejectOther = chkRejectOther->isChecked();
    pClient->setRejectFilter(edtFilter->text().local8Bit());
    edtFilter->setText(QString::fromLocal8Bit(pClient->RejectFilter.c_str()));
    if (grpDirect->selected())
        pClient->DirectMode = grpDirect->id(grpDirect->selected());
    if (pClient->m_state == ICQClient::Logged)
        pClient->setStatus(pClient->uStatus);
}

void GeneralSecurity::rejectToggled(bool bOn)
{
    lblFilter->setEnabled(!bOn);
    edtFilter->setEnabled(!bOn);
}

void GeneralSecurity::hideHelp(bool)
{
    edtFilter->closeHelp();
}

#ifndef _WINDOWS
#include "generalsec.moc"
#endif

