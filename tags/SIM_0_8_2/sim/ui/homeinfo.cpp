/***************************************************************************
                          homeinfo.cpp  -  description
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
#include <qfontmetrics.h>

HomeInfo::HomeInfo(QWidget *p, bool readOnly)
        : HomeInfoBase(p)
{
    edtState->setMaxLength(5);
    if (!readOnly) {
        load(pClient->owner);
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
    set(u->City, edtCity->text());
    set(u->State, edtState->text());
    set(u->Address, edtAddress->text());
    set(u->Zip, edtZip->text());
    u->Country = getComboValue(cmbCountry, countries);
    u->TimeZone = getTZComboValue(cmbZone);
}

void HomeInfo::load(ICQGroup*)
{
}

void HomeInfo::save(ICQGroup*)
{
}

#ifndef _WINDOWS
#include "homeinfo.moc"
#endif

