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

#include "workinfo.h"
#include "../icqclient.h"
#include "contacts/contact.h"
#include "imagestorage/imagestorage.h"

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

using namespace SIM;

WorkInfo::WorkInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) :
        QWidget(parent),
        m_contact(contact),
        m_client(client),
        m_ui(new Ui::WorkInfo)
{
    m_ui->setupUi(this);
    m_ui->btnSite->setIcon(SIM::getImageStorage()->icon("home"));
    connect(m_ui->btnSite, SIGNAL(clicked()), this, SLOT(goUrl()));
    if(m_contact != client->ownerContact())
    {
        m_ui->edtAddress->setReadOnly(true);
        m_ui->edtCity->setReadOnly(true);
        m_ui->edtState->setReadOnly(true);
        m_ui->edtZip->setReadOnly(true);
        m_ui->cmbCountry->setEnabled(false);
        m_ui->cmbOccupation->setEnabled(false);
        m_ui->edtName->setReadOnly(true);
        m_ui->edtDept->setReadOnly(true);
        m_ui->edtPosition->setReadOnly(true);
        m_ui->edtSite->setReadOnly(true);
    }
    else
    {
        connect(m_ui->edtSite, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    }
    fill();
}
//
//void WorkInfo::apply()
//{
//}
//
//bool WorkInfo::processEvent(Event *e)
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
    m_ui->edtAddress->setPlainText(m_contact->getWorkAddress());
    m_ui->edtCity->setText(m_contact->getWorkCity());
    m_ui->edtState->setText(m_contact->getWorkState());
    m_ui->edtZip->setText(m_contact->getWorkZip());
    initCombo(m_ui->cmbCountry, m_contact->getWorkCountry(), getCountries());
    initCombo(m_ui->cmbOccupation, m_contact->getOccupation(), occupations);
    m_ui->edtName->setText(m_contact->getWorkName());
    m_ui->edtDept->setText(m_contact->getWorkDepartment());
    m_ui->edtPosition->setText(m_contact->getWorkPosition());
    m_ui->edtSite->setText(m_contact->getWorkHomepage());
    urlChanged(m_ui->edtSite->text());
}

void WorkInfo::goUrl()
{
//    QString url = edtSite->text();
//    if (url.isEmpty())
//        return;
//    EventGoURL e(url);
//    e.process();
}

void WorkInfo::urlChanged(const QString &text)
{
    m_ui->btnSite->setEnabled(!text.isEmpty());
}

Ui::WorkInfo* WorkInfo::ui() const
{
    return m_ui;
}
//
//void WorkInfo::updateData(ICQUserData* data)
//{
//    data->setWorkAddress(edtAddress->toPlainText());
//    data->setWorkCity(edtCity->text());
//    data->setWorkState(edtState->text());
//    data->setWorkZip(edtZip->text());
//    data->setWorkCountry(getComboValue(cmbCountry, getCountries()));
//    data->setOccupation(getComboValue(cmbOccupation, occupations));
//    data->setWorkName(edtName->text());
//    data->setWorkDepartment(edtDept->text());
//    data->setWorkPosition(edtPosition->text());
//    data->setWorkHomepage(edtSite->text());
//}
//
//void WorkInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
//{
//    if (client != m_client)
//        return;
//    updateData(m_client->toICQUserData(contact));
//}
//
//void WorkInfo::apply(Client *client, void *_data)
//{
//    if (client != m_client)
//        return;
//    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
//    updateData(data);
//}

