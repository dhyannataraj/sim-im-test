/***************************************************************************
                          jabberworkinfo.cpp  -  description
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
#include "jabberworkinfo.h"
#include "jabberclient.h"
#include "jabber.h"

#include <qlineedit.h>
#include <qstringlist.h>

JabberWorkInfo::JabberWorkInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client)
        : JabberWorkInfoBase(parent)
{
    m_client  = client;
    m_data	  = data;
    if (m_data){
        edtCompany->setReadOnly(true);
        edtDepartment->setReadOnly(true);
        edtTitle->setReadOnly(true);
        edtRole->setReadOnly(true);
    }
    fill(m_data);
}

void JabberWorkInfo::apply()
{
}

int str_cmp(const char *s1, const char *s2);

void *JabberWorkInfo::processEvent(Event *e)
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
		if (!str_cmp(m_data->ID, data->ID) && !str_cmp(m_data->Node, data->Node))
			fill(data);
	}
    return NULL;
}

void JabberWorkInfo::fill(JabberUserData *data)
{
    if (data == NULL) data = &m_client->data.owner;
    edtCompany->setText(data->OrgName ? QString::fromUtf8(data->OrgName) : QString(""));
    edtDepartment->setText(data->OrgUnit ? QString::fromUtf8(data->OrgUnit) : QString(""));
    edtTitle->setText(data->Title ? QString::fromUtf8(data->Title) : QString(""));
    edtRole->setText(data->Role ? QString::fromUtf8(data->Role) : QString(""));
}

void JabberWorkInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = (JabberUserData*)_data;
    set_str(&data->OrgName, edtCompany->text().utf8());
    set_str(&data->OrgUnit, edtDepartment->text().utf8());
    set_str(&data->Title, edtTitle->text().utf8());
    set_str(&data->Role, edtRole->text().utf8());
}

#ifndef WIN32
#include "jabberworkinfo.moc"
#endif

