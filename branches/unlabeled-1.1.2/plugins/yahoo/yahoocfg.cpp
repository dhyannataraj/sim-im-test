/***************************************************************************
                          yahoocfg.cpp  -  description
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

#include "yahoocfg.h"
#include "yahooclient.h"
#include "linklabel.h"

#include <qtimer.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qspinbox.h>

YahooConfig::YahooConfig(QWidget *parent, YahooClient *client, bool bConfig)
        : YahooConfigBase(parent)
{
    m_client = client;
    m_bConfig = bConfig;
    if (m_bConfig)
        tabCfg->removePage(tabYahoo);
    QTimer::singleShot(0, this, SLOT(changed()));
    edtLogin->setText(m_client->getLogin());
    edtPassword->setText(m_client->getPassword());
    edtServer->setText(QString::fromLocal8Bit(m_client->getServer()));
    edtPort->setValue(m_client->getPort());
    edtFTServer->setText(QString::fromLocal8Bit(m_client->getFTServer()));
    edtFTPort->setValue(m_client->getFTPort());
    connect(edtLogin, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPassword, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    lnkReg->setText(i18n("Get a Yahoo! ID"));
    lnkReg->setUrl("http://edit.yahoo.com/config/eval_register");
}

void YahooConfig::apply(Client*, void*)
{
}

void YahooConfig::apply()
{
    if (!m_bConfig){
        m_client->setLogin(edtLogin->text());
        m_client->setPassword(edtPassword->text());
    }
    m_client->setServer(edtServer->text().local8Bit());
    m_client->setPort((unsigned short)atol(edtPort->text()));
    m_client->setFTServer(edtFTServer->text().local8Bit());
    m_client->setFTPort((unsigned short)atol(edtFTPort->text()));
}

void YahooConfig::changed(const QString&)
{
    changed();
}

void YahooConfig::changed()
{
    emit okEnabled(!edtLogin->text().isEmpty() &&
                   !edtPassword->text().isEmpty() &&
                   !edtServer->text().isEmpty() &&
                   atol(edtPort->text()));
}

#ifndef WIN32
#include "yahoocfg.moc"
#endif

