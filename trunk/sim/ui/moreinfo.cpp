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
#include <qdatetime.h>

MoreInfo::MoreInfo(QWidget *p, bool readOnly)
        : MoreInfoBase(p)
{
    btnHomePage->setPixmap(Pict("home"));
    connect(btnHomePage, SIGNAL(clicked()), this, SLOT(goUrl()));
    QDate now = QDate::currentDate();
    spnAge->setSpecialValueText(" ");
    spnAge->setRange(0, 100);
    spnDay->setSpecialValueText(" ");
    spnDay->setRange(0, 31);
    spnYear->setSpecialValueText(" ");
    spnYear->setRange(now.year() - 100, now.year());
    connect(cmbLang1, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang2, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang3, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbMonth, SIGNAL(activated(int)), this, SLOT(birthDayChanged(int)));
    connect(spnDay, SIGNAL(valueChanged(int)), this, SLOT(birthDayChanged(int)));
    connect(spnYear, SIGNAL(valueChanged(int)), this, SLOT(birthDayChanged(int)));
    disableWidget(spnAge);
    if (!readOnly) {
        load(pClient->owner);
        return;
    }
    edtHomePage->setReadOnly(true);
    disableWidget(cmbGender);
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
    if (spnAge->text() == "0") spnAge->setSpecialValueText("");
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
    spnYear->setValue(u->BirthYear);
    birthDayChanged(0);
    initCombo(cmbLang1, u->Language1, languages);
    initCombo(cmbLang2, u->Language2, languages);
    initCombo(cmbLang3, u->Language3, languages);
    setLang(0);
}

void MoreInfo::birthDayChanged(int)
{
    int maxDay = 31;
    int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int year = atol(spnYear->text().latin1());
    int month = cmbMonth->currentItem();
    int day = atol(spnDay->text().latin1());
    if (month){
        maxDay = days[month - 1];
        if ((month == 3) && ((year & 3) == 0)) maxDay = 29;
    }
    spnDay->setRange(0, maxDay);
    if (year && month && day){
        QDate now = QDate::currentDate();
        int age = now.year() - year;
        if ((now.month() < month) || ((now.month() == month) && (now.day() < day))) age--;
        if (age < 100){
            spnAge->setValue(age);
        }else{
            spnAge->setValue(0);
        }
    }else{
        spnAge->setValue(0);
    }
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
    u->Age = atol(spnAge->text().latin1());
    u->Gender = getComboValue(cmbGender, genders);
    set(u->Homepage, edtHomePage->text());
    u->BirthYear = atol(spnYear->text().latin1());
    u->BirthMonth = cmbMonth->currentItem();
    u->BirthDay = atol(spnDay->text().latin1());
    u->Language1 = getComboValue(cmbLang1, languages);
    u->Language2 = getComboValue(cmbLang2, languages);
    u->Language3 = getComboValue(cmbLang3, languages);
}

#ifndef _WINDOWS
#include "moreinfo.moc"
#endif

