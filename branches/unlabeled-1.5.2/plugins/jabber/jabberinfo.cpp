/***************************************************************************
                          jabberinfo.cpp  -  description
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
#include "jabberinfo.h"
#include "jabberclient.h"
#include "datepicker.h"

#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>

JabberInfo::JabberInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client)
        : JabberInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    btnUrl->setPixmap(Pict("home"));
    connect(btnUrl, SIGNAL(clicked()), this, SLOT(goUrl()));
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtResource->setReadOnly(true);
    edtID->setReadOnly(true);
    if (m_data){
        edtFirstName->setReadOnly(true);
        edtNick->setReadOnly(true);
        disableWidget(edtDate);
        edtUrl->setReadOnly(true);
        edtAutoReply->setReadOnly(true);
    }else{
        connect(edtUrl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
        edtAutoReply->hide();
    }
    fill();
}

void JabberInfo::apply()
{
}

void *JabberInfo::processEvent(Event *e)
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

void JabberInfo::fill()
{
    JabberUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtID->setText(QString::fromUtf8(data->ID));
    edtFirstName->setText(data->FirstName ? QString::fromUtf8(data->FirstName) : QString(""));
    edtNick->setText(data->Nick ? QString::fromUtf8(data->Nick) : QString(""));
    edtDate->setText(data->Bday ? QString::fromUtf8(data->Bday) : QString(""));
    edtUrl->setText(data->Url ? QString::fromUtf8(data->Url) : QString(""));
    urlChanged(edtUrl->text());
    int current = 0;
    const char *text = NULL;
    unsigned status = m_data ? m_data->Status : m_client->getStatus();
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
        edtOnline->setText(formatDateTime(data->StatusTime));
        lblNA->hide();
        edtNA->hide();
    }else{
        if (data->OnlineTime){
            edtOnline->setText(formatDateTime(data->OnlineTime));
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || (text == NULL)){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(data->StatusTime));
        }
    }
    if ((status != STATUS_ONLINE) && (status != STATUS_OFFLINE) && m_data){
        QString autoReply;
        if (m_data->AutoReply)
            autoReply = QString::fromUtf8(m_data->AutoReply);
        edtAutoReply->setText(autoReply);
    }else{
        edtAutoReply->hide();
    }
    edtResource->setText(data->Resource ? QString::fromUtf8(data->Resource) : QString(""));
}

void JabberInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = (JabberUserData*)_data;
    set_str(&data->FirstName, edtFirstName->text().utf8());
    set_str(&data->Nick, edtNick->text().utf8());
    set_str(&data->Bday, edtDate->text().utf8());
    set_str(&data->Url, edtUrl->text().utf8());
}

void JabberInfo::goUrl()
{
    QString url = edtUrl->text();
    if (url.isEmpty())
        return;
    Event e(EventGoURL, (void*)(const char*)(url.local8Bit()));
    e.process();
}

void JabberInfo::urlChanged(const QString &text)
{
    btnUrl->setEnabled(!text.isEmpty());
}

#ifndef WIN32
#include "jabberinfo.moc"
#endif

