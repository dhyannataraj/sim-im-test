/***************************************************************************
                          jabberconfig.cpp  -  description
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

#include "jabberconfig.h"
#include "jabberclient.h"
#include "jabber.h"
#include "linklabel.h"

#include <qtimer.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtabwidget.h>

JabberConfig::JabberConfig(QWidget *parent, JabberClient *client, bool bConfig)
        : JabberConfigBase(parent)
{
    m_client = client;
    m_bConfig = bConfig;
    QTimer::singleShot(0, this, SLOT(changed()));
    edtID->setText(m_client->getID());
    edtPasswd->setText(m_client->getPassword());
    edtServer->setText(QString::fromLocal8Bit(m_client->getServer()));
    edtPort->setValue(m_client->getPort());
    edtPriority->setValue(m_client->getPriority());
    if (m_client->data.owner.Resource)
        edtResource->setText(QString::fromUtf8(m_client->data.owner.Resource));
    if (m_client->data.VHost)
        edtVHost->setText(QString::fromUtf8(m_client->data.VHost));
    if (m_bConfig){
        tabCfg->removePage(tabJabber);
    }else{
        lblServer->hide();
        edtServer->hide();
        lblPort->hide();
        edtPort->hide();
        edtServer1->setText(i18n("jabber.org"));
        edtPort1->setValue(m_client->getPort());
    }
#ifdef USE_OPENSSL
    chkSSL->setChecked(m_client->getUseSSL());
    chkPlain->setChecked(m_client->getUsePlain());
#else
    chkSSL->hide();
    chkPlain->hide();
#endif
    chkVHost->setChecked(m_client->getUseVHost());
    lnkPublic->setText(i18n("List of public servers"));
    lnkPublic->setUrl("http://www.jabber.org/user/publicservers.php");
    connect(edtID, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(chkSSL, SIGNAL(toggled(bool)), this, SLOT(toggledSSL(bool)));
    connect(chkVHost, SIGNAL(toggled(bool)), this, SLOT(toggledVHost(bool)));
}

void JabberConfig::apply(Client*, void*)
{
}

void JabberConfig::apply()
{
    QString jid = edtID->text();
    if (jid.find('@') < 0){
        QString host;
        if (chkVHost->isChecked() && !edtVHost->text().isEmpty()){
            host = edtVHost->text();
        }else{
            host = edtServer->text();
        }
        if (!host.isEmpty()){
            jid += "@";
            jid += host;
        }
    }
    if (!m_bConfig){
        m_client->setID(jid);
        m_client->setPassword(edtPasswd->text());
        m_client->setRegister(chkRegister->isChecked());
    }
    if (m_bConfig){
        m_client->setServer(edtServer->text().local8Bit());
        m_client->setPort(atol(edtPort->text()));
    }else{
        m_client->setServer(edtServer1->text().local8Bit());
        m_client->setPort(atol(edtPort1->text()));
    }
#ifdef USE_OPENSSL
    m_client->setUseSSL(chkSSL->isChecked());
    m_client->setUsePlain(chkPlain->isChecked());
#endif
    m_client->setUseVHost(chkVHost->isChecked());
    set_str(&m_client->data.owner.Resource, edtResource->text().utf8());
    m_client->setPriority(atol(edtPriority->text().latin1()));
    if (chkVHost->isChecked())
        set_str(&m_client->data.VHost, edtVHost->text().utf8());
}

void JabberConfig::toggledSSL(bool bState)
{
    unsigned port = atol(edtPort1->text());
    if (m_bConfig)
        port = atol(edtPort->text());
    if (port == 0)
        port = 5222;
    if (bState){
        port++;
    }else{
        port--;
    }
    edtPort->setValue(port);
    edtPort1->setValue(port);
}

void JabberConfig::toggledVHost(bool bState)
{
    edtVHost->setEnabled(bState);
}

void JabberConfig::changed(const QString&)
{
    changed();
}

void JabberConfig::changed()
{
    bool bOK =  !edtID->text().isEmpty() &&
                !edtPasswd->text().isEmpty();
    if (bOK){
        if (m_bConfig){
            bOK = !edtServer->text().isEmpty() &&
                  atol(edtPort->text());
        }else{
            bOK = !edtServer1->text().isEmpty() &&
                  atol(edtPort1->text());
        }
    }
    emit okEnabled(bOK);
}

#ifndef WIN32
#include "jabberconfig.moc"
#endif

