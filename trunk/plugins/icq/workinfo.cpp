/***************************************************************************
                          workinfo.cpp  -  description
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
#include "workinfo.h"
#include "icqclient.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>

WorkInfo::WorkInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client)
        : WorkInfoBase(parent)
{
    m_data    = data;
    m_client  = client;
    m_contact = contact;
    btnSite->setPixmap(Pict("home"));
    connect(btnSite, SIGNAL(clicked()), this, SLOT(goUrl()));
    if (m_data){
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
    }else{
        connect(edtSite, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    }
    fill();
}

void WorkInfo::apply()
{
}

void *WorkInfo::processEvent(Event *e)
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

static const ext_info occupations[] =
    {
        { I18N_NOOP("Academic"), 1  },
        { I18N_NOOP("Administrative"), 2  },
        { I18N_NOOP("Art/Entertainment"), 3  },
        { I18N_NOOP("College Student"), 4  },
        { I18N_NOOP("Computers"), 5  },
        { I18N_NOOP("Community & Social"), 6  },
        { I18N_NOOP("Education"), 7  },
        { I18N_NOOP("Engineering"), 8  },
        { I18N_NOOP("Financial Services"), 9  },
        { I18N_NOOP("Government"), 10  },
        { I18N_NOOP("High School Student"), 11  },
        { I18N_NOOP("Home"), 12  },
        { I18N_NOOP("ICQ - Providing Help"), 13  },
        { I18N_NOOP("Law"), 14  },
        { I18N_NOOP("Managerial"), 15  },
        { I18N_NOOP("Manufacturing"), 16  },
        { I18N_NOOP("Medical/Health"), 17  },
        { I18N_NOOP("Military"), 18  },
        { I18N_NOOP("Non-Goverment Organisation"), 19  },
        { I18N_NOOP("Other services"), 20  },
        { I18N_NOOP("Professional"), 21  },
        { I18N_NOOP("Retail"), 22  },
        { I18N_NOOP("Retired"), 23  },
        { I18N_NOOP("Science & Research"), 24  },
        { I18N_NOOP("Sports"), 25  },
        { I18N_NOOP("Technical"), 26  },
        { I18N_NOOP("University student"), 27  },
        { I18N_NOOP("Web building"), 28  },
        { "", 0  }
    };

const ext_info *p_occupations = occupations;

void WorkInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    Contact *contact = getContacts()->contact(m_contact);
    edtAddress->setText(getContacts()->toUnicode(contact, data->WorkAddress.ptr));
    edtCity->setText(getContacts()->toUnicode(contact, data->WorkCity.ptr));
    edtState->setText(getContacts()->toUnicode(contact, data->WorkState.ptr));
    edtZip->setText(getContacts()->toUnicode(contact, data->WorkZip.ptr));
    initCombo(cmbCountry, (unsigned short)(data->WorkCountry.value), getCountries());
    initCombo(cmbOccupation, (unsigned short)(data->Occupation.value), occupations);
    edtName->setText(getContacts()->toUnicode(contact, data->WorkName.ptr));
    edtDept->setText(getContacts()->toUnicode(contact, data->WorkDepartment.ptr));
    edtPosition->setText(getContacts()->toUnicode(contact, data->WorkPosition.ptr));
    edtSite->setText(getContacts()->toUnicode(contact, data->WorkHomepage.ptr));
    urlChanged(edtSite->text());
}

void WorkInfo::goUrl()
{
    QString url = edtSite->text();
    if (url.isEmpty())
        return;
    Event e(EventGoURL, (void*)(const char*)(url.local8Bit()));
    e.process();
}

void WorkInfo::urlChanged(const QString &text)
{
    btnSite->setEnabled(!text.isEmpty());
}

void WorkInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    set_str(&data->WorkAddress.ptr, getContacts()->fromUnicode(NULL, edtAddress->text()).c_str());
    set_str(&data->WorkCity.ptr, getContacts()->fromUnicode(NULL, edtCity->text()).c_str());
    set_str(&data->WorkState.ptr, getContacts()->fromUnicode(NULL, edtState->text()).c_str());
    set_str(&data->WorkZip.ptr, getContacts()->fromUnicode(NULL, edtZip->text()).c_str());
    data->WorkCountry.value = getComboValue(cmbCountry, getCountries());
    data->Occupation.value = getComboValue(cmbOccupation, occupations);
    set_str(&data->WorkName.ptr, getContacts()->fromUnicode(NULL, edtName->text()).c_str());
    set_str(&data->WorkDepartment.ptr, getContacts()->fromUnicode(NULL, edtDept->text()).c_str());
    set_str(&data->WorkPosition.ptr, getContacts()->fromUnicode(NULL, edtPosition->text()).c_str());
    set_str(&data->WorkHomepage.ptr, getContacts()->fromUnicode(NULL, edtSite->text()).c_str());
}

#ifndef WIN32
#include "workinfo.moc"
#endif

