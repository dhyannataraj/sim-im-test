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

#include "icons.h"
#include "jabberclient.h"
#include "simapi.h"
#include "jabberinfo.h"
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

using namespace SIM;

JabberInfo::JabberInfo(QWidget *parent, JabberUserData *data, JabberClient *client)
        : JabberInfoBase(parent)
{
    m_client  = client;
    m_data    = data;
    btnUrl->setPixmap(Pict("home"));
    connect(btnUrl, SIGNAL(clicked()), this, SLOT(goUrl()));
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtID->setReadOnly(true);
    edtClient->setReadOnly(true);
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
            m_client->changePassword(edtPswd1->text());
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
    QString clientName, clientVersion, clientOS;
    if ((n == 0) || (n > data->nResources.toULong())){
        status = m_data ? m_data->Status.toULong() : m_client->getStatus();
        statusTime = data->StatusTime.toULong();
        onlineTime = data->OnlineTime.toULong();
    }else{
        status = get_str(data->ResourceStatus, n).toUInt();
        statusTime = get_str(data->ResourceStatusTime, n).toUInt();
        onlineTime = get_str(data->ResourceOnlineTime, n).toUInt();
        autoReply = get_str(data->ResourceReply, n);
        clientName = get_str(data->ResourceClientName, n);
        clientVersion = get_str(data->ResourceClientVersion, n);
        clientOS = get_str(data->ResourceClientOS, n);
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
    if (clientName.isEmpty()){
        edtClient->setEnabled(false);
    }else{
        edtClient->setEnabled(true);
        QString clientString = clientName + " " + clientVersion;
        if (!clientOS.isEmpty())
            clientString += " / " + clientOS;
        edtClient->setText(clientString);
    }
}

void *JabberInfo::processEvent(Event *e)
{
    if ((e->type() == EventMessageReceived) && m_data){
        Message *msg = (Message*)(e->param());
        if ((msg->type() == MessageStatus) && (m_client->dataName(m_data) == msg->client()))
            fill();
    } else
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return NULL;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return NULL;
}

void JabberInfo::fill()
{
    JabberUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    edtID->setText(data->ID.str());
    edtFirstName->setText(data->FirstName.str());
    edtNick->setText(data->Nick.str());
    edtDate->setText(data->Bday.str());
    edtUrl->setText(data->Url.str());
    urlChanged(edtUrl->text());
    cmbResource->clear();
    if (data->nResources.toULong()){
        for (unsigned i = 1; i <= data->nResources.toULong(); i++)
            cmbResource->insertItem(get_str(data->Resources, i));
        cmbResource->setEnabled(data->nResources.toULong() > 1);
    }else{
        if (!data->Resource.str().isEmpty())
            cmbResource->insertItem(data->Resource.str());
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
    data->FirstName.str() = edtFirstName->text();
    data->Nick.str()      = edtNick->text();
    data->Bday.str()      = edtDate->text();
    data->Url.str()       = edtUrl->text();
}

void JabberInfo::goUrl()
{
    QString url = edtUrl->text();
    if (url.isEmpty())
        return;
    EventGoURL e(url);
    e.process();
}

void JabberInfo::urlChanged(const QString &text)
{
    btnUrl->setEnabled(!text.isEmpty());
}

#ifndef NO_MOC_INCLUDES
#include "jabberinfo.moc"
#endif

