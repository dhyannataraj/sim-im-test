/***************************************************************************
                          logindlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "logindlg.h"
#include "ui/ballonmsg.h"
#include "client.h"
#include "icons.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qvalidator.h>

LoginDialog::LoginDialog()
        : QDialog(NULL, NULL, true)
{
    setIcon(Pict("licq"));
    bLogin = false;
    QGridLayout *lay = new QGridLayout(this, 4, 2, 10, 5);
    lblUIN = new QLabel(i18n("UIN:"), this);
    lblUIN->setAlignment(AlignRight | AlignVCenter);
    lay->addWidget(lblUIN, 0, 0);
    edtUIN = new QLineEdit(this);
    edtUIN->setValidator(new QIntValidator(100000, 0x7FFFFFFF, this));
    connect(edtUIN, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    lay->addWidget(edtUIN, 0, 1);
    lblPasswd = new QLabel(i18n("Password:"), this);
    lblPasswd->setAlignment(AlignRight | AlignVCenter);
    lay->addWidget(lblPasswd, 1, 0);
    edtPasswd = new QLineEdit(this);
    edtPasswd->setEchoMode(QLineEdit::Password);
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    lay->addWidget(edtPasswd, 1, 1);
    chkOldUser = new QCheckBox(i18n("Use existing UIN"), this);
    connect(chkOldUser, SIGNAL(toggled(bool)), this, SLOT(setOldUser(bool)));
    lay->addMultiCellWidget(chkOldUser, 2, 2, 0, 1);
    QHBoxLayout *hLay = new QHBoxLayout();
    lay->addMultiCellLayout(hLay, 3, 3, 0, 1);
    hLay->addStretch();
    btnClose = new QPushButton(i18n("Close"), this);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    hLay->addWidget(btnClose);
    hLay->addStretch();
    btnLogin = new QPushButton(i18n("Register"), this);
    btnLogin->setDefault(true);
    connect(btnLogin, SIGNAL(clicked()), this, SLOT(login()));
    hLay->addWidget(btnLogin);
    hLay->addStretch();
    setCaption(i18n("Registration"));
    setOldUser(false);
    textChanged("");
    QSize s = sizeHint();
    QWidget *desktop = QApplication::desktop();
    move((desktop->width() - s.width()) / 2, (desktop->height() - s.height()) / 2);
};

void LoginDialog::setOldUser(bool bSet)
{
    lblUIN->setEnabled(bSet);
    edtUIN->setEnabled(bSet);
    if (!bSet) edtUIN->setText("");
}

void LoginDialog::textChanged(const QString&)
{
    btnLogin->setEnabled((edtPasswd->text().length() > 0) &&
                         (!chkOldUser->isOn() || (edtUIN->text().length() > 0)));
}

void LoginDialog::login()
{
    btnClose->setText(i18n("Cancel"));
    lblUIN->setEnabled(false);
    edtUIN->setEnabled(false);
    lblPasswd->setEnabled(false);
    edtPasswd->setEnabled(false);
    chkOldUser->setEnabled(false);
    btnLogin->setEnabled(false);
    bLogin = true;
    pClient->storePassword(edtPasswd->text().local8Bit());
    pClient->DecryptedPassword = edtPasswd->text().local8Bit();
    if (chkOldUser->isOn())
        pClient->Uin = edtUIN->text().toULong();
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    pClient->setStatus(ICQ_STATUS_ONLINE);
}

void LoginDialog::closeEvent(QCloseEvent *e)
{
    if (!bLogin){
        QDialog::closeEvent(e);
        return;
    }
    e->ignore();
    stopLogin();
}

void LoginDialog::stopLogin()
{
    disconnect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    pClient->setStatus(ICQ_STATUS_OFFLINE);
    pClient->Uin = 0;
    pClient->EncryptedPassword = "";
    pClient->DecryptedPassword = "";
    btnClose->setText(i18n("Close"));
    lblPasswd->setEnabled(true);
    edtPasswd->setEnabled(true);
    chkOldUser->setEnabled(true);
    setOldUser(chkOldUser->isOn());
    textChanged("");
    bLogin = false;
}

void LoginDialog::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_STATUS_CHANGED:
        if (pClient->m_state == ICQClient::Logged){
            bLogin = false;
            pClient->DecryptedPassword = "";
            close();
        }
        return;
    case EVENT_LOGIN_ERROR:
        BalloonMsg::message(i18n("Unknown login error"), btnLogin);
        stopLogin();
        return;
    case EVENT_RATE_LIMIT:
        BalloonMsg::message(i18n("Rate limit"), btnLogin);
        stopLogin();
        return;
    case EVENT_BAD_PASSWORD:
        BalloonMsg::message(i18n("Invalid password"), edtPasswd);
        stopLogin();
        return;
    }
}

#ifndef _WINDOWS
#include "logindlg.moc"
#endif

