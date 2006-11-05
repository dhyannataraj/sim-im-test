/***************************************************************************
                          jabberaboutinfo.cpp  -  description
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

#include "jabberclient.h"
#include "simapi.h"
#include "jabberaboutinfo.h"
#include "jabber.h"

#include <qmultilineedit.h>

using namespace SIM;

JabberAboutInfo::JabberAboutInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client)
        : JabberAboutInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    if (m_data)
        edtAbout->setReadOnly(true);
    fill(m_data);
}

void JabberAboutInfo::apply()
{
}

void *JabberAboutInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return NULL;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill(m_data);
    }
    if ((e->type() == EventClientChanged) && (m_data == 0)){
        Client *client = (Client*)(e->param());
        if (client == m_client)
            fill(m_data);
    }
    if (m_data && (e->type() == EventVCard)){
        JabberUserData *data = (JabberUserData*)(e->param());
        if (m_data->ID.str() == data->ID.str() && m_data->Node.str() == data->Node.str())
            fill(data);
    }
    return NULL;
}

void JabberAboutInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtAbout->setText(data->Desc.str());
}

void JabberAboutInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = (JabberUserData*)_data;
    data->Desc.str() = edtAbout->text();
}

#ifndef NO_MOC_INCLUDES
#include "jabberaboutinfo.moc"
#endif

