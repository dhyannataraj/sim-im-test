/***************************************************************************
                          homeinfo.cpp  -  description
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

#include "homeinfo.h"
#include "country.h"
#include "icqclient.h"
#include "enable.h"
#include "icons.h"
#include "client.h"

#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpixmap.h>

HomeInfo::HomeInfo(QWidget *p, bool readOnly)
        : HomeInfoBase(p)
{
    lblPict->setPixmap(Pict("home"));
    if (!readOnly) {
        load(pClient);
        return;
    }
    edtAddress->setReadOnly(true);
    edtCity->setReadOnly(true);
    edtState->setReadOnly(true);
    edtZip->setReadOnly(true);
    disableWidget(cmbCountry);
    disableWidget(cmbZone);
}

void HomeInfo::load(ICQUser *u)
{
    edtAddress->setText(QString::fromLocal8Bit(u->Address.c_str()));
    edtCity->setText(QString::fromLocal8Bit(u->City.c_str()));
    edtState->setText(QString::fromLocal8Bit(u->State.c_str()));
    edtZip->setText(QString::fromLocal8Bit(u->Zip.c_str()));
    initCombo(cmbCountry, u->Country, countries);
    initTZCombo(cmbZone, u->TimeZone);
}

void HomeInfo::save(ICQUser*)
{
}

void HomeInfo::apply(ICQUser *u)
{
    u->City = edtCity->text().local8Bit();
    u->State = edtState->text().local8Bit();
    u->Address = edtAddress->text().local8Bit();
    u->Zip = edtZip->text().local8Bit();
    u->Country = getComboValue(cmbCountry, countries);
    u->TimeZone = getTZComboValue(cmbZone);
}

#ifndef _WINDOWS
#include "homeinfo.moc"
#endif

