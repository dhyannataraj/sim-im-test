/***************************************************************************
                          msninfo.cpp  -  description
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
#include "msninfo.h"
#include "msnclient.h"

#include <qlineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>

MSNInfo::MSNInfo(QWidget *parent, struct MSNUserData *data, MSNClient *client)
        : MSNInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtEMail->setReadOnly(true);
    if (m_data){
        edtNick->setReadOnly(true);
    }
    fill();
}

void MSNInfo::apply()
{
}

void *MSNInfo::processEvent(Event *e)
{
    if ((e->type() == EventMessageReceived) && m_data){
        Message *msg = (Message*)(e->param());
        if ((msg->type() == MessageStatus) && (m_client->dataName(m_data) == msg->client()))
            fill();
    }
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

void MSNInfo::fill()
{
    MSNUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtEMail->setText(QString::fromUtf8(data->EMail.ptr));
    edtNick->setText(data->ScreenName.ptr ? QString::fromUtf8(data->ScreenName.ptr) : edtEMail->text());
    int current = 0;
    const char *text = NULL;
    unsigned status = m_data ? m_data->Status.value : m_client->getStatus();
    for (const CommandDef *cmd = m_client->protocol()->statusList(); cmd->id; cmd++){
        if (cmd->flags & COMMAND_CHECK_STATE)
            continue;
        if (status == cmd->id){
            current = cmbStatus->count();
            text = cmd->text;
        }
        cmbStatus->insertItem(Pict(cmd->icon), i18n(cmd->text));
    }
    cmbStatus->setCurrentItem(current);
    disableWidget(cmbStatus);
    if (status == STATUS_OFFLINE){
        lblOnline->setText(i18n("Last online") + ":");
        edtOnline->setText(formatDateTime(data->StatusTime.value));
        lblNA->hide();
        edtNA->hide();
    }else{
        if (data->OnlineTime.value){
            edtOnline->setText(formatDateTime(data->OnlineTime.value));
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || (text == NULL)){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(data->StatusTime.value));
        }
    }
}

void MSNInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    QString nick = edtNick->text();
    if (nick == edtEMail->text())
        nick = "";
    MSNUserData *data = (MSNUserData*)_data;
    set_str(&data->ScreenName.ptr, nick.utf8());
}

#ifndef WIN32
#include "msninfo.moc"
#endif

