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
#include "icqclient.h"
#include "textshow.h"

using namespace SIM;

AboutInfo::AboutInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client)
        : AboutInfoBase(parent)
{
    m_data   = data;
    m_client = client;
    if (m_data)
        edtAbout->setReadOnly(true);
    m_contact = contact;
    fill();
}

void AboutInfo::apply()
{
}

void AboutInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    data->About.str() = edtAbout->text();
}

void *AboutInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return NULL;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    }
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return NULL;
}

void AboutInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    if (data->Uin.toULong()){
        edtAbout->setTextFormat(QTextEdit::PlainText);
        edtAbout->setText(data->About.str());
    }else{
        edtAbout->setTextFormat(QTextEdit::RichText);
        edtAbout->setText(data->About.str());
        if (m_data == NULL)
            edtAbout->showBar();
    }
}

#ifndef NO_MOC_INCLUDES
#include "aboutinfo.moc"
#endif

