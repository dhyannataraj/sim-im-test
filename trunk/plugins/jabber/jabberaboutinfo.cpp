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

#include "simapi.h"
#include "jabberaboutinfo.h"
#include "jabberclient.h"
#include "jabber.h"

#include <qmultilineedit.h>

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

int str_cmp(const char *s1, const char *s2);

void *JabberAboutInfo::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
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
        if (!str_cmp(m_data->ID.ptr, data->ID.ptr) && !str_cmp(m_data->Node.ptr, data->Node.ptr))
            fill(data);
    }
    return NULL;
}

void JabberAboutInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtAbout->setText(data->Desc.ptr ? QString::fromUtf8(data->Desc.ptr) : QString(""));
}

void JabberAboutInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = (JabberUserData*)_data;
    set_str(&data->Desc.ptr, edtAbout->text().utf8());
}

#ifndef WIN32
#include "jabberaboutinfo.moc"
#endif

