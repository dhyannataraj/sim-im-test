/***************************************************************************
                         services.cpp  -  description
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

#include "services.h"
#include "jabber.h"
#include "jabbersearch.h"
#include "jabbermessage.h"
#include "listview.h"
#include "ballonmsg.h"

#include <qwidgetstack.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>

const unsigned COL_JID		= 0;
const unsigned COL_STATUS	= 1;
const unsigned COL_ID		= 2;
const unsigned COL_STATE	= 3;

Services::Services(QWidget *parent, JabberClient *client)
        : ServicesBase(parent)
{
    m_client  = client;
    m_bOnline = false;
    wndInfo->hide();
    wndInfo->addWidget(new QChildWidget(NULL), 0);
    wndInfo->raiseWidget(0);
    cmbAgents->hide();
    btnRegister->hide();
    lstAgents->addColumn(i18n("JID"));
    lstAgents->addColumn(i18n("Status"));
    lstAgents->setExpandingColumn(0);
    statusChanged();
    connect(cmbAgents, SIGNAL(activated(int)), this, SLOT(selectAgent(int)));
    connect(btnRegister, SIGNAL(clicked()), this, SLOT(regAgent()));
    connect(btnUnregister, SIGNAL(clicked()), this, SLOT(unregAgent()));
    connect(btnLogon, SIGNAL(clicked()), this, SLOT(logon()));
    connect(btnLogoff, SIGNAL(clicked()), this, SLOT(logoff()));
    connect(lstAgents, SIGNAL(selectionChanged()), this, SLOT(selectChanged()));
    connect(wndInfo, SIGNAL(aboutToShow(QWidget*)), this, SLOT(showAgent(QWidget*)));
    selectChanged();
}

void *Services::processEvent(Event *e)
{
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentFound){
        JabberAgentsInfo *data = (JabberAgentsInfo*)(e->param());
        if ((data->Client == m_client) && data->Register.bValue){
            AGENTS_MAP::iterator it = m_agents.find(data->ID.ptr);
            if (it == m_agents.end()){
                agentInfo info;
                info.search = NULL;
                info.name   = data->Name.ptr;
                m_agents.insert(AGENTS_MAP::value_type(data->ID.ptr, info));
                m_client->get_agent_info(data->ID.ptr, NULL, "register");
            }
        }
        return NULL;
    }
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo){
        JabberAgentInfo *data = (JabberAgentInfo*)(e->param());
        if (data->ID.ptr == NULL)
            return NULL;
        AGENTS_MAP::iterator it = m_agents.find(data->ID.ptr);
        if (it != m_agents.end()){
            agentInfo &info = (*it).second;
            if (info.search == NULL){
                info.search = new JabberSearch(this, m_client, data->ID.ptr, NULL, QString::fromUtf8(info.name.c_str()), true);
                unsigned id = cmbAgents->count();
                wndInfo->addWidget(info.search, id + 1);
                cmbAgents->insertItem(QString::fromUtf8(info.name.c_str()));
                if (id == 0){
                    cmbAgents->setCurrentItem(0);
                    selectAgent(0);
                    textChanged("");
                }
            }
            info.search->addWidget(data);
        }
        return NULL;
    }
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentRegister){
        agentRegisterInfo *info = (agentRegisterInfo*)(e->param());
        if (m_reg_id != info->id)
            return NULL;
        if (info->err_code){
            QString err;
            if (info->error && *info->error)
                err = i18n(info->error);
            if (err.isEmpty())
                err = i18n("Error %1") .arg(info->err_code);
            BalloonMsg::message(err, btnRegister);
        }
        btnRegister->setEnabled(true);
        return NULL;
    }
    switch (e->type()){
    case EventClientChanged:
        if ((Client*)(e->param()) == m_client)
            statusChanged();
        break;
    case EventContactChanged:{
            Contact *contact = (Contact*)(e->param());
            ClientDataIterator it(contact->clientData, m_client);
            JabberUserData *data;
            while ((data = ((JabberUserData*)(++it))) != NULL){
                if (!m_client->isAgent(data->ID.ptr))
                    continue;
                QListViewItem *item;
                for (item = lstAgents->firstChild(); item; item = item->nextSibling())
                    if ((item->text(COL_JID) + "/registered") == QString::fromUtf8(data->ID.ptr))
                        break;
                if (item == NULL)
                    makeAgentItem(data, contact->id());
            }
            break;
        }
    case EventMessageReceived:{
            Message *msg = (Message*)(e->param());
            if (msg->type() != MessageStatus)
                break;
            Contact *contact = getContacts()->contact(msg->contact());
            if (contact == NULL)
                break;
            ClientDataIterator it(contact->clientData, m_client);
            JabberUserData *data;
            while ((data = ((JabberUserData*)(++it))) != NULL){
                if (!m_client->isAgent(data->ID.ptr))
                    continue;
                QListViewItem *item;
                for (item = lstAgents->firstChild(); item; item = item->nextSibling())
                    if ((item->text(COL_JID) + "/registered") == QString::fromUtf8(data->ID.ptr))
                        break;
                if (item)
                    setAgentStatus(data, item);
            }
            break;
        }
    case EventContactDeleted:{
            Contact *contact = (Contact*)(e->param());
            for (QListViewItem *item = lstAgents->firstChild(); item; ){
                QListViewItem *next = item->nextSibling();
                if (item->text(COL_ID) == QString::number(contact->id()))
                    delete item;
                item = next;
            }
            break;
        }
    }
    return NULL;
}

void Services::statusChanged()
{
    bool bOnline = m_client->getStatus() != STATUS_OFFLINE;
    if (m_bOnline == bOnline)
        return;
    m_bOnline = bOnline;
    if (m_bOnline){
        lblOffline->hide();
        lblOffline2->hide();
        lblRegistered->show();
        lstAgents->show();
        btnLogon->show();
        btnLogoff->show();
        btnUnregister->show();
        cmbAgents->show();
        wndInfo->show();
        btnRegister->show();
        m_client->get_agents();
        Contact *contact;
        ContactList::ContactIterator it;
        while ((contact = ++it) != NULL){
            ClientDataIterator itd(contact->clientData, m_client);
            JabberUserData *data;
            while ((data = ((JabberUserData*)(++itd))) != NULL){
                if (!m_client->isAgent(data->ID.ptr))
                    continue;
                makeAgentItem(data, contact->id());
            }
        }
    }else{
        cmbAgents->clear();
        for (AGENTS_MAP::iterator it = m_agents.begin(); it != m_agents.end(); ++it){
            agentInfo &info = (*it).second;
            if (info.search){
                delete info.search;
            }
        }
        m_agents.clear();
        lblOffline->show();
        lblOffline2->show();
        cmbAgents->hide();
        wndInfo->hide();
        btnRegister->hide();
        lblRegistered->hide();
        lstAgents->hide();
        lstAgents->clear();
        btnLogon->hide();
        btnLogoff->hide();
        btnUnregister->hide();
    }
}

void Services::makeAgentItem(JabberUserData *data, unsigned contact_id)
{
    QString jid = QString::fromUtf8(data->ID.ptr);
    jid = jid.left(jid.length() - 11);
    QListViewItem *item = new QListViewItem(lstAgents, jid);
    item->setText(COL_ID, QString::number(contact_id));
    setAgentStatus(data, item);
}

void Services::setAgentStatus(JabberUserData *data, QListViewItem *item)
{
    if (data->Status.value == STATUS_OFFLINE){
        item->setText(COL_STATUS, i18n("Offline"));
        item->setText(COL_STATE, "");
    }else{
        item->setText(COL_STATUS, i18n("Online"));
        item->setText(COL_STATE, "1");
    }
    selectChanged();
}

void Services::selectAgent(int index)
{
    wndInfo->raiseWidget(index + 1);
}

void Services::selectChanged()
{
    QListViewItem *item = lstAgents->currentItem();
    if (item == NULL){
        btnLogon->setEnabled(false);
        btnLogoff->setEnabled(false);
        btnUnregister->setEnabled(false);
        return;
    }
    btnUnregister->setEnabled(true);
    bool bLogon = !item->text(COL_STATE).isEmpty();
    btnLogon->setEnabled(!bLogon);
    btnLogoff->setEnabled(bLogon);
}

void Services::regAgent()
{
    QWidget *w = wndInfo->visibleWidget();
    if (w == NULL)
        return;
    btnRegister->setEnabled(false);
    JabberSearch *s = static_cast<JabberSearch*>(w);
    bool bXSearch;
    m_reg_id = m_client->process(s->id(), NULL, s->condition(bXSearch).utf8(), "register");
}

void Services::unregAgent()
{
    QListViewItem *item = lstAgents->currentItem();
    if (item == NULL)
        return;
    string jid;
    jid = item->text(COL_JID).latin1();
    jid += "/registered";
    Contact *contact;
    string resource;
    JabberUserData *data = m_client->findContact(jid.c_str(), NULL, false, contact, resource);
    if (data == NULL)
        return;
    m_client->listRequest(data, NULL, NULL, true);
    contact->clientData.freeData(data);
    ClientDataIterator it(contact->clientData);
    if (++it == NULL){
        delete contact;
    }else{
        delete item;
    }
}

void Services::logon()
{
    QListViewItem *item = lstAgents->currentItem();
    if (item == NULL)
        return;
    string jid;
    jid = item->text(COL_JID).latin1();
    jid += "/registered";
    Contact *contact;
    string resource;
    JabberUserData *data = m_client->findContact(jid.c_str(), NULL, false, contact, resource);
    if (data == NULL)
        return;
    Message *msg = new Message(MessageJabberOnline);
    if (!m_client->send(msg, data))
        delete msg;
}

void Services::logoff()
{
    QListViewItem *item = lstAgents->currentItem();
    if (item == NULL)
        return;
    string jid;
    jid = item->text(COL_JID).latin1();
    jid += "/registered";
    Contact *contact;
    string resource;
    JabberUserData *data = m_client->findContact(jid.c_str(), NULL, false, contact, resource);
    if (data == NULL)
        return;
    Message *msg = new Message(MessageJabberOffline);
    if (!m_client->send(msg, data))
        delete msg;
}

void Services::apply(Client*, void*)
{
}

void Services::apply()
{
}

void Services::showAgent(QWidget*)
{
    textChanged("");
}

void Services::textChanged(const QString&)
{
    bool bEnable = false;
    QWidget *w = wndInfo->visibleWidget();
    if (w)
        bEnable = static_cast<JabberSearch*>(w)->canSearch();
    btnRegister->setEnabled(bEnable);
}

void Services::search()
{
}

#ifndef WIN32
#include "services.moc"
#endif

