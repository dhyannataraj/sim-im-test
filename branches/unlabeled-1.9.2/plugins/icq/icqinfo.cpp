/***************************************************************************
                          icqinfo.cpp  -  description
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
#include "icqinfo.h"
#include "icqclient.h"
#include "core.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qlabel.h>

const ext_info chat_groups[] =
    {
        { I18N_NOOP("General chat"), 1 },
        { I18N_NOOP("Romance"), 2 },
        { I18N_NOOP("Games"), 3 },
        { I18N_NOOP("Students"), 4 },
        { I18N_NOOP("20 Something"), 5 },
        { I18N_NOOP("30 Something"), 6 },
        { I18N_NOOP("40 Something"), 7 },
        { I18N_NOOP("50 Plus"), 8 },
        { I18N_NOOP("Seeking Women"), 9 },
        { I18N_NOOP("Seeking Men"), 10 },
        { "", 0 }
    };

const ext_info *p_chat_groups = chat_groups;

ICQInfo::ICQInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client)
        : ICQInfoBase(parent)
{
    m_bInit		= false;
    m_client	= client;
    m_data		= data;
	m_contact	= contact;
    edtUin->setReadOnly(true);
    if (m_data){
        edtFirst->setReadOnly(true);
        edtLast->setReadOnly(true);
        edtNick->setReadOnly(true);
        edtAutoReply->setReadOnly(true);
        lblRandom->hide();
        cmbRandom->hide();
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

void ICQInfo::apply()
{
    ICQUserData *data = m_data;
    if (data == NULL){
        data = &m_client->data.owner;
        m_client->setRandomChatGroup(getComboValue(cmbRandom, chat_groups));
    }
    m_client->getEncoding(cmbEncoding, data, m_data == NULL);
}

void ICQInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    set_str(&data->FirstName.ptr, getContacts()->fromUnicode(NULL, edtFirst->text()).c_str());
    set_str(&data->LastName.ptr, getContacts()->fromUnicode(NULL, edtLast->text()).c_str());
    set_str(&data->Nick.ptr, getContacts()->fromUnicode(NULL, edtNick->text()).c_str());
}

void *ICQInfo::processEvent(Event *e)
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

void ICQInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;

    edtUin->setText(QString::number(data->Uin.value));
    edtFirst->setText(m_client->toUnicode(data->FirstName.ptr, data));
    edtLast->setText(m_client->toUnicode(data->LastName.ptr, data));
    edtNick->setText(m_client->toUnicode(data->Nick.ptr, data));

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
        }else if (s & ICQ_STATUS_DND){
            status = STATUS_DND;
        }else if (s & ICQ_STATUS_OCCUPIED){
            status = STATUS_OCCUPIED;
        }else if (s & ICQ_STATUS_NA){
            status = STATUS_NA;
        }else if (s & ICQ_STATUS_AWAY){
            status = STATUS_AWAY;
        }else if (s & ICQ_STATUS_FFC){
            status = STATUS_FFC;
        }
    }else{
        status = m_client->getStatus();
        initCombo(cmbRandom, m_client->getRandomChatGroup(), chat_groups);
    }
    if ((status != STATUS_ONLINE) && (status != STATUS_OFFLINE) && m_data){
        edtAutoReply->setText(m_client->toUnicode(m_data->AutoReply.ptr, m_data));
    }else{
        edtAutoReply->hide();
    }

    int current = 0;
    const char *text = NULL;
    if (m_data && (status == STATUS_OFFLINE) && m_data->bInvisible.bValue){
        cmbStatus->insertItem(Pict("ICQ_invisible"), i18n("Possibly invisible"));
    }else{
        for (const CommandDef *cmd = m_client->protocol()->statusList(); cmd->id; cmd++){
            if (cmd->flags & COMMAND_CHECK_STATE)
                continue;
            if (status == cmd->id){
                current = cmbStatus->count();
                text = cmd->text;
            }
            cmbStatus->insertItem(Pict(cmd->icon), i18n(cmd->text));
        }
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
    if ((data->RealIP.ptr) && ((data->IP.ptr == NULL) || (get_ip(data->IP) != get_ip(data->RealIP)))){
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

    if (m_bInit)
        return;
    m_bInit = true;
    m_client->fillEncoding(cmbEncoding, data);
}

void ICQClient::fillEncoding(QComboBox *cmbEncoding, ICQUserData *data)
{
    int current = 0;
    int n_item = 1;
    cmbEncoding->clear();
    cmbEncoding->insertItem("Default");
    QStringList l;
    const ENCODING *e;
    QStringList main;
    QStringList::Iterator it;
    for (e = ICQPlugin::core->encodings; e->language; e++){
        if (!e->bMain)
            continue;
        main.append(i18n(e->language) + " (" + e->codec + ")");
    }
    main.sort();
    for (it = main.begin(); it != main.end(); ++it, n_item++){
        QString str = *it;
        int n = str.find('(');
        str = str.mid(n + 1);
        n = str.find(')');
        str = str.left(n);
        if (data->Encoding.ptr && !strcmp(data->Encoding.ptr, str.latin1()))
            current = n_item;
        cmbEncoding->insertItem(*it);
    }
    QStringList noMain;
    for (e = ICQPlugin::core->encodings; e->language; e++){
        if (e->bMain)
            continue;
        noMain.append(i18n(e->language) + " (" + e->codec + ")");
    }
    noMain.sort();
    for (it = noMain.begin(); it != noMain.end(); ++it, n_item++){
        QString str = *it;
        int n = str.find('(');
        str = str.mid(n + 1);
        n = str.find(')');
        str = str.left(n);
        if (data->Encoding.ptr && !strcmp(data->Encoding.ptr, str.latin1()))
            current = n_item;
        cmbEncoding->insertItem(*it);
    }
    cmbEncoding->setCurrentItem(current);
}

void ICQClient::getEncoding(QComboBox *cmbEncoding, ICQUserData *data, bool bDefault)
{
    string encoding;
    int n = cmbEncoding->currentItem();
    QString t = cmbEncoding->currentText();
    if (n){
        n--;
        QStringList l;
        const ENCODING *e;
        QStringList main;
        for (e = ICQPlugin::core->encodings; e->language; e++){
            if (!e->bMain)
                continue;
            main.append(i18n(e->language) + " (" + e->codec + ")");
        }
        main.sort();
        QStringList::Iterator it;
        for (it = main.begin(); it != main.end(); ++it){
            l.append(*it);
        }
        QStringList noMain;
        for (e = ICQPlugin::core->encodings; e->language; e++){
            if (e->bMain)
                continue;
            noMain.append(i18n(e->language) + " (" + e->codec + ")");
        }
        noMain.sort();
        for (it = noMain.begin(); it != noMain.end(); ++it){
            l.append(*it);
        }
        for (it = l.begin(); it != l.end(); ++it){
            if (n-- == 0){
                QString str = *it;
                int n = str.find('(');
                str = str.mid(n + 1);
                n = str.find(')');
                str = str.left(n);
                encoding = str.latin1();
                break;
            }
        }
    }
    if (bDefault)
        ICQPlugin::core->setDefaultEncoding(encoding.c_str());
    if (!set_str(&data->Encoding.ptr, encoding.c_str()))
        return;
    Contact *contact;
    if (data->Uin.value ?
            findContact(number(data->Uin.value).c_str(), NULL, false, contact) :
            findContact(data->Screen.ptr, NULL, false, contact)){
        Event e(EventContactChanged, contact);
        e.process();
        Event eh(EventHistoryConfig, (void*)(contact->id()));
        eh.process();
    }
}

#ifndef WIN32
#include "icqinfo.moc"
#endif

