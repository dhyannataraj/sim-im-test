/***************************************************************************
                          moreinfo.cpp  -  description
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

#include "moreinfo.h"
#include "icqclient.h"
#include "icons.h"
#include "mainwin.h"
#include "country.h"
#include "enable.h"
#include "client.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>

MoreInfo::MoreInfo(QWidget *p, bool readOnly)
        : MoreInfoBase(p)
{
    lblPict->setPixmap(Pict("more"));
    btnHomePage->setPixmap(Pict("home"));
    connect(btnHomePage, SIGNAL(clicked()), this, SLOT(goUrl()));
    spnDay->setMaxValue(31);
    spnYear->setMaxValue(2000);
    connect(cmbLang1, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang2, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang3, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    if (!readOnly) {
        load(&pClient->owner);
        return;
    }
    edtHomePage->setReadOnly(true);
    disableWidget(cmbGender);
    disableWidget(spnAge);
    disableWidget(cmbMonth);
    disableWidget(spnDay);
    disableWidget(spnYear);
    disableWidget(cmbLang1);
    disableWidget(cmbLang2);
    disableWidget(cmbLang3);
}

void MoreInfo::load(ICQUser *u)
{
    edtHomePage->setText(QString::fromLocal8Bit(u->Homepage.c_str()));
    btnHomePage->setEnabled(*(u->Homepage.c_str()));
    initCombo(cmbGender, u->Gender, genders);
    spnAge->setValue(u->Age);
    if (spnAge->value() == 0) spnAge->setSpecialValueText("");
    cmbMonth->insertItem("");
    cmbMonth->insertItem(i18n("January"));
    cmbMonth->insertItem(i18n("February"));
    cmbMonth->insertItem(i18n("March"));
    cmbMonth->insertItem(i18n("April"));
    cmbMonth->insertItem(i18n("May"));
    cmbMonth->insertItem(i18n("June"));
    cmbMonth->insertItem(i18n("July"));
    cmbMonth->insertItem(i18n("August"));
    cmbMonth->insertItem(i18n("September"));
    cmbMonth->insertItem(i18n("October"));
    cmbMonth->insertItem(i18n("November"));
    cmbMonth->insertItem(i18n("December"));
    cmbMonth->setCurrentItem(u->BirthMonth);
    spnDay->setValue(u->BirthDay);
    if (spnDay->value() == 0) spnAge->setSpecialValueText("");
    spnYear->setValue(u->BirthYear);
    if (spnYear->value() == 0) spnYear->setSpecialValueText("");
    initCombo(cmbLang1, u->Language1, languages);
    initCombo(cmbLang2, u->Language2, languages);
    initCombo(cmbLang3, u->Language3, languages);
    setLang(0);
}

void MoreInfo::goUrl()
{
    pMain->goURL(edtHomePage->text());
}

void MoreInfo::save(ICQUser*)
{
}

void MoreInfo::setLang(int)
{
    unsigned l[3], sl[3];
    l[0] = cmbLang1->currentItem();
    l[1] = cmbLang2->currentItem();
    l[2] = cmbLang3->currentItem();
    unsigned j = 0;
    for (unsigned i = 0; i < 3; i++)
        if (l[i]) sl[j++] = l[i];
    for (; j < 3; j++)
        sl[j] = 0;
    cmbLang1->setCurrentItem(sl[0]);
    cmbLang2->setCurrentItem(sl[1]);
    cmbLang3->setCurrentItem(sl[2]);
    cmbLang2->setEnabled(sl[0] != 0);
    cmbLang3->setEnabled(sl[1] != 0);
}

void MoreInfo::apply(ICQUser *u)
{
    u->Age = spnAge->value();
    u->Gender = getComboValue(cmbGender, genders);
    set(u->Homepage, edtHomePage->text());
    u->BirthYear = spnYear->value();
    u->BirthMonth = cmbMonth->currentItem();
    u->BirthDay = spnDay->value();
    u->Language1 = getComboValue(cmbLang1, languages);
    u->Language2 = getComboValue(cmbLang2, languages);
    u->Language3 = getComboValue(cmbLang3, languages);
}

#ifndef _WINDOWS
#include "moreinfo.moc"
#endif

