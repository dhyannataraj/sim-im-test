/***************************************************************************
                          workinfo.cpp  -  description
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

#include "workinfo.h"
#include "icqclient.h"
#include "icons.h"
#include "mainwin.h"
#include "enable.h"
#include "client.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>

WorkInfo::WorkInfo(QWidget *p, bool readOnly)
        : WorkInfoBase(p)
{
    lblPict->setPixmap(Pict("work"));
    btnSite->setPixmap(Pict("home"));
    connect(btnSite, SIGNAL(clicked()), this, SLOT(goUrl()));
    if (!readOnly) {
        load(pClient->owner);
        return;
    }
    edtAddress->setReadOnly(true);
    edtCity->setReadOnly(true);
    edtState->setReadOnly(true);
    edtZip->setReadOnly(true);
    disableWidget(cmbCountry);
    disableWidget(cmbOccupation);
    edtName->setReadOnly(true);
    edtDept->setReadOnly(true);
    edtPosition->setReadOnly(true);
    edtSite->setReadOnly(true);
}

void WorkInfo::load(ICQUser *u)
{
    edtAddress->setText(QString::fromLocal8Bit(u->WorkAddress.c_str()));
    edtCity->setText(QString::fromLocal8Bit(u->WorkCity.c_str()));
    edtState->setText(QString::fromLocal8Bit(u->WorkState.c_str()));
    edtZip->setText(QString::fromLocal8Bit(u->WorkZip.c_str()));
    initCombo(cmbCountry, u->WorkCountry, countries);
    initCombo(cmbOccupation, u->Occupation, occupations);
    edtName->setText(QString::fromLocal8Bit(u->WorkName.c_str()));
    edtDept->setText(QString::fromLocal8Bit(u->WorkDepartment.c_str()));
    edtPosition->setText(QString::fromLocal8Bit(u->WorkPosition.c_str()));
    edtSite->setText(QString::fromLocal8Bit(u->WorkHomepage.c_str()));
    btnSite->setEnabled(*(u->WorkHomepage.c_str()));
}

void WorkInfo::goUrl()
{
    pMain->goURL(edtSite->text());
}

void WorkInfo::save(ICQUser*)
{
}

void WorkInfo::apply(ICQUser *u)
{
    set(u->WorkCity, edtCity->text());
    set(u->WorkState, edtState->text());
    set(u->WorkAddress, edtAddress->text());
    set(u->WorkZip, edtZip->text());
    u->WorkCountry = getComboValue(cmbCountry, countries);
    set(u->WorkName, edtName->text());
    set(u->WorkDepartment, edtDept->text());
    set(u->WorkPosition, edtPosition->text());
    u->Occupation = getComboValue(cmbOccupation, occupations);
    set(u->WorkHomepage, edtSite->text());
}

#ifndef _WINDOWS
#include "workinfo.moc"
#endif

