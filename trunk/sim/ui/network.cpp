/***************************************************************************
                          network.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "network.h"
#include "sockets.h"
#include "client.h"
#include "icons.h"
#include "enable.h"
#include "mainwin.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qcheckbox.h>

NetworkSetup::NetworkSetup(QWidget *p)
        : NetworkBase(p)
{
    edtPort->setValidator(new QIntValidator(1024, 0xFFFF, edtPort));

    edtMinPort->setValidator(new QIntValidator(1024, 0xFFFF, edtMinPort));
    edtMaxPort->setValidator(new QIntValidator(1024, 0xFFFF, edtMaxPort));

    edtHost->setText(QString::fromLocal8Bit(pClient->ServerHost.c_str()));
    edtPort->setText(QString::number(pClient->ServerPort));

    SocketFactory *factory = pClient->factory();

    edtMinPort->setText(QString::number(factory->MinTCPPort));
    edtMaxPort->setText(QString::number(factory->MaxTCPPort));

    cmbProxy->insertItem(i18n("No proxy"));
    cmbProxy->insertItem(i18n("SOCKS4"));
    cmbProxy->insertItem(i18n("SOCKS5"));
    cmbProxy->insertItem(i18n("HTTP"));
    cmbProxy->insertItem(i18n("HTTPS"));
    cmbProxy->setCurrentItem((int)(factory->ProxyType));

    edtProxyHost->setText(QString::fromLocal8Bit(factory->ProxyHost.c_str()));
    edtProxyPort->setText(QString::number(factory->ProxyPort));
    edtProxyPort->setValidator(new QIntValidator(1, 0xFFFF, edtProxyPort));

    edtProxyUser->setText(QString::fromLocal8Bit(factory->ProxyUser.c_str()));
    edtProxyPasswd->setText(QString::fromLocal8Bit(factory->ProxyPasswd.c_str()));
    edtProxyPasswd->setEchoMode(QLineEdit::Password);

    chkProxyAuth->setChecked(factory->ProxyAuth);

    connect(cmbProxy, SIGNAL(activated(int)), this, SLOT(proxyChanged(int)));
    connect(chkProxyAuth, SIGNAL(toggled(bool)), this, SLOT(proxyChanged(bool)));

    proxyChanged(0);
}

void NetworkSetup::apply(ICQUser*)
{
    set(pClient->ServerHost, edtHost->text());
    pClient->ServerPort = edtPort->text().toUInt();
    SocketFactory *factory = pClient->factory();
    factory->MinTCPPort = edtMinPort->text().toUInt();
    factory->MaxTCPPort = edtMaxPort->text().toUInt();
    set(factory->ProxyHost, edtProxyHost->text());
    factory->ProxyPort = edtProxyPort->text().toUInt();
    set(factory->ProxyUser, edtProxyUser->text());
    set(factory->ProxyPasswd, edtProxyPasswd->text());
    factory->ProxyAuth = chkProxyAuth->isChecked();
    if (factory->ProxyType != (PROXY_TYPE)(cmbProxy->currentItem())){
        pClient->setStatus(ICQ_STATUS_OFFLINE);
        factory->ProxyType = (PROXY_TYPE)(cmbProxy->currentItem());
        pMain->realSetStatus();
    }
}

void NetworkSetup::proxyChanged(bool)
{
    proxyChanged(0);
}

void NetworkSetup::proxyChanged(int)
{
    int proxyType = cmbProxy->currentItem();
    lblProxyHost->setEnabled(proxyType > 0);
    lblProxyPort->setEnabled(proxyType > 0);
    edtProxyHost->setEnabled(proxyType > 0);
    edtProxyPort->setEnabled(proxyType > 0);
    chkProxyAuth->setEnabled(proxyType > 1);
    lblProxyUser->setEnabled((proxyType > 1) && chkProxyAuth->isChecked());
    lblProxyPasswd->setEnabled((proxyType > 1) && chkProxyAuth->isChecked());
    edtProxyUser->setEnabled((proxyType > 1) && chkProxyAuth->isChecked());
    edtProxyPasswd->setEnabled((proxyType > 1) && chkProxyAuth->isChecked());
}

#ifndef _WINDOWS
#include "network.moc"
#endif

