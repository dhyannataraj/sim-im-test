/***************************************************************************
                          jabberhomeinfo.cpp  -  description
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
#include "jabberhomeinfo.h"
#include "jabberclient.h"
#include "jabber.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qstringlist.h>

JabberHomeInfo::JabberHomeInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client)
        : JabberHomeInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    if (m_data){
        edtStreet->setReadOnly(true);
        edtExt->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        edtCountry->setReadOnly(true);
    }
    fill(m_data);
}

void JabberHomeInfo::apply()
{
}

int str_cmp(const char *s1, const char *s2)
{
    if (s1 == NULL)
        s1 = "";
    if (s2 == NULL)
        s2 = "";
    return strcmp(s1, s2);
}

void *JabberHomeInfo::processEvent(Event *e)
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
    if (m_data && (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventVCard)){
        JabberUserData *data = (JabberUserData*)(e->param());
        if (!str_cmp(m_data->ID.ptr, data->ID.ptr) && !str_cmp(m_data->Node.ptr, data->Node.ptr))
            fill(data);
    }
    return NULL;
}

void JabberHomeInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtStreet->setText(data->Street.ptr ? QString::fromUtf8(data->Street.ptr) : QString(""));
    edtExt->setText(data->ExtAddr.ptr ? QString::fromUtf8(data->ExtAddr.ptr) : QString(""));
    edtCity->setText(data->City.ptr ? QString::fromUtf8(data->City.ptr) : QString(""));
    edtState->setText(data->Region.ptr ? QString::fromUtf8(data->Region.ptr) : QString(""));
    edtZip->setText(data->PCode.ptr ? QString::fromUtf8(data->PCode.ptr) : QString(""));
    edtCountry->setText(data->Country.ptr ? QString::fromUtf8(data->Country.ptr) : QString(""));
}

void JabberHomeInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = (JabberUserData*)_data;
    set_str(&data->Street.ptr, edtStreet->text().utf8());
    set_str(&data->ExtAddr.ptr, edtExt->text().utf8());
    set_str(&data->City.ptr, edtCity->text().utf8());
    set_str(&data->Region.ptr, edtState->text().utf8());
    set_str(&data->PCode.ptr, edtZip->text().utf8());
    set_str(&data->Country.ptr, edtCountry->text().utf8());
}

#ifndef WIN32
#include "jabberhomeinfo.moc"
#endif

