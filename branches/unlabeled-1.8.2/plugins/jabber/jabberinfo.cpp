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
#include "ballonmsg.h"

#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qtabwidget.h>

JabberInfo::JabberInfo(QWidget *parent, struct JabberUserData *data, JabberClient *client)
        : JabberInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    btnUrl->setPixmap(Pict("home"));
    connect(btnUrl, SIGNAL(clicked()), this, SLOT(goUrl()));
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtID->setReadOnly(true);
    if (m_data){
        edtFirstName->setReadOnly(true);
        edtNick->setReadOnly(true);
        disableWidget(edtDate);
        edtUrl->setReadOnly(true);
        edtAutoReply->setReadOnly(true);
        tabWnd->removePage(password);
    }else{
        connect(edtUrl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
        connect(this, SIGNAL(raise(QWidget*)), topLevelWidget(), SLOT(raisePage(QWidget*)));
        edtAutoReply->hide();
    }
    fill();
    connect(cmbResource, SIGNAL(activated(int)), this, SLOT(resourceActivated(int)));
}

void JabberInfo::apply()
{
    if ((m_data == NULL) && (m_client->getState() == Client::Connected)){
        QString errMsg;
        QWidget *errWidget = edtCurrent;
        if (!edtPswd1->text().isEmpty() || !edtPswd2->text().isEmpty()){
            if (edtCurrent->text().isEmpty()){
                errMsg = i18n("Input current password");
            }else{
                if (edtPswd1->text() != edtPswd2->text()){
                    errMsg = i18n("Confirm password does not match");
                    errWidget = edtPswd2;
                }else if (edtCurrent->text() != m_client->getPassword()){
                    errMsg = i18n("Invalid password");
                }
            }
        }
        if (!errMsg.isEmpty()){
            for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
                if (p->inherits("QTabWidget")){
                    static_cast<QTabWidget*>(p)->showPage(this);
                    break;
                }
            }
            emit raise(this);
            BalloonMsg::message(errMsg, errWidget);
            return;
        }
        if (!edtPswd1->text().isEmpty())
            m_client->changePassword(edtPswd1->text().utf8());
        // clear Textboxes
        edtCurrent->clear();
        edtPswd1->clear();
        edtPswd2->clear();
    }
}

void JabberInfo::resourceActivated(int i)
{
    JabberUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    unsigned n = i + 1;
    unsigned status = STATUS_OFFLINE;
    unsigned statusTime;
    unsigned onlineTime;
    QString autoReply;
    if ((n == 0) || (n > data->nResources.value)){
        status = m_data ? m_data->Status.value : m_client->getStatus();
        statusTime = data->StatusTime.value;
        onlineTime = data->OnlineTime.value;
    }else{
        status = atol(get_str(data->ResourceStatus, n));
        statusTime = atol(get_str(data->ResourceStatusTime, n));
        onlineTime = atol(get_str(data->ResourceOnlineTime, n));
        autoReply = QString::fromUtf8(get_str(data->ResourceReply, n));
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
        edtOnline->setText(formatDateTime(statusTime));
        lblOnline->show();
        edtOnline->show();
        lblNA->hide();
        edtNA->hide();
    }else{
        if (onlineTime){
            edtOnline->setText(formatDateTime(onlineTime));
            lblOnline->show();
            edtOnline->show();
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        if ((status == STATUS_ONLINE) || (text == NULL)){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(i18n(text));
            edtNA->setText(formatDateTime(statusTime));
            lblNA->show();
            edtNA->show();
        }
    }
    if (autoReply.isEmpty()){
        edtAutoReply->hide();
    }else{
        edtAutoReply->show();
        edtAutoReply->setText(autoReply);
    }
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
    edtID->setText(QString::fromUtf8(data->ID.ptr));
    edtFirstName->setText(data->FirstName.ptr ? QString::fromUtf8(data->FirstName.ptr) : QString(""));
    edtNick->setText(data->Nick.ptr ? QString::fromUtf8(data->Nick.ptr) : QString(""));
    edtDate->setText(data->Bday.ptr ? QString::fromUtf8(data->Bday.ptr) : QString(""));
    edtUrl->setText(data->Url.ptr ? QString::fromUtf8(data->Url.ptr) : QString(""));
    urlChanged(edtUrl->text());
    cmbResource->clear();
    if (data->nResources.value){
        for (unsigned i = 1; i <= data->nResources.value; i++)
            cmbResource->insertItem(QString::fromUtf8(get_str(data->Resources, i)));
        cmbResource->setEnabled(data->nResources.value > 1);
    }else{
        if (data->Resource.ptr)
            cmbResource->insertItem(QString::fromUtf8(data->Resource.ptr));
        cmbResource->setEnabled(false);
    }
    resourceActivated(0);
    if (m_data == NULL)
        password->setEnabled(m_client->getState() == Client::Connected);
}

void JabberInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    JabberUserData *data = (JabberUserData*)_data;
    set_str(&data->FirstName.ptr, edtFirstName->text().utf8());
    set_str(&data->Nick.ptr, edtNick->text().utf8());
    set_str(&data->Bday.ptr, edtDate->text().utf8());
    set_str(&data->Url.ptr, edtUrl->text().utf8());
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

