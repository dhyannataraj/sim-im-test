/***************************************************************************
                          icqconfig.cpp  -  description
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

#include "icqconfig.h"
#include "icq.h"

#include <qtimer.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <qtabwidget.h>
#include <qcombobox.h>

ICQConfig::ICQConfig(QWidget *parent, ICQClient *client, bool bConfig)
        : ICQConfigBase(parent)
{
    m_client = client;
    m_bConfig = bConfig;
    if (m_bConfig){
        QTimer::singleShot(0, this, SLOT(changed()));
        connect(chkNew, SIGNAL(toggled(bool)), this, SLOT(newToggled(bool)));
        if (m_client->data.owner.Uin.value){
            edtUin->setText(QString::number(m_client->data.owner.Uin.value));
            chkNew->setChecked(false);
        }else{
            chkNew->setChecked(true);
        }
        edtPasswd->setText(m_client->getPassword());
        edtUin->setValidator(new QIntValidator(1000, 0x1FFFFFFF, edtUin));
        connect(edtUin, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        connect(edtPasswd, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    }else{
        tabConfig->removePage(tabICQ);
    }
    edtServer->setText(QString::fromLocal8Bit(m_client->getServer()));
    edtPort->setValue(m_client->getPort());
    edtMinPort->setValue(m_client->getMinPort());
    edtMaxPort->setValue(m_client->getMaxPort());
    connect(edtServer, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPort, SIGNAL(valueChanged(const QString&)), this, SLOT(changed(const QString&)));
    cmbFormat->insertItem(i18n("RTF"));
    cmbFormat->insertItem(i18n("UTF"));
    cmbFormat->insertItem(i18n("Plain text"));
    cmbFormat->setCurrentItem(client->getSendFormat());
	chkPlugins->setChecked(client->getDisablePlugins());
    chkUpdate->setChecked(client->getDisableAutoUpdate());
    chkAutoReply->setChecked(client->getDisableAutoReplyUpdate());
    chkTyping->setChecked(client->getDisableTypingNotification());
	chkInvisible->setChecked(client->getAutoCheckInvisible());
	edtInvisible->setValue(client->getCheckInvisibleInterval());
    chkDND->setChecked(client->getAcceptInDND());
    chkOccupied->setChecked(client->getAcceptInOccupied());
    chkHTTP->setChecked(client->getUseHTTP());
    connect(chkAuto, SIGNAL(toggled(bool)), this, SLOT(autoToggled(bool)));
	connect(chkInvisible, SIGNAL(toggled(bool)), this, SLOT(invisibleToggled(bool)));
    chkAuto->setChecked(client->getAutoHTTP());
    chkKeepAlive->setChecked(client->getKeepAlive());
    cmbAck->setCurrentItem(client->getAckMode());
	invisibleToggled(client->getAutoCheckInvisible());
}

void ICQConfig::autoToggled(bool bState)
{
    chkHTTP->setEnabled(!bState);
}

void ICQConfig::invisibleToggled(bool bState)
{
	lblInvisible->setEnabled(bState);
	lblInvisible2->setEnabled(bState);
	edtInvisible->setEnabled(bState);
}

void ICQConfig::apply(Client*, void*)
{
}

void ICQConfig::apply()
{
    if (m_bConfig){
        m_client->setUin(atol(edtUin->text().latin1()));
        m_client->setPassword(edtPasswd->text());
    }
    m_client->setServer(edtServer->text().local8Bit());
    m_client->setPort((unsigned short)atol(edtPort->text()));
    m_client->setMinPort((unsigned short)atol(edtMinPort->text()));
    m_client->setMaxPort((unsigned short)atol(edtMaxPort->text()));
    m_client->setSendFormat(cmbFormat->currentItem());
    m_client->setDisablePlugins(chkPlugins->isChecked());
    m_client->setDisableAutoUpdate(chkUpdate->isChecked());
    m_client->setDisableAutoReplyUpdate(chkAutoReply->isChecked());
    m_client->setDisableTypingNotification(chkTyping->isChecked());
	m_client->setAutoCheckInvisible(chkInvisible->isChecked());
	m_client->setCheckInvisibleInterval(atol(edtInvisible->text().latin1()));
    m_client->setAcceptInDND(chkDND->isChecked());
    m_client->setAcceptInOccupied(chkOccupied->isChecked());
    m_client->setUseHTTP(chkHTTP->isChecked());
    m_client->setAutoHTTP(chkAuto->isChecked());
    m_client->setKeepAlive(chkKeepAlive->isChecked());
    m_client->setAckMode(cmbAck->currentItem());
}

void ICQConfig::changed(const QString&)
{
    changed();
}

void ICQConfig::newToggled(bool bNew)
{
    if (bNew)
        edtUin->setText("");
    lblUin->setEnabled(!bNew);
    edtUin->setEnabled(!bNew);
}

void ICQConfig::changed()
{
    bool bOK = true;
    if (!chkNew->isChecked())
        bOK = atol(edtUin->text().latin1()) > 1000;
    bOK =  bOK && !edtPasswd->text().isEmpty() &&
           !edtServer->text().isEmpty() &&
           atol(edtPort->text());
    emit okEnabled(bOK);
}

#ifndef WIN32
#include "icqconfig.moc"
#endif


