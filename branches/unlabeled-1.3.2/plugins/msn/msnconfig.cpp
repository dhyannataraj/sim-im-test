/***************************************************************************
                          msnconfig.cpp  -  description
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

#include "msnconfig.h"
#include "msnclient.h"
#include "linklabel.h"

#include <qtimer.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtabwidget.h>

MSNConfig::MSNConfig(QWidget *parent, MSNClient *client, bool bConfig)
        : MSNConfigBase(parent)
{
    m_client = client;
    m_bConfig = bConfig;
    if (m_bConfig)
        tabCfg->removePage(tabMsn);
    QTimer::singleShot(0, this, SLOT(changed()));
    edtLogin->setText(QString::fromUtf8(m_client->getLogin()));
    edtPassword->setText(m_client->getPassword());
    edtServer->setText(QString::fromLocal8Bit(m_client->getServer()));
    edtPort->setValue(m_client->getPort());
    connect(edtLogin, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPassword, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    lnkReg->setText(i18n("Register in .NET Passport"));
    lnkReg->setUrl(i18n("https://register.passport.net/reg.srf?lc=1033&langid=1033&sl=1"));
	edtMinPort->setValue(m_client->getMinPort());
	edtMaxPort->setValue(m_client->getMaxPort());
}

void MSNConfig::apply(Client*, void*)
{
}

void MSNConfig::apply()
{
    if (!m_bConfig){
        m_client->setLogin(edtLogin->text().local8Bit());
        m_client->setPassword(edtPassword->text());
    }
    m_client->setServer(edtServer->text().local8Bit());
    m_client->setPort(atol(edtPort->text()));
    m_client->setMinPort(atol(edtMinPort->text()));
    m_client->setMaxPort(atol(edtMaxPort->text()));
}

void MSNConfig::changed(const QString&)
{
    changed();
}

void MSNConfig::changed()
{
    emit okEnabled(!edtLogin->text().isEmpty() &&
                   !edtPassword->text().isEmpty() &&
                   !edtServer->text().isEmpty() &&
                   atol(edtPort->text()));
}

#ifndef WIN32
#include "msnconfig.moc"
#endif

