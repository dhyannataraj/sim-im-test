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

#include <qwidgetstack.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>

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
    statusChanged();
    connect(cmbAgents, SIGNAL(activated(int)), this, SLOT(selectAgent(int)));
}

void *Services::processEvent(Event *e)
{
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentFound){
        JabberAgentsInfo *data = (JabberAgentsInfo*)(e->param());
        if ((data->Client == m_client) && data->Register){
            AGENTS_MAP::iterator it = m_agents.find(data->ID);
            if (it == m_agents.end()){
                agentInfo info;
                info.search = NULL;
                info.name   = data->Name;
                m_agents.insert(AGENTS_MAP::value_type(data->ID, info));
                m_client->get_agent_info(data->ID, "register");
            }
        }
        return NULL;
    }
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo){
        JabberAgentInfo *data = (JabberAgentInfo*)(e->param());
        AGENTS_MAP::iterator it = m_agents.find(data->ID);
        if (it != m_agents.end()){
            agentInfo &info = (*it).second;
            if (info.search == NULL){
                info.search = new JabberSearch(this, m_client, data->ID, QString::fromUtf8(info.name.c_str()));
                unsigned id = cmbAgents->count();
                wndInfo->addWidget(info.search, id + 1);
                cmbAgents->insertItem(QString::fromUtf8(info.name.c_str()));
                if (id == 0){
                    cmbAgents->setCurrentItem(0);
                    selectAgent(0);
                }
            }
            info.search->addWidget(data);
        }
        return NULL;
    }
    switch (e->type()){
    case EventClientChanged:
        if ((Client*)(e->param()) == m_client)
            statusChanged();
        break;
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
        cmbAgents->show();
        wndInfo->show();
        btnRegister->show();
        m_client->get_agents();
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
        cmbAgents->hide();
        wndInfo->hide();
        btnRegister->hide();
    }
}

void Services::selectAgent(int index)
{
    wndInfo->raiseWidget(index + 1);
}

void Services::regAgent()
{
}

void Services::apply(Client*, void*)
{
}

void Services::apply()
{
}

void Services::textChanged(const QString&)
{
}

void Services::search()
{
}

#ifndef WIN32
#include "services.moc"
#endif

