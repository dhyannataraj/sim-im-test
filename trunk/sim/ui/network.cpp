/***************************************************************************
                          network.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "network.h"
#include "client.h"
#include "icons.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qcheckbox.h>

NetworkSetup::NetworkSetup(QWidget *p)
        : NetworkBase(p)
{
    lblPict->setPixmap(Pict("network"));
    edtPort->setValidator(new QIntValidator(1024, 0xFFFF, edtPort));

    edtMinPort->setValidator(new QIntValidator(1024, 0xFFFF, edtMinPort));
    edtMaxPort->setValidator(new QIntValidator(1024, 0xFFFF, edtMaxPort));

    edtHost->setText(QString::fromLocal8Bit(pClient->ServerHost.c_str()));
    edtPort->setText(QString::number(pClient->ServerPort()));

    edtMinPort->setText(QString::number(pClient->MinTCPPort()));
    edtMaxPort->setText(QString::number(pClient->MaxTCPPort()));

    cmbProxy->insertItem(i18n("No proxy"));
    cmbProxy->insertItem(i18n("SOCKS4"));
    cmbProxy->insertItem(i18n("SOCKS5"));
    cmbProxy->insertItem(i18n("HTTP"));
    cmbProxy->setCurrentItem(pClient->ProxyType());

    edtProxyHost->setText(QString::fromLocal8Bit(pClient->ProxyHost.c_str()));
    edtProxyPort->setText(QString::number(pClient->ProxyPort()));
    edtProxyPort->setValidator(new QIntValidator(1, 0xFFFF, edtProxyPort));

    edtProxyUser->setText(QString::fromLocal8Bit(pClient->ProxyUser.c_str()));
    edtProxyPasswd->setText(QString::fromLocal8Bit(pClient->ProxyPasswd.c_str()));

    chkProxyAuth->setChecked(pClient->ProxyAuth());

    connect(cmbProxy, SIGNAL(activated(int)), this, SLOT(proxyChanged(int)));
    connect(chkProxyAuth, SIGNAL(toggled(bool)), this, SLOT(proxyChanged(bool)));

    proxyChanged(0);
}

void NetworkSetup::apply(ICQUser*)
{
    pClient->ServerHost = edtHost->text().local8Bit();
    pClient->ServerPort = edtPort->text().toUInt();
    pClient->MinTCPPort = edtMinPort->text().toUInt();
    pClient->MaxTCPPort = edtMaxPort->text().toUInt();
    pClient->ProxyType = cmbProxy->currentItem();
    pClient->ProxyHost = edtProxyHost->text().local8Bit();
    pClient->ProxyPort = edtProxyPort->text().toUInt();
    pClient->ProxyUser = edtProxyUser->text().local8Bit();
    pClient->ProxyPasswd = edtProxyPasswd->text().local8Bit();
    pClient->ProxyAuth = chkProxyAuth->isChecked();
    pClient->setupProxy();
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

