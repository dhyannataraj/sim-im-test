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

#include "simapi.h"
#include "aboutinfo.h"
#include "icqclient.h"

#include <qmultilineedit.h>

AboutInfo::AboutInfo(QWidget *parent, struct ICQUserData *data, ICQClient *client)
        : AboutInfoBase(parent)
{
    m_data   = data;
    m_client = client;
    if (m_data)
        edtAbout->setReadOnly(true);
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
    set_str(&data->About, m_client->fromUnicode(edtAbout->text(), NULL).c_str());
}

void *AboutInfo::processEvent(Event *e)
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

void AboutInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtAbout->setText(m_client->toUnicode(data->About, data));
}

#ifndef WIN32
#include "aboutinfo.moc"
#endif

