/***************************************************************************
                          aboutinfo.cpp  -  description
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

#include "aboutinfo.h"
#include "../icqclient.h"
#include "contacts/contact.h"
#include "events/eventhub.h"

using namespace SIM;

AboutInfo::AboutInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) :
        QWidget(parent),
        m_contact(contact),
        m_client(client),
        m_ui(new Ui::aboutInfo)
{
	m_ui->setupUi(this);
    if(m_contact != m_client->ownerContact())
        m_ui->edtAbout->setReadOnly(true);
    m_contact = contact;
    fill();
    SIM::getEventHub()->getEvent("icq_contact_about_info_updated")->connectTo(this, SLOT(contactAboutInfoUpdated(QString)));
}

void AboutInfo::contactAboutInfoUpdated(const QString& contactScreen)
{
    if(contactScreen != m_contact->getScreen())
        return;

    fill();
}

void AboutInfo::apply()
{
    if(changed())
    {
        m_contact->setAbout(m_ui->edtAbout->toPlainText());
        m_client->uploadAboutInfo();
    }
}

//void AboutInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
//{
//    if(client.data() != m_client)
//        return;
//    ICQUserData *data = m_client->toICQUserData(contact);
//    data->setAbout(edtAbout->toPlainText());
//}
//
//void AboutInfo::apply(Client *client, void *_data)
//{
//    if (client != m_client)
//        return;
//    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
//    data->setAbout(edtAbout->toPlainText());
//}
//
//bool AboutInfo::processEvent(Event *e)
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

void AboutInfo::fill()
{
    if(m_contact->getUin())
	{
        m_ui->edtAbout->setText(m_contact->getAbout());
    }
	else
	{
	    m_ui->edtAbout->setText(m_contact->getAbout());
    }
}

bool AboutInfo::changed() const
{
    return m_ui->edtAbout->toPlainText() != m_contact->getAbout();
}

