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
    if (m_client->data.owner.Resource.ptr)
        edtResource->setText(QString::fromUtf8(m_client->data.owner.Resource.ptr));
    if (m_client->data.VHost.ptr)
        edtVHost->setText(QString::fromUtf8(m_client->data.VHost.ptr));
    if (m_bConfig){
        tabCfg->removePage(tabJabber);
    }else{
        lblServer->hide();
        edtServer->hide();
        lblPort->hide();
        edtPort->hide();
        chkSSL1->hide();
        edtServer1->setText(i18n("jabber.org"));
        edtPort1->setValue(m_client->getPort());
    }
#ifdef USE_OPENSSL
    chkSSL->setChecked(m_client->getUseSSL());
    chkSSL1->setChecked(m_client->getUseSSL());
    chkPlain->setChecked(m_client->getUsePlain());
#else
    chkSSL1->hide();
    chkSSL->hide();
    chkPlain->hide();
#endif
    edtMinPort->setValue(m_client->getMinPort());
    edtMaxPort->setValue(m_client->getMaxPort());
    chkVHost->setChecked(m_client->getUseVHost());
    chkTyping->setChecked(m_client->getTyping());
    chkRichText->setChecked(m_client->getRichText());
    chkIcons->setChecked(m_client->getProtocolIcons());
    chkSubscribe->setChecked(m_client->getAutoSubscribe());
    chkAccept->setChecked(m_client->getAutoAccept());
    lnkPublic->setText(i18n("List of public servers"));
    lnkPublic->setUrl("http://www.jabber.org/user/publicservers.php");
    connect(edtID, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(chkSSL, SIGNAL(toggled(bool)), this, SLOT(toggledSSL(bool)));
    connect(chkSSL1, SIGNAL(toggled(bool)), this, SLOT(toggledSSL(bool)));
    connect(chkVHost, SIGNAL(toggled(bool)), this, SLOT(toggledVHost(bool)));
    chkHTTP->setChecked(m_client->getUseHTTP());
    edtUrl->setText(m_client->getURL());
    lnkRich->setText("(JEP-0071)");
    lnkRich->setUrl("http://www.jabber.org/jeps/jep-0071.html");
    lnkTyping->setText("(JEP-0085)");
    lnkTyping->setUrl("http://www.jabber.org/jeps/jep-0085.html");
}

void JabberConfig::apply(Client*, void*)
{
}

void JabberConfig::apply()
{
    if (m_bConfig){
        m_client->setServer(edtServer->text().local8Bit());
        m_client->setPort((unsigned short)atol(edtPort->text()));
    }else{
        m_client->setServer(edtServer1->text().local8Bit());
        m_client->setPort((unsigned short)atol(edtPort1->text()));
    }
    m_client->setUseVHost(false);
    if (chkVHost->isChecked()){
        set_str(&m_client->data.VHost.ptr, edtVHost->text().utf8());
        if (!edtVHost->text().isEmpty())
            m_client->setUseVHost(true);
    }
    QString jid = edtID->text();
    int n = jid.find('@');
    if (n < 0){
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
    }else{
        QString host = jid.mid(n + 1);
        set_str(&m_client->data.VHost.ptr, host.utf8());
        m_client->setUseVHost(true);
    }
    if (!m_bConfig){
        m_client->setID(jid);
        m_client->setPassword(edtPasswd->text());
        m_client->setRegister(chkRegister->isChecked());
    }
#ifdef USE_OPENSSL
    if (m_bConfig){
        m_client->setUseSSL(chkSSL1->isChecked());
    }else{
        m_client->setUseSSL(chkSSL->isChecked());
    }
    m_client->setUsePlain(chkPlain->isChecked());
#endif
    m_client->setMinPort((unsigned short)atol(edtMinPort->text().latin1()));
    m_client->setMaxPort((unsigned short)atol(edtMaxPort->text().latin1()));
    m_client->setTyping(chkTyping->isChecked());
    m_client->setRichText(chkRichText->isChecked());
    m_client->setAutoSubscribe(chkSubscribe->isChecked());
    m_client->setAutoAccept(chkAccept->isChecked());
    if (m_client->getProtocolIcons() != chkIcons->isChecked()){
        m_client->setProtocolIcons(chkIcons->isChecked());
        Event e(EventRepaintView);
        e.process();
    }
    set_str(&m_client->data.owner.Resource.ptr, edtResource->text().utf8());
    m_client->setPriority(atol(edtPriority->text().latin1()));
    m_client->setUseHTTP(chkHTTP->isChecked());
    m_client->setURL(edtUrl->text().latin1());
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

