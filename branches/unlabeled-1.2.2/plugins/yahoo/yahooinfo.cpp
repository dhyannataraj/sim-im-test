/***************************************************************************
                          yahooinfo.cpp  -  description
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
#include "yahoo.h"
#include "yahooinfo.h"
#include "yahooclient.h"
#include "core.h"

#include <qlineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>

YahooInfo::YahooInfo(QWidget *parent, struct YahooUserData *data, YahooClient *client)
        : YahooInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtLogin->setReadOnly(true);
    if (m_data){
        edtNick->setReadOnly(true);
        edtFirst->setReadOnly(true);
        edtLast->setReadOnly(true);
    }
	m_bInit = false;
    fill();
}

void YahooInfo::apply()
{
    YahooUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    getEncoding(data, m_data == NULL);
}

void *YahooInfo::processEvent(Event *e)
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

void YahooInfo::fill()
{
    YahooUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtLogin->setText(QString::fromUtf8(data->Login.ptr));
    edtNick->setText(data->Nick.ptr ? QString::fromUtf8(data->Nick.ptr) : QString(""));
    edtFirst->setText(data->First.ptr ? QString::fromUtf8(data->First.ptr) : QString(""));
    edtLast->setText(data->Last.ptr ? QString::fromUtf8(data->Last.ptr) : QString(""));
    int current = 0;
    const char *text = NULL;
    unsigned long status = STATUS_OFFLINE;
    if (m_data == NULL){
        if (m_client->getState() == Client::Connected){
            const char *statusIcon = NULL;
            unsigned style  = 0;
            m_client->contactInfo(&m_client->data.owner, status, style, statusIcon);
        }
    }else{
        const char *statusIcon = NULL;
        unsigned style  = 0;
        m_client->contactInfo(data, status, style, statusIcon);
    }
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
        if (data->StatusTime.value){
            lblOnline->setText(i18n("Last online") + ":");
            edtOnline->setText(formatDateTime(data->StatusTime.value));
            lblOnline->show();
            edtOnline->show();
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
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
    if (m_bInit)
        return;
    m_bInit = true;
    fillEncoding(data);
}

void YahooInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    YahooUserData *data = (YahooUserData*)_data;
    set_str(&data->Nick.ptr, edtNick->text().utf8());
    set_str(&data->First.ptr, edtFirst->text().utf8());
    set_str(&data->Last.ptr, edtLast->text().utf8());
}

void YahooInfo::fillEncoding(YahooUserData *data)
{
    int current = 0;
    int n_item = 1;
    cmbEncoding->clear();
    cmbEncoding->insertItem("Default");
    QStringList l;
    const ENCODING *e;
    QStringList main;
    QStringList::Iterator it;
    for (e = YahooPlugin::core->encodings; e->language; e++){
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
    for (e = YahooPlugin::core->encodings; e->language; e++){
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

void YahooInfo::getEncoding(YahooUserData *data, bool bDefault)
{
    string encoding;
    int n = cmbEncoding->currentItem();
    QString t = cmbEncoding->currentText();
    if (n){
        n--;
        QStringList l;
        const ENCODING *e;
        QStringList main;
        for (e = YahooPlugin::core->encodings; e->language; e++){
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
        for (e = YahooPlugin::core->encodings; e->language; e++){
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
        YahooPlugin::core->setDefaultEncoding(encoding.c_str());
    if (!set_str(&data->Encoding.ptr, encoding.c_str()))
        return;
    if (data->Login.ptr){
		Contact *contact = NULL;
        m_client->findContact(data->Login.ptr, NULL, contact);
		if (contact){
			Event e(EventContactChanged, contact);
			e.process();
			Event eh(EventHistoryConfig, (void*)(contact->id()));
			eh.process();
		}
    }
}

#ifndef WIN32
#include "yahooinfo.moc"
#endif

