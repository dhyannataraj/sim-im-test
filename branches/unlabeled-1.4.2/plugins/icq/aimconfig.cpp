/***************************************************************************
                          aimconfig.cpp  -  description
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

#include "aimconfig.h"
#include "icq.h"
#include "icqsearch.h"
#include "linklabel.h"

#include <qtimer.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <qtabwidget.h>
#include <qcombobox.h>

AIMConfig::AIMConfig(QWidget *parent, ICQClient *client, bool bConfig)
        : AIMConfigBase(parent)
{
    m_client = client;
    m_bConfig = bConfig;
    if (m_bConfig){
        QTimer::singleShot(0, this, SLOT(changed()));
        if (m_client->data.owner.Screen)
            edtScreen->setText(m_client->data.owner.Screen);
        edtScreen->setValidator(new AIMValidator(edtScreen));
        edtPasswd->setText(m_client->getPassword());
        edtScreen->setValidator(new AIMValidator(edtScreen));
        connect(edtScreen, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        lnkReg->setText(i18n("Register new ScreenName"));
        lnkReg->setUrl("http://my.screenname.aol.com/_cqr/login/login.psp?siteId=aimregistrationPROD&authLev=1&mcState=initialized&createSn=1&triedAimAuth=y");
    }else{
        tabConfig->removePage(tabAIM);
    }
    edtServer->setText(QString::fromLocal8Bit(m_client->getServer()));
    edtPort->setValue(m_client->getPort());
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
}

void AIMConfig::apply(Client*, void*)
{
}

void AIMConfig::apply()
{
    if (m_bConfig){
        m_client->setScreen(edtScreen->text().lower().latin1());
        m_client->setPassword(edtPasswd->text());
    }
    m_client->setServer(edtServer->text().local8Bit());
    m_client->setPort((unsigned short)atol(edtPort->text()));
}

void AIMConfig::changed(const QString&)
{
    changed();
}

void AIMConfig::changed()
{
    bool bOK = true;
    bOK =  !edtScreen->text().isEmpty() &&
           !edtPasswd->text().isEmpty() &&
           !edtServer->text().isEmpty() &&
           atol(edtPort->text());
    emit okEnabled(bOK);
}

#ifndef WIN32
#include "aimconfig.moc"
#endif


