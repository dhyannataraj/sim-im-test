/***************************************************************************
                          moreinfo.cpp  -  description
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

#include "simapi.h"
#include "moreinfo.h"
#include "icqclient.h"
#include "datepicker.h"

#include <qpushbutton.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

MoreInfo::MoreInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client)
        : MoreInfoBase(parent)
{
    m_data    = data;
    m_client  = client;
	m_contact = contact;
    btnHomePage->setPixmap(Pict("home"));
    connect(btnHomePage, SIGNAL(clicked()), this, SLOT(goUrl()));
    QDate now = QDate::currentDate();
    spnAge->setSpecialValueText(" ");
    spnAge->setRange(0, 100);
    connect(cmbLang1, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang2, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang3, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(edtDate, SIGNAL(changed()), this, SLOT(birthDayChanged()));
    if (m_data){
        disableWidget(spnAge);
        edtHomePage->setReadOnly(true);
        disableWidget(cmbGender);
        disableWidget(edtDate);
        disableWidget(cmbLang1);
        disableWidget(cmbLang2);
        disableWidget(cmbLang3);
    }else{
        connect(edtHomePage, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    }
    fill();
}

void MoreInfo::apply()
{
}

void *MoreInfo::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact->clientData.have(m_data))
            fill();
    }
    if ((e->type() == EventClientChanged) && (m_data == 0)){
        Client *client = (Client*)(e->param());
        if (client == m_client)
            fill();
    }
    return NULL;
}

const ext_info genders[] =
    {
        { I18N_NOOP("Female"), 1 },
        { I18N_NOOP("Male"), 2 },
        { "", 0 }
    };

const ext_info *p_genders = genders;

const ext_info languages[] =
    {
        {I18N_NOOP("Arabic"), 1},
        {I18N_NOOP("Bhojpuri"), 2},
        {I18N_NOOP("Bulgarian"), 3},
        {I18N_NOOP("Burmese"), 4},
        {I18N_NOOP("Cantonese"), 5},
        {I18N_NOOP("Catalan"), 6},
        {I18N_NOOP("Chinese"), 7},
        {I18N_NOOP("Croatian"), 8},
        {I18N_NOOP("Czech"), 9},
        {I18N_NOOP("Danish"), 10},
        {I18N_NOOP("Dutch"), 11},
        {I18N_NOOP("English"), 12},
        {I18N_NOOP("Esperanto"), 13},
        {I18N_NOOP("Estonian"), 14},
        {I18N_NOOP("Farsi"), 15},
        {I18N_NOOP("Finnish"), 16},
        {I18N_NOOP("French"), 17},
        {I18N_NOOP("Gaelic"), 18},
        {I18N_NOOP("German"), 19},
        {I18N_NOOP("Greek"), 20},
        {I18N_NOOP("Hebrew"), 21},
        {I18N_NOOP("Hindi"), 22},
        {I18N_NOOP("Hungarian"), 23},
        {I18N_NOOP("Icelandic"), 24},
        {I18N_NOOP("Indonesian"), 25},
        {I18N_NOOP("Italian"), 26},
        {I18N_NOOP("Japanese"), 27},
        {I18N_NOOP("Khmer"), 28},
        {I18N_NOOP("Korean"), 29},
        {I18N_NOOP("Lao"), 30},
        {I18N_NOOP("Latvian"), 31},
        {I18N_NOOP("Lithuanian"), 32},
        {I18N_NOOP("Malay"), 33},
        {I18N_NOOP("Norwegian"), 34},
        {I18N_NOOP("Polish"), 35},
        {I18N_NOOP("Portuguese"), 36},
        {I18N_NOOP("Romanian"), 37},
        {I18N_NOOP("Russian"), 38},
        {I18N_NOOP("Serbian"), 39},
        {I18N_NOOP("Slovak"), 40},
        {I18N_NOOP("Slovenian"), 41},
        {I18N_NOOP("Somali"), 42},
        {I18N_NOOP("Spanish"), 43},
        {I18N_NOOP("Swahili"), 44},
        {I18N_NOOP("Swedish"), 45},
        {I18N_NOOP("Tagalog"), 46},
        {I18N_NOOP("Tatar"), 47},
        {I18N_NOOP("Thai"), 48},
        {I18N_NOOP("Turkish"), 49},
        {I18N_NOOP("Ukrainian"), 50},
        {I18N_NOOP("Urdu"), 51},
        {I18N_NOOP("Vietnamese"), 52},
        {I18N_NOOP("Yiddish"), 53},
        {I18N_NOOP("Yoruba"), 54},
        {I18N_NOOP("Taiwanese"), 55},
        {I18N_NOOP("Afrikaans"), 56},
        {I18N_NOOP("Persian"), 57},
        {I18N_NOOP("Albanian"), 58},
        {I18N_NOOP("Armenian"), 59},
        {"", 0}
    };

const ext_info *p_languages = languages;

void MoreInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtHomePage->setText(getContacts()->toUnicode(getContacts()->contact(m_contact), data->Homepage.ptr));
    initCombo(cmbGender, (unsigned short)(data->Gender.value), genders);
    if (spnAge->text() == "0") spnAge->setSpecialValueText("");
    edtDate->setDate(data->BirthDay.value, data->BirthMonth.value, data->BirthYear.value);
    birthDayChanged();
    unsigned l = data->Language.value;
    char l1 = (char)(l & 0xFF);
    l = l >> 8;
    char l2 = (char)(l & 0xFF);
    l = l >> 8;
    char l3 = (char)(l & 0xFF);
    initCombo(cmbLang1, l1, languages);
    initCombo(cmbLang2, l2, languages);
    initCombo(cmbLang3, l3, languages);
    setLang(0);
    urlChanged(edtHomePage->text());
}

void MoreInfo::birthDayChanged()
{
    int day, month, year;
    edtDate->getDate(day, month, year);
    if (year){
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
    QString url = edtHomePage->text();
    if (url.isEmpty())
        return;
    string url_str;
    url_str = url.local8Bit();
    Event e(EventGoURL, (void*)url_str.c_str());
    e.process();
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

void MoreInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    set_str(&data->Homepage.ptr, getContacts()->fromUnicode(NULL, edtHomePage->text()).c_str());
    data->Gender.value = getComboValue(cmbGender, genders);
    int day, month, year;
    edtDate->getDate(day, month, year);
    data->BirthMonth.value = month;
    data->BirthDay.value   = day;
    data->BirthYear.value  = year;
    unsigned l1 = getComboValue(cmbLang1, languages);
    unsigned l2 = getComboValue(cmbLang2, languages);
    unsigned l3 = getComboValue(cmbLang3, languages);
    data->Language.value = (l3 << 16) | (l2 << 8) | l1;
}

void MoreInfo::urlChanged(const QString &text)
{
    btnHomePage->setEnabled(!text.isEmpty());
}

#ifndef WIN32
#include "moreinfo.moc"
#endif

