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

#include "simapi.h"
#include "homeinfo.h"
#include "icqclient.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>

HomeInfo::HomeInfo(QWidget *parent, struct ICQUserData *data, ICQClient *client)
        : HomeInfoBase(parent)
{
    m_data   = data;
    m_client = client;
    if (m_data){
        edtAddress->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        disableWidget(cmbCountry);
        disableWidget(cmbZone);
    }
    fill();
}

void HomeInfo::apply()
{
}

void HomeInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    set_str(&data->Address, m_client->fromUnicode(edtAddress->text(), NULL).c_str());
    set_str(&data->City, m_client->fromUnicode(edtCity->text(), NULL).c_str());
    set_str(&data->State, m_client->fromUnicode(edtState->text(), NULL).c_str());
    set_str(&data->Zip, m_client->fromUnicode(edtZip->text(), NULL).c_str());
    data->Country = getComboValue(cmbCountry, getCountries());
}

void *HomeInfo::processEvent(Event *e)
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

static QString formatTime(char n)
{
    QString res;
    res.sprintf("%+i:%02u", -n/2, (n & 1) * 30);
    return res;
}

static void initTZCombo(QComboBox *cmb, char tz)
{
    if (tz < -24) tz = 0;
    if (tz > 24) tz = 0;
    if (cmb->isEnabled()){
        unsigned nSel = 12;
        unsigned n = 0;
        for (char i = 24; i >= -24; i--, n++){
            cmb->insertItem(formatTime(i));
            if (i == tz) nSel = n;
        }
        cmb->setCurrentItem(nSel);
    }else{
        cmb->insertItem(formatTime(tz));
    }
}

void HomeInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtAddress->setText(m_client->toUnicode(data->Address, data));
    edtCity->setText(m_client->toUnicode(data->City, data));
    edtState->setText(m_client->toUnicode(data->State, data));
    edtZip->setText(m_client->toUnicode(data->Zip, data));
    initCombo(cmbCountry, data->Country, getCountries());
    initTZCombo(cmbZone, data->TimeZone);
}

#ifndef WIN32
#include "homeinfo.moc"
#endif

