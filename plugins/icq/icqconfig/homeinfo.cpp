/***************************************************************************
                          homeinfo.cpp  -  description
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

#include "homeinfo.h"
#include "icqclient.h"
#include "log.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

using namespace SIM;

HomeInfo::HomeInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) : QWidget(parent)
{
    m_ui = new Ui::HomeInfo;
	m_ui->setupUi(this);
    m_client  = client;
    m_contact = contact;
    if(contact != client->ownerContact())
    {
        m_ui->edtAddress->setReadOnly(true);
        m_ui->edtCity->setReadOnly(true);
        m_ui->edtState->setReadOnly(true);
        m_ui->edtZip->setReadOnly(true);
        m_ui->cmbCountry->setEnabled(false);
        m_ui->cmbZone->setEnabled(false);
    }
    fill();
    m_ui->btnWebLocation->setText(i18n("map"));
//    connect(m_ui->btnWebLocation, SIGNAL(clicked()), this, SLOT(goUrl()));
}

HomeInfo::~HomeInfo()
{
    delete m_ui;
}

//void HomeInfo::apply()
//{
//}
//
//void HomeInfo::updateData(ICQUserData* data)
//{
//    data->setAddress(edtAddress->toPlainText());
//    data->setCity(edtCity->text());
//    data->setState(edtState->text());
//    data->setZip(edtZip->text());
//    data->setCountry(getComboValue(cmbCountry, getCountries()));
//}
//
//void HomeInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
//{
//    if (client != m_client)
//        return;
//    updateData(m_client->toICQUserData(contact));
//}
//
//void HomeInfo::apply(Client *client, void *_data)
//{
//    if (client != m_client)
//        return;
//    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
//    updateData(data);
//}
//
//bool HomeInfo::processEvent(Event *e)
//{
//    if (e->type() == eEventContact){
//        EventContact *ec = static_cast<EventContact*>(e);
//        if(ec->action() != EventContact::eChanged)
//            return false;
//        Contact *contact = ec->contact();
//        if (contact->have(m_data))
//            fill();
//    } else
//    if ((e->type() == eEventClientChanged) && (m_data == 0)){
//        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
//        if (ecc->client() == m_client)
//            fill();
//    }
//    return false;
//}
//
//static QString formatTime(char n)
//{
//    QString res;
//    res.sprintf("%+i:%02u", -n/2, (n & 1) * 30);
//    return res;
//}

static void initTZCombo(QComboBox *cmb, char tz)
{
    if(tz < -24)
        tz = 0;
    if(tz > 24)
        tz = 0;
    if(cmb->isEnabled())
    {
        unsigned nSel = 12;
        unsigned n = 0;
        for(char i = 24; i >= -24; i--, n++)
        {
            QString num = QString::number(i);
            if(i > 0)
            {
                num.prepend("+");
            }
            cmb->addItem(num);
            if(i == tz)
                nSel = n;
        }
        cmb->setCurrentIndex(nSel);
    }
    else
    {
        QString num = QString::number(tz);
        if(tz > 0)
        {
            num.prepend("+");
        }
        cmb->addItem(num);
    }
}

void HomeInfo::fill()
{
    m_ui->edtAddress->setPlainText(m_contact->getAddress());
    m_ui->edtCity->setText(m_contact->getCity());
    m_ui->edtState->setText(m_contact->getState());
    m_ui->edtZip->setText(m_contact->getZip());
    initCountryCombobox();
    initTZCombo(m_ui->cmbZone, m_contact->getTimeZone());
}

void HomeInfo::initCountryCombobox()
{
    m_ui->cmbCountry->clear();
    int currentIndex = -1;
    int i = 0;
    for(const ext_info* countryInfo = getCountries(); countryInfo->nCode; countryInfo++, i++)
    {
        m_ui->cmbCountry->addItem(countryInfo->szName);
        if(countryInfo->nCode == m_contact->getCountry())
        {
            currentIndex = i;
        }
    }
    m_ui->cmbCountry->addItem("Not specified"); // For not specified country
    if(currentIndex >= 0)
        m_ui->cmbCountry->setCurrentIndex(currentIndex);
    else
        m_ui->cmbCountry->setCurrentIndex(m_ui->cmbCountry->count() - 1);
}

Ui::HomeInfo* HomeInfo::ui()
{
    return m_ui;
}

//
//void HomeInfo::goUrl()
//{
//    ICQUserData *data = m_data;
//    if (data == NULL)
//        data = &m_client->data.owner;
//    QString url = QString("http://www.mapquest.com/maps/map.adp?city=%1&state=%2&country=%3&zip=%4")
//                    .arg(edtCity->text())
//                    .arg(edtState->text())
//                    .arg(cmbCountry->currentText())
//                    .arg(edtZip->text());
//    EventGoURL e(url);
//    e.process();
//}

