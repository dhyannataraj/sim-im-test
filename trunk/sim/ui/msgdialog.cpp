/***************************************************************************
                          alertdialog.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
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
#include <qtabwidget.h>

static void set(QMultiLineEdit *edit, const string &s, ICQUser *u)
{
    edit->setText(pClient->from8Bit(u->Uin, s.c_str()));
}

MsgDialog::MsgDialog(QWidget *p, unsigned long _status, bool bReadOnly)
        : MsgDialogBase(p), status(_status)
{
    tabWnd->setCurrentPage(0);
    QWidget *w = tabWnd->currentPage();
    tabWnd->changeTab(w, SIMClient::getStatusText(status));
    if (bReadOnly){
        chkNoShow->hide();
    }else{
        chkOverride->hide();
    }
    switch (status){
    case ICQ_STATUS_AWAY:
        setup(pClient->owner, offsetof(UserSettings, AutoResponseAway));
        chkNoShow->setChecked(pMain->isNoShowAway());
        break;
    case ICQ_STATUS_NA:
        setup(pClient->owner, offsetof(UserSettings, AutoResponseNA));
        chkNoShow->setChecked(pMain->isNoShowNA());
        break;
    case ICQ_STATUS_DND:
        setup(pClient->owner, offsetof(UserSettings, AutoResponseDND));
        chkNoShow->setChecked(pMain->isNoShowDND());
        break;
    case ICQ_STATUS_OCCUPIED:
        setup(pClient->owner, offsetof(UserSettings, AutoResponseOccupied));
        chkNoShow->setChecked(pMain->isNoShowOccupied());
        break;
    case ICQ_STATUS_FREEFORCHAT:
        setup(pClient->owner, offsetof(UserSettings, AutoResponseFFC));
        chkNoShow->setChecked(pMain->isNoShowFFC());
        break;
    }
    connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
}

void MsgDialog::setup(ICQUser *u, unsigned offs)
{
    ICQUser *_u = u;
    UserSettings *settings = &static_cast<SIMUser*>(u)->settings;
    char **s = (char**)((char*)settings + offs);
    if ((*s == NULL) || (**s == 0)){
        chkOverride->setChecked(false);
        _u = pClient->owner;
    }else{
        chkOverride->setChecked(true);
    }
    set(edtMessage, pClient->getAutoResponse(u, offs), _u);
}

void MsgDialog::setup(ICQGroup *g, unsigned offs)
{
    UserSettings *settings = &static_cast<SIMGroup*>(g)->settings;
    char **s = (char**)((char*)settings + offs);
    if ((*s == NULL) || (**s == 0)){
        chkOverride->setChecked(false);
    }else{
        chkOverride->setChecked(true);
    }
    settings = pClient->getSettings(g, offs, true);
    s = (char**)((char*)settings + offs);
    set(edtMessage, *s ? *s : "", pClient->owner);
}

void MsgDialog::load(ICQUser *u)
{
    switch (status){
    case ICQ_STATUS_AWAY:
        setup(u, offsetof(UserSettings, AutoResponseAway));
        break;
    case ICQ_STATUS_NA:
        setup(u, offsetof(UserSettings, AutoResponseNA));
        break;
    case ICQ_STATUS_DND:
        setup(u, offsetof(UserSettings, AutoResponseDND));
        break;
    case ICQ_STATUS_OCCUPIED:
        setup(u, offsetof(UserSettings, AutoResponseOccupied));
        break;
    case ICQ_STATUS_FREEFORCHAT:
        setup(u, offsetof(UserSettings, AutoResponseFFC));
        break;
    }
    overrideChanged(chkOverride->isChecked());
}

void MsgDialog::load(ICQGroup *g)
{
    switch (status){
    case ICQ_STATUS_AWAY:
        setup(g, offsetof(UserSettings, AutoResponseAway));
        break;
    case ICQ_STATUS_NA:
        setup(g, offsetof(UserSettings, AutoResponseNA));
        break;
    case ICQ_STATUS_DND:
        setup(g, offsetof(UserSettings, AutoResponseDND));
        break;
    case ICQ_STATUS_OCCUPIED:
        setup(g, offsetof(UserSettings, AutoResponseOccupied));
        break;
    case ICQ_STATUS_FREEFORCHAT:
        setup(g, offsetof(UserSettings, AutoResponseFFC));
        break;
    }
    overrideChanged(chkOverride->isChecked());
}

void MsgDialog::save(ICQUser *u)
{
    save(&static_cast<SIMUser*>(u)->settings);
}

void MsgDialog::save(ICQGroup *g)
{
    save(&static_cast<SIMGroup*>(g)->settings);
}

void MsgDialog::save(UserSettings *settings)
{
    string res;
    set(res, edtMessage->text());
    if (!chkOverride->isChecked()) res = "";
    switch (status){
    case ICQ_STATUS_AWAY:
        set_str(&settings->AutoResponseAway, res.c_str());
        break;
    case ICQ_STATUS_NA:
        set_str(&settings->AutoResponseNA, res.c_str());
        break;
    case ICQ_STATUS_DND:
        set_str(&settings->AutoResponseDND, res.c_str());
        break;
    case ICQ_STATUS_OCCUPIED:
        set_str(&settings->AutoResponseOccupied, res.c_str());
        break;
    case ICQ_STATUS_FREEFORCHAT:
        set_str(&settings->AutoResponseFFC, res.c_str());
        break;
    }
}

void MsgDialog::overrideChanged(bool bOverride)
{
    edtMessage->setReadOnly(!bOverride);
}

void MsgDialog::apply(ICQUser*)
{
    UserSettings *settings = &static_cast<SIMUser*>(pClient->owner)->settings;
    switch (status){
    case ICQ_STATUS_AWAY:
        set(&settings->AutoResponseAway, edtMessage->text());
        pMain->setNoShowAway(chkNoShow->isChecked());
        break;
    case ICQ_STATUS_NA:
        set(&settings->AutoResponseNA, edtMessage->text());
        pMain->setNoShowNA(chkNoShow->isChecked());
        break;
    case ICQ_STATUS_DND:
        set(&settings->AutoResponseDND, edtMessage->text());
        pMain->setNoShowDND(chkNoShow->isChecked());
        break;
    case ICQ_STATUS_OCCUPIED:
        set(&settings->AutoResponseOccupied, edtMessage->text());
        pMain->setNoShowOccupied(chkNoShow->isChecked());
        break;
    case ICQ_STATUS_FREEFORCHAT:
        set(&settings->AutoResponseFFC, edtMessage->text());
        pMain->setNoShowFFC(chkNoShow->isChecked());
        break;
    }
}

#ifndef _WINDOWS
#include "msgdialog.moc"
#endif

