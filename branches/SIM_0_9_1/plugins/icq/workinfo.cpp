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

WorkInfo::WorkInfo(QWidget *parent, struct ICQUserData *data, ICQClient *client)
        : WorkInfoBase(parent)
{
    m_data   = data;
    m_client = client;
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

void WorkInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtAddress->setText(m_client->toUnicode(data->WorkAddress, data));
    edtCity->setText(m_client->toUnicode(data->WorkCity, data));
    edtState->setText(m_client->toUnicode(data->WorkState, data));
    edtZip->setText(m_client->toUnicode(data->WorkZip, data));
    initCombo(cmbCountry, (unsigned short)(data->WorkCountry), getCountries());
    initCombo(cmbOccupation, (unsigned short)(data->Occupation), occupations);
    edtName->setText(m_client->toUnicode(data->WorkName, data));
    edtDept->setText(m_client->toUnicode(data->WorkDepartment, data));
    edtPosition->setText(m_client->toUnicode(data->WorkPosition, data));
    edtSite->setText(m_client->toUnicode(data->WorkHomepage, data));
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
    set_str(&data->WorkAddress, m_client->fromUnicode(edtAddress->text(), NULL).c_str());
    set_str(&data->WorkCity, m_client->fromUnicode(edtCity->text(), NULL).c_str());
    set_str(&data->WorkState, m_client->fromUnicode(edtState->text(), NULL).c_str());
    set_str(&data->WorkZip, m_client->fromUnicode(edtZip->text(), NULL).c_str());
    data->WorkCountry = getComboValue(cmbCountry, getCountries());
    data->Occupation = getComboValue(cmbOccupation, occupations);
    set_str(&data->WorkName, m_client->fromUnicode(edtName->text(), NULL).c_str());
    set_str(&data->WorkDepartment, m_client->fromUnicode(edtDept->text(), NULL).c_str());
    set_str(&data->WorkPosition, m_client->fromUnicode(edtPosition->text(), NULL).c_str());
    set_str(&data->WorkHomepage, m_client->fromUnicode(edtSite->text(), NULL).c_str());
}

#ifndef WIN32
#include "workinfo.moc"
#endif

