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

#include "icons.h"
#include "simapi.h"
#include "workinfo.h"
#include "icqclient.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>

using namespace SIM;

WorkInfo::WorkInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client)
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
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return NULL;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
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
        { I18N_NOOP("Professional"), 20  },
        { I18N_NOOP("Retail"), 21  },
        { I18N_NOOP("Retired"), 22  },
        { I18N_NOOP("Science & Research"), 23  },
        { I18N_NOOP("Sports"), 24  },
        { I18N_NOOP("Technical"), 25  },
        { I18N_NOOP("University student"), 26  },
        { I18N_NOOP("Web building"), 27  },
        { I18N_NOOP("Other services"), 99  },
        { "", 0  }
    };

const ext_info *p_occupations = occupations;

void WorkInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    edtAddress->setText(data->WorkAddress.str());
    edtCity->setText(data->WorkCity.str());
    edtState->setText(data->WorkState.str());
    edtZip->setText(data->WorkZip.str());
    initCombo(cmbCountry, (unsigned short)(data->WorkCountry.toULong()), getCountries());
    initCombo(cmbOccupation, (unsigned short)(data->Occupation.toULong()), occupations);
    edtName->setText(data->WorkName.str());
    edtDept->setText(data->WorkDepartment.str());
    edtPosition->setText(data->WorkPosition.str());
    edtSite->setText(data->WorkHomepage.str());
    urlChanged(edtSite->text());
}

void WorkInfo::goUrl()
{
    QString url = edtSite->text();
    if (url.isEmpty())
        return;
    EventGoURL e(url);
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
    data->WorkAddress.str()     = edtAddress->text();
    data->WorkCity.str()        = edtCity->text();
    data->WorkState.str()       = edtState->text();
    data->WorkZip.str()         = edtZip->text();
    data->WorkCountry.asULong() = getComboValue(cmbCountry, getCountries());
    data->Occupation.asULong()  = getComboValue(cmbOccupation, occupations);
    data->WorkName.str()        = edtName->text();
    data->WorkDepartment.str()  = edtDept->text();
    data->WorkPosition.str()    = edtPosition->text();
    data->WorkHomepage.str()    = edtSite->text();
}

#ifndef NO_MOC_INCLUDES
#include "workinfo.moc"
#endif

