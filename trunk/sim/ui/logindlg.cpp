/***************************************************************************
                          logindlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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
#include <qfile.h>
#include <qdir.h>

extern LoginDialog* pLoginDlg;

LoginDialog::LoginDialog()
        : LoginDlgBase(NULL, "logindlg", false, WDestructiveClose)
{
    SET_WNDPROC("login")
    setCaption(caption());
    setButtonsPict(this);
    setIcon(Pict("licq"));
    bLogin = false;
    bMyInit = false;
    cmbUIN->setEditable(true);
    cmbUIN->lineEdit()->setValidator(new QIntValidator(100000, 0x7FFFFFFF, this));
    loadUins();
    edtPasswd->setEchoMode(QLineEdit::Password);
    connect(cmbUIN->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(uinChanged(const QString&)));
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(pswdChanged(const QString&)));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnLogin, SIGNAL(clicked()), this, SLOT(login()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteUin()));
    connect(btnProxy, SIGNAL(clicked()), this, SLOT(proxySetup()));
    connect(chkSave, SIGNAL(toggled(bool)), this, SLOT(saveChanged(bool)));
    QSize s = sizeHint();
    QWidget *desktop = QApplication::desktop();
    move((desktop->width() - s.width()) / 2, (desktop->height() - s.height()) / 2);
    chkSave->setChecked(pSplash->isSavePassword());
    chkNoShow->setChecked(pSplash->isNoShowLogin());
    uinChanged("");
    bPswdChanged = true;
    if (pSplash->isSavePassword()){
        unsigned long uin = cmbUIN->lineEdit()->text().toULong();
        if (uin){
            pClient->load(uin);
            QString pswd;
            for (const char *p = pClient->EncryptedPassword.c_str(); *p; p++){
                if (*p == '\\') continue;
                pswd += '*';
            }
            edtPasswd->setText(pswd);
            pswdChanged("");
            if (!pswd.isEmpty()) bPswdChanged = false;
        }
    }
    bCloseMain = true;
};

LoginDialog::~LoginDialog() {
	pLoginDlg = NULL;	
}

void LoginDialog::loadUins()
{
    cmbUIN->insertItem("");
    cmbUIN->insertItem(i18n("<New UIN>"));
    int id = 2;
    for (list<unsigned long>::iterator it = uins.begin(); it != uins.end(); ++it, ++id){
        cmbUIN->insertItem(QString::number(*it));
        if ((*it) == pSplash->getLastUIN()) cmbUIN->setCurrentItem(id);
    }
}

void LoginDialog::uinChanged(const QString&)
{
    edtPasswd->setText("");
    pswdChanged("");
}

void LoginDialog::pswdChanged(const QString&)
{
    unsigned long uin = cmbUIN->lineEdit()->text().toULong();
    bool isUin = !cmbUIN->lineEdit()->text().isEmpty();
    bool isPswd = !edtPasswd->text().isEmpty();
    btnProxy->setEnabled(isUin);
    btnLogin->setEnabled(isUin && isPswd);
    chkSave->setEnabled(isUin && isPswd);
    btnDelete->setEnabled(uin && isUin && isPswd);
    bPswdChanged = true;
    saveChanged(false);
}

void LoginDialog::saveChanged(bool)
{
    bool isPswd = !edtPasswd->text().isEmpty();
    if (isPswd){
        chkSave->setEnabled(true);
        chkNoShow->setEnabled(chkSave->isChecked());
        return;
    }
    chkSave->setEnabled(false);
    chkNoShow->setEnabled(false);
}

void LoginDialog::login()
{
    pMain->reset();
    unsigned long uin = cmbUIN->lineEdit()->text().toULong();
    pClient->load(uin);
    if (uin){
        bool bOk = !bPswdChanged;
        if (pClient->EncryptedPassword.length() == 0) bOk = false;
        if (!bOk){
            string s = ICQClient::cryptPassword(edtPasswd->text().local8Bit());
            if (!strcmp(s.c_str(), pClient->EncryptedPassword.c_str()))
                bOk = true;
        }
        if (bOk){
            pSplash->setLastUIN(pClient->owner->Uin);
            pSplash->setSavePassword(chkSave->isChecked());
            pSplash->setNoShowLogin(chkNoShow->isChecked());
            pSplash->save();
            if (!pMain->init()){
                BalloonMsg::message(i18n("Can't initialize main window"), btnLogin);
                return;
            }
            bCloseMain = false;
            close();
            return;
        }
    }
    btnProxy->setEnabled(false);
    btnClose->setText(i18n("Cancel"));
    lblUIN->setEnabled(false);
    cmbUIN->setEnabled(false);
    lblPasswd->setEnabled(false);
    edtPasswd->setEnabled(false);
    chkSave->setEnabled(false);
    chkNoShow->setEnabled(false);
    btnLogin->setEnabled(false);
    btnDelete->setEnabled(false);
    bLogin = true;
    pClient->storePassword(edtPasswd->text().local8Bit());
    pClient->DecryptedPassword = edtPasswd->text().local8Bit();
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    pMain->init(true);
    bMyInit = true;
    pClient->setStatus((pMain->getManualStatus() == ICQ_STATUS_OFFLINE) ? ICQ_STATUS_ONLINE : pMain->getManualStatus());
}

void LoginDialog::closeEvent(QCloseEvent *e)
{
    if (!bLogin){
        if (bMyInit || !pMain->isLoad()){
            if (bCloseMain)
                pMain->quit();
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
    uinChanged("");
    bLogin = false;
}

void LoginDialog::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_STATUS_CHANGED:
        if (pClient->isLogged() && bLogin){
            bLogin = false;
            pClient->DecryptedPassword = "";
            pSplash->setLastUIN(pClient->owner->Uin);
            pSplash->setSavePassword(chkSave->isChecked());
            pSplash->setNoShowLogin(chkNoShow->isChecked());
            pSplash->save();
            if (!pMain->init()){
                BalloonMsg::message(i18n("Can't initialize main window"), btnLogin);
                stopLogin();
                return;
            }
            if (pMain->getManualStatus() == ICQ_STATUS_OFFLINE)
                pMain->setManualStatus(ICQ_STATUS_ONLINE);
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
    pMain->reset();
    pClient->load(cmbUIN->lineEdit()->text().toULong());
    ProxyDialog d(this);
    d.exec();
}

static void rmDir(const QString &path)
{
    QDir d(path);
    QStringList l = d.entryList(QDir::Dirs);
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it){
        if (((*it) == ".") || ((*it) == "..")) continue;
        QString p = path;
#ifdef WIN32
        p += "\\";
#else
        p += "/";
#endif
        p += *it;
        rmDir(p);
    }
    l = d.entryList();
    for (it = l.begin(); it != l.end(); ++it){
        if (((*it) == ".") || ((*it) == "..")) continue;
        QString p = path;
#ifdef WIN32
        p += "\\";
#else
        p += "/";
#endif
        p += *it;
        d.remove(p);
    }
    d.rmdir(path);
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
    QStringList btns;
    btns.append(i18n("&Yes"));
    btns.append(i18n("&No"));
    BalloonMsg *msg = new BalloonMsg(i18n("Delete history and incoming files for %u?") .arg(uin),
                                     btns, btnDelete);
    connect(msg, SIGNAL(action(int)), this, SLOT(realDeleteUin(int)));
    msg->show();
}

void LoginDialog::realDeleteUin(int n)
{
    if (n) return;
    rmDir(QString::fromLocal8Bit(pMain->getFullPath("").c_str()));
    pClient->load(0);
    scanUIN();
    cmbUIN->clear();
    loadUins();
}

#ifndef _WINDOWS
#include "logindlg.moc"
#endif

