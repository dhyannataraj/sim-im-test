/***************************************************************************
                          aiminfo.cpp  -  description
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
#include "aiminfo.h"
#include "icqclient.h"
#include "core.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qlabel.h>

AIMInfo::AIMInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client)
        : AIMInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    m_contact = contact;
    edtScreen->setReadOnly(true);
    if (m_data){
        edtFirst->setReadOnly(true);
        edtLast->setReadOnly(true);
        edtMiddle->setReadOnly(true);
        edtMaiden->setReadOnly(true);
        edtNick->setReadOnly(true);
        edtStreet->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        edtAutoReply->setReadOnly(true);
        disableWidget(cmbCountry);
    }else{
        edtAutoReply->hide();
    }
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtExtIP->setReadOnly(true);
    edtIntIP->setReadOnly(true);
    edtClient->setReadOnly(true);
    fill();
}

void AIMInfo::apply()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
}

void AIMInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    set_str(&data->FirstName.ptr, edtFirst->text().utf8());
    set_str(&data->LastName.ptr, edtLast->text().utf8());
    set_str(&data->MiddleName.ptr, edtMiddle->text().utf8());
    set_str(&data->Maiden.ptr, edtMaiden->text().utf8());
    set_str(&data->Nick.ptr, edtNick->text().utf8());
    set_str(&data->Address.ptr, edtStreet->text().utf8());
    set_str(&data->City.ptr, edtCity->text().utf8());
    set_str(&data->State.ptr, edtState->text().utf8());
    set_str(&data->Zip.ptr, edtZip->text().utf8());
}

void *AIMInfo::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact->clientData.have(m_data))
            fill();
    }
    if ((e->type() == EventMessageReceived) && m_data){
        Message *msg = (Message*)(e->param());
        if (msg->type() == MessageStatus){
            if (m_client->dataName(m_data) == msg->client())
                fill();
        }
    }
    if ((e->type() == EventClientChanged) && (m_data == 0)){
        Client *client = (Client*)(e->param());
        if (client == m_client)
            fill();
    }
    return NULL;
}

static void setText(QLineEdit *edit, const char *str)
{
    if (str == NULL)
        str = "";
    edit->setText(QString::fromUtf8(str));
}

void AIMInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;

    setText(edtScreen, data->Screen.ptr);
    setText(edtFirst, data->FirstName.ptr);
    setText(edtLast, data->LastName.ptr);
    setText(edtMiddle, data->MiddleName.ptr);
    setText(edtMaiden, data->Maiden.ptr);
    setText(edtNick, data->Nick.ptr);
    setText(edtStreet, data->Address.ptr);
    setText(edtCity, data->City.ptr);
    setText(edtState, data->State.ptr);
    setText(edtZip, data->Zip.ptr);
    initCombo(cmbCountry, (unsigned short)(data->Country.value), getCountries(), true, getCountryCodes());

    if (m_data == NULL){
        if (edtFirst->text().isEmpty())
            edtFirst->setText(getContacts()->owner()->getFirstName());
        if (edtLast->text().isEmpty())
            edtLast->setText(getContacts()->owner()->getLastName());
    }

    cmbStatus->clear();
    unsigned status = STATUS_ONLINE;
    if (m_data){
        unsigned s = m_data->Status.value;
        if (s == ICQ_STATUS_OFFLINE){
            status = STATUS_OFFLINE;
        }else if (s & ICQ_STATUS_AWAY){
            status = STATUS_AWAY;
        }
    }else{
        status = m_client->getStatus();
    }
    if ((status != STATUS_ONLINE) && (status != STATUS_OFFLINE) && m_data){
        edtAutoReply->setText(getContacts()->toUnicode(getContacts()->contact(m_contact), m_data->AutoReply.ptr));
    }else{
        edtAutoReply->hide();
    }

    int current = 0;
    const char *text = NULL;
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
    if (data->IP.ptr){
        edtExtIP->setText(formatAddr(data->IP, data->Port.value));
    }else{
        lblExtIP->hide();
        edtExtIP->hide();
    }
    if (data->RealIP.ptr && ((data->IP.ptr == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
        edtIntIP->setText(formatAddr(data->RealIP, data->Port.value));
    }else{
        lblIntIP->hide();
        edtIntIP->hide();
    }
    if (m_data){
        string client_name = m_client->clientName(data);
        if (client_name.length()){
            edtClient->setText(client_name.c_str());
        }else{
            lblClient->hide();
            edtClient->hide();
        }
    }else{
        string name = PACKAGE;
        name += " ";
        name += VERSION;
#ifdef WIN32
        name += "/win32";
#endif
        edtClient->setText(name.c_str());
    }
}

#ifndef WIN32
#include "aiminfo.moc"
#endif

