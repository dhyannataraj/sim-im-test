/***************************************************************************
                          proxycfg.cpp  -  description
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

#include "proxycfg.h"
#include "proxy.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>

ProxyConfig::ProxyConfig(QWidget *parent, ProxyPlugin *plugin)
        : ProxyConfigBase(parent)
{
    m_plugin = plugin;
    cmbType->insertItem(i18n("None"));
    cmbType->insertItem("SOCKS4");
    cmbType->insertItem("SOCKS5");
    cmbType->insertItem("HTTPS");
    cmbType->insertItem("HTTP");
    cmbType->setCurrentItem(plugin->getType());
    edtHost->setText(QString::fromLocal8Bit(plugin->getHost()));
    edtPort->setValue(plugin->getPort());
    chkAuth->setChecked(plugin->getAuth());
    edtUser->setText(QString::fromLocal8Bit(plugin->getUser()));
    edtPswd->setText(QString::fromLocal8Bit(plugin->getPassword()));
    typeChanged(plugin->getType());
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    connect(chkAuth, SIGNAL(toggled(bool)), this, SLOT(authToggled(bool)));
}

void ProxyConfig::apply()
{
    m_plugin->setType(cmbType->currentItem());
    m_plugin->setHost(edtHost->text().local8Bit());
    m_plugin->setPort(atol(edtPort->text()));
    m_plugin->setAuth(chkAuth->isChecked());
    m_plugin->setUser(edtUser->text().local8Bit());
    m_plugin->setPassword(edtPswd->text().local8Bit());
}

void ProxyConfig::typeChanged(int type)
{
    if (type){
        edtHost->show();
        edtPort->show();
        lblHost->show();
        lblPort->show();
    }else{
        edtHost->hide();
        edtPort->hide();
        lblHost->hide();
        lblPort->hide();
    }
    if (type > 1){
        chkAuth->show();
        edtUser->show();
        edtPswd->show();
        lblUser->show();
        lblPswd->show();
    }else{
        chkAuth->hide();
        edtUser->hide();
        edtPswd->hide();
        lblUser->hide();
        lblPswd->hide();
    }
    if ((unsigned)type == PROXY_HTTP){
        lblNote->setText(i18n("<b>Note !</b><br>HTTP-proxy support only ICQ-protocol"));
    }else{
        lblNote->setText("");
    }
    authToggled(chkAuth->isChecked());
}

void ProxyConfig::authToggled(bool bState)
{
    edtUser->setEnabled(bState);
    edtPswd->setEnabled(bState);
    lblUser->setEnabled(bState);
    lblPswd->setEnabled(bState);
}

#ifndef WIN32
#include "proxycfg.moc"
#endif

