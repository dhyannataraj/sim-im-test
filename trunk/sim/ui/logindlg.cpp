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
#include "ballonmsg.h"
#include "proxydlg.h"
#include "client.h"
#include "icons.h"
#include "log.h"
#include "enable.h"
#include "mainwin.h"
#include "sim.h"
#include "splash.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qvalidator.h>

LoginDialog::LoginDialog()
        : LoginDlgBase(NULL, "logindlg")
{
    setButtonsPict(this);
    setIcon(Pict("licq"));
    bLogin = false;
    cmbUIN->setEditable(true);
    cmbUIN->insertItem("");
    cmbUIN->insertItem(i18n("<New UIN>"));
    int id = 2;
    for (list<unsigned long>::iterator it = uins.begin(); it != uins.end(); ++it, ++id){
        cmbUIN->insertItem(QString::number(*it));
        if ((*it) == pSplash->LastUIN) cmbUIN->setCurrentItem(id);
    }
    cmbUIN->lineEdit()->setValidator(new QIntValidator(100000, 0x7FFFFFFF, this));
    edtPasswd->setEchoMode(QLineEdit::Password);
    connect(cmbUIN->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnLogin, SIGNAL(clicked()), this, SLOT(login()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteUin()));
    connect(btnProxy, SIGNAL(clicked()), this, SLOT(proxySetup()));
    textChanged("");
    QSize s = sizeHint();
    QWidget *desktop = QApplication::desktop();
    move((desktop->width() - s.width()) / 2, (desktop->height() - s.height()) / 2);
    bCloseMain = true;
};

void LoginDialog::textChanged(const QString&)
{
    unsigned long uin = cmbUIN->lineEdit()->text().toULong();
    bool isUin = !cmbUIN->lineEdit()->text().isEmpty();
    bool isPswd = !edtPasswd->text().isEmpty();
    btnProxy->setEnabled(isUin);
    btnLogin->setEnabled(isUin && isPswd);
    chkSave->setEnabled(isUin && isPswd);
    btnDelete->setEnabled(uin && isUin && isPswd);
}

void LoginDialog::login()
{
    unsigned long uin = cmbUIN->lineEdit()->text().toULong();
    pClient->load(uin);
    if (uin && pClient->EncryptedPassword.length()){
        string s = ICQClient::cryptPassword(edtPasswd->text().local8Bit());
        if (strcmp(s.c_str(), pClient->EncryptedPassword.c_str())){
            BalloonMsg::message(i18n("Invalid password"), btnLogin);
            return;
        }
        pSplash->LastUIN = pClient->owner->Uin;
        pSplash->SavePassword = chkSave->isChecked();
        pSplash->save();
        pMain->init();
        bCloseMain = false;
        close();
        return;
    }
    btnProxy->setEnabled(false);
    btnClose->setText(i18n("Cancel"));
    lblUIN->setEnabled(false);
    cmbUIN->setEnabled(false);
    lblPasswd->setEnabled(false);
    edtPasswd->setEnabled(false);
    chkSave->setEnabled(false);
    btnLogin->setEnabled(false);
    bLogin = true;
    pClient->storePassword(edtPasswd->text().local8Bit());
    pClient->DecryptedPassword = edtPasswd->text().local8Bit();
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    pClient->setStatus(ICQ_STATUS_ONLINE);
}

void LoginDialog::closeEvent(QCloseEvent *e)
{
    if (!bLogin){
        if (bCloseMain){
            pMain->quit();
        }else{
            QTimer::singleShot(0, pMain, SLOT(deleteLogin()));
        }
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
    pClient->owner->Uin = 0;
    pClient->EncryptedPassword = "";
    pClient->DecryptedPassword = "";
    btnClose->setText(i18n("Close"));
    lblPasswd->setEnabled(true);
    edtPasswd->setEnabled(true);
    btnProxy->setEnabled(true);
    textChanged("");
    bLogin = false;
}

void LoginDialog::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_STATUS_CHANGED:
        if (pClient->isLogged()){
            bLogin = false;
            pClient->DecryptedPassword = "";
            pSplash->LastUIN = pClient->owner->Uin;
            pSplash->SavePassword = chkSave->isChecked();
            pSplash->save();
            pMain->init();
            bCloseMain = false;
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
    case EVENT_PROXY_ERROR:{
            stopLogin();
            ProxyDialog d(this, i18n("Can't connect to proxy server"));
            d.exec();
            return;
        }
    case EVENT_PROXY_BAD_AUTH:{
            stopLogin();
            ProxyDialog d(this, pClient->factory()->ProxyAuth ?
                          i18n("Proxy server require authorization") :
                          i18n("Invalid password for proxy"));
            d.exec();
            return;
        }
    }
}

void LoginDialog::proxySetup()
{
    pClient->load(cmbUIN->lineEdit()->text().toULong());
    ProxyDialog d(this);
    d.exec();
}

void LoginDialog::deleteUin()
{
    unsigned long uin = cmbUIN->lineEdit()->text().toULong();
    if (uin == 0) return;
    pClient->load(uin);
    string s = ICQClient::cryptPassword(edtPasswd->text().local8Bit());
    if (strcmp(s.c_str(), pClient->EncryptedPassword.c_str())){
        BalloonMsg::message(i18n("Invalid password"), btnDelete);
        return;
    }
}

#ifndef _WINDOWS
#include "logindlg.moc"
#endif

