/***************************************************************************
                          search.cpp  -  description
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

#include "search.h"
#include "usercfg.h"
#include "core.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcombobox.h>

SearchDialog::SearchDialog()
{
    SET_WNDPROC("search")
    setIcon(Pict("find"));
    setButtonsPict(this);
    setCaption(caption());
    helpButton()->hide();
    m_client = (Client*)(-1);
    m_widget = NULL;
    fill();
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    connect(finishButton(), SIGNAL(clicked()), this, SLOT(apply()));
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::goNext()
{
    next();
}

void *SearchDialog::processEvent(Event *e)
{
    if ((e->type() == EventPluginChanged) || (e->type() == EventClientChanged))
        fill();
    return NULL;
}

void SearchDialog::fill()
{
    cmbType->clear();
    clients.clear();
    int currentItem = -1;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        Protocol *protocol = client->protocol();
        const CommandDef *descr = protocol->description();
        if ((descr->flags & PROTOCOL_SEARCH) ||
                ((descr->flags & PROTOCOL_SEARCH_ONLINE) && (client->getState() == Client::Connected))){
            unsigned n;
            for (n = 0; n < i; n++){
                Client *c = getContacts()->getClient(n);
                Protocol *p = c->protocol();
                unsigned flags = p->description()->flags;
                if ((flags & PROTOCOL_SEARCH) ||
                        ((flags & PROTOCOL_SEARCH_ONLINE) && (c->getState() == Client::Connected))){
                    if (p == protocol)
                        break;
                }
            }
            if (n < i)
                continue;
            if (client == m_client)
                currentItem = cmbType->count();
            cmbType->insertItem(Pict(descr->icon), i18n(descr->text));
            clients.push_back(client);
        }
    }
    if (m_client == NULL)
        currentItem = cmbType->count();
    cmbType->insertItem(Pict("nonim"), i18n("Non-IM contact"));
    clients.push_back(NULL);
    if (currentItem == -1){
        if (m_widget){
            while (currentPage() != firstPage)
                back();
            removePage(m_widget);
            delete m_widget;
            m_widget = NULL;
        }
        currentItem = 0;
    }
    cmbType->setCurrentItem(currentItem);
    typeChanged(currentItem);
}

void SearchDialog::closeEvent(QCloseEvent *e)
{
    SearchBase::closeEvent(e);
    emit finished();
}

void SearchDialog::reject()
{
    SearchBase::reject();
    emit finished();
}

void SearchDialog::accept()
{
    SearchBase::accept();
    emit finished();
}

void SearchDialog::typeChanged(int item)
{
    if ((item < 0) || (item >= (int)clients.size()))
        return;
    Client *client = clients[item];
    if (client){
        unsigned i;
        for (i = 0; i < getContacts()->nClients(); i++){
            if (client == getContacts()->getClient(i))
                break;
        }
        if (i >= getContacts()->nClients())
            client = NULL;
    }
    if (client == m_client)
        return;
    m_client = client;
    if (m_widget){
        removePage(m_widget);
        delete m_widget;
        m_widget = NULL;
    }
    if (m_client)
        m_widget = m_client->searchWindow();
    if (m_widget){
        addPage(m_widget, i18n("Search %1") .arg(i18n(m_client->protocol()->description()->text)));
        setFinishEnabled(firstPage, false);
    }else{
        setFinishEnabled(firstPage, true);
    }
}

void SearchDialog::apply()
{
    if ((currentPage() == firstPage) && (m_widget == NULL)){
        Contact *contact = new Contact;
        UserConfig *cfg = new UserConfig(contact, NULL);
        cfg->raisePage(CmdInfo);
        raiseWindow(cfg);
    }
}

#ifndef WIN32
#include "search.moc"
#endif

