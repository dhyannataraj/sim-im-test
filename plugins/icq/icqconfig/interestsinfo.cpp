/***************************************************************************
                          interestsinfo.cpp  -  description
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

#include "interestsinfo.h"
#include "../icqclient.h"
#include "contacts/contact.h"
#include "events/eventhub.h"

#include <QLineEdit>
#include <QComboBox>

using namespace SIM;

InterestsInfo::InterestsInfo(QWidget *parent, const ICQContactPtr& contact, ICQClient *client) : QWidget(parent),
        m_contact(contact),
        m_client(client),
        m_ui(new Ui::InterestsInfoBase)
{
	m_ui->setupUi(this);
    if(m_contact != m_client->ownerContact())
    {
        m_ui->edtBg1->setReadOnly(true);
        m_ui->edtBg2->setReadOnly(true);
        m_ui->edtBg3->setReadOnly(true);
        m_ui->edtBg4->setReadOnly(true);
        m_ui->cmbBg1->setEnabled(false);
        m_ui->cmbBg2->setEnabled(false);
        m_ui->cmbBg3->setEnabled(false);
        m_ui->cmbBg4->setEnabled(false);
    }
    fill();
    SIM::getEventHub()->getEvent("icq_contact_interests_info_updated")->connectTo(this, SLOT(contactInterestsInfoUpdated(QString)));
}

void InterestsInfo::contactInterestsInfoUpdated(const QString& contactScreen)
{
    if(contactScreen != m_contact->getScreen())
        return;

    fill();
}

//void InterestsInfo::apply()
//{
//}
//
//void InterestsInfo::updateData(ICQUserData* data)
//{
//    QString info[4];
//    info[0] = getInfo(cmbBg1, edtBg1);
//    info[1] = getInfo(cmbBg2, edtBg2);
//    info[2] = getInfo(cmbBg3, edtBg3);
//    info[3] = getInfo(cmbBg4, edtBg4);
//    QString res;
//    for (unsigned i = 0; i < 4; i++){
//        if (info[i].isEmpty())
//            continue;
//        if (!res.isEmpty())
//            res += ';';
//        res += info[i];
//    }
//    data->setInterests(res);
//}
//
//void InterestsInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
//{
//    if (client != m_client)
//        return;
//    updateData(m_client->toICQUserData(contact));
//}
//
//void InterestsInfo::apply(Client *client, void *_data)
//{
//    if (client != m_client)
//        return;
//    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
//    updateData(data);
//}
//
//bool InterestsInfo::processEvent(Event *e)
//{
//    if (e->type() == eEventContact){
//        EventContact *ec = static_cast<EventContact*>(e);
//        if(ec->action() != EventContact::eChanged)
//            return false;
//        Contact *contact = ec->contact();
//        if (contact->have(m_data))
//            fill();
//    }
//    if ((e->type() == eEventClientChanged) && (m_data == 0)){
//        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
//        if (ecc->client() == m_client)
//            fill();
//    }
//    return false;
//}

static const ext_info interests[] =
    {
        { I18N_NOOP("Art"), 100  },
        { I18N_NOOP("Cars"), 101  },
        { I18N_NOOP("Celebrity Fans"), 102  },
        { I18N_NOOP("Collections"), 103  },
        { I18N_NOOP("Computers"), 104  },
        { I18N_NOOP("Culture & Literature"), 105 },
        { I18N_NOOP("Fitness"), 106  },
        { I18N_NOOP("Games"), 107  },
        { I18N_NOOP("Hobbies"), 108  },
        { I18N_NOOP("ICQ - Providing Help"), 109  },
        { I18N_NOOP("Internet"), 110  },
        { I18N_NOOP("Lifestyle"), 111  },
        { I18N_NOOP("Movies/TV"), 112  },
        { I18N_NOOP("Music"), 113  },
        { I18N_NOOP("Outdoor Activities"), 114  },
        { I18N_NOOP("Parenting"), 115  },
        { I18N_NOOP("Pets/Animals"), 116  },
        { I18N_NOOP("Religion"), 117  },
        { I18N_NOOP("Science/Technology"), 118  },
        { I18N_NOOP("Skills"), 119  },
        { I18N_NOOP("Sports"), 120  },
        { I18N_NOOP("Web Design"), 121  },
        { I18N_NOOP("Nature and Environment"), 122  },
        { I18N_NOOP("News & Media"), 123  },
        { I18N_NOOP("Government"), 124  },
        { I18N_NOOP("Business & Economy"), 125  },
        { I18N_NOOP("Mystics"), 126  },
        { I18N_NOOP("Travel"), 127  },
        { I18N_NOOP("Astronomy"), 128  },
        { I18N_NOOP("Space"), 129  },
        { I18N_NOOP("Clothing"), 130  },
        { I18N_NOOP("Parties"), 131  },
        { I18N_NOOP("Women"), 132  },
        { I18N_NOOP("Social science"), 133  },
        { I18N_NOOP("60's"), 134  },
        { I18N_NOOP("70's"), 135  },
        { I18N_NOOP("80's"), 136  },
        { I18N_NOOP("50's"), 137  },
        { I18N_NOOP("Finance and corporate"), 138  },
        { I18N_NOOP("Entertainment"), 139  },
        { I18N_NOOP("Consumer electronics"), 140  },
        { I18N_NOOP("Retail stores"), 141  },
        { I18N_NOOP("Health and beauty"), 142  },
        { I18N_NOOP("Media"), 143  },
        { I18N_NOOP("Household products"), 144  },
        { I18N_NOOP("Mail order catalog"), 145  },
        { I18N_NOOP("Business services"), 146  },
        { I18N_NOOP("Audio and visual"), 147  },
        { I18N_NOOP("Sporting and athletic"), 148  },
        { I18N_NOOP("Publishing"), 149  },
        { I18N_NOOP("Home automation"), 150  },
        { "", 0  }
    };

const ext_info *p_interests = interests;

void InterestsInfo::fill()
{
    m_ui->edtBg1->setText(m_contact->getInterestText(0));
    initCombo(m_ui->cmbBg1, m_contact->getInterest(0), interests);

    m_ui->edtBg2->setText(m_contact->getInterestText(1));
    initCombo(m_ui->cmbBg2, m_contact->getInterest(1), interests);

    m_ui->edtBg3->setText(m_contact->getInterestText(2));
    initCombo(m_ui->cmbBg3, m_contact->getInterest(2), interests);

    m_ui->edtBg4->setText(m_contact->getInterestText(3));
    initCombo(m_ui->cmbBg4, m_contact->getInterest(3), interests);
}

Ui::InterestsInfoBase* InterestsInfo::ui() const
{
    return m_ui;
}
