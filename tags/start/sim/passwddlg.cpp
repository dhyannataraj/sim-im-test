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

#include "passwddlg.h"
#include "client.h"
#include "mainwin.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qapplication.h>

PasswdDialog::PasswdDialog()
        : QDialog(NULL, NULL, true)
{
    bLogin = false;
    oldPassword = QString::fromLocal8Bit(pClient->Password);
    QGridLayout *lay = new QGridLayout(this, 2, 2, 10, 5);
    lblPasswd = new QLabel(i18n("Password:"), this);
    lblPasswd->setAlignment(AlignRight | AlignVCenter);
    lay->addWidget(lblPasswd, 0, 0);
    edtPasswd = new QLineEdit(this);
    edtPasswd->setEchoMode(QLineEdit::Password);
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    lay->addWidget(edtPasswd, 0, 1);
    QHBoxLayout *hLay = new QHBoxLayout();
    lay->addMultiCellLayout(hLay, 2, 2, 0, 1);
    hLay->addStretch();
    btnClose = new QPushButton(i18n("Close"), this);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    hLay->addWidget(btnClose);
    hLay->addStretch();
    btnLogin = new QPushButton(i18n("Login"), this);
    btnLogin->setDefault(true);
    connect(btnLogin, SIGNAL(clicked()), this, SLOT(login()));
    hLay->addWidget(btnLogin);
    hLay->addStretch();
    setCaption(i18n("Invalid password"));
    textChanged("");
    QSize s = sizeHint();
    QWidget *desktop = QApplication::desktop();
    move((desktop->width() - s.width()) / 2, (desktop->height() - s.height()) / 2);
};

void PasswdDialog::textChanged(const QString&)
{
    btnLogin->setEnabled(edtPasswd->text().length() > 0);
}

void PasswdDialog::login()
{
    btnClose->setText(i18n("Cancel"));
    lblPasswd->setEnabled(false);
    edtPasswd->setEnabled(false);
    btnLogin->setEnabled(false);
    bLogin = true;
    pClient->Password = edtPasswd->text().local8Bit();
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    pClient->setStatus(pMain->ManualStatus);
}

void PasswdDialog::closeEvent(QCloseEvent *e)
{
    if (!bLogin) QDialog::closeEvent(e);
    e->ignore();
    stopLogin();
}

void PasswdDialog::stopLogin()
{
    disconnect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    pClient->setStatus(ICQ_STATUS_OFFLINE);
    pClient->Password = oldPassword;
    btnClose->setText(i18n("Close"));
    lblPasswd->setEnabled(true);
    edtPasswd->setEnabled(true);
    textChanged("");
    bLogin = false;
}

void PasswdDialog::processEvent(ICQEvent *e)
{
    if (pClient->m_state == ICQClient::Logged){
        bLogin = false;
        close();
        return;
    }
    if (e->type() == EVENT_BAD_PASSWORD){
        stopLogin();
    }
}

#ifndef _WINDOWS
#include "passwddlg.moc"
#endif

