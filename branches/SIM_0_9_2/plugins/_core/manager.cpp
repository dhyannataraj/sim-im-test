/***************************************************************************
                          manager.cpp  -  description
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

#include "manager.h"
#include "newprotocol.h"
#include "connectionsettings.h"
#include "core.h"

#include <qlistview.h>
#include <qheader.h>
#include <qscrollbar.h>
#include <qpushbutton.h>

ConnectionManager::ConnectionManager(bool bModal)
        : ConnectionManagerBase(NULL, "manager", bModal)
{
    SET_WNDPROC("manager")
    setIcon(Pict("configure"));
    setButtonsPict(this);
    setCaption(caption());
    lstConnection->setHScrollBarMode(QScrollView::AlwaysOff);
    lstConnection->header()->hide();
    lstConnection->setSorting(1);
    fill();
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClient()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeClient()));
    connect(btnUp, SIGNAL(clicked()), this, SLOT(upClient()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(downClient()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateClient()));
    connect(lstConnection, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    m_bModal = bModal;
}

void ConnectionManager::fill(Client *current)
{
    lstConnection->clear();
    QListViewItem *curItem = NULL;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        Protocol *protocol = client->protocol();
        const CommandDef *descr = protocol->description();
        QString text = CorePlugin::m_plugin->clientName(client);
        QListViewItem *item = new QListViewItem(lstConnection, text);
        if (descr)
            item->setPixmap(0, Pict(descr->icon));
        if (current == client)
            curItem = item;
        QString index = QString::number(i);
        while (index.length() < 6)
            index = QString("0") + index;
        item->setText(1, index);
    }
    if (curItem)
        lstConnection->setCurrentItem(curItem);
    selectionChanged();
}

void ConnectionManager::selectionChanged()
{
    QListViewItem *item = lstConnection->currentItem();
    if (item == NULL){
        btnUp->setEnabled(false);
        btnDown->setEnabled(false);
        btnRemove->setEnabled(false);
        btnUpdate->setEnabled(false);
        return;
    }
    btnUpdate->setEnabled(true);
    btnRemove->setEnabled(true);
    int n = 0;
    int index = -1;
    for (QListViewItem *i = lstConnection->firstChild(); i; i = i->nextSibling(), n++){
        if (i == item)
            index = n;
    }
    btnUp->setEnabled(index > 0);
    btnDown->setEnabled(index < n - 1);
}

void ConnectionManager::closeEvent(QCloseEvent *e)
{
    ConnectionManagerBase::closeEvent(e);
    emit finished();
}

void ConnectionManager::resizeEvent(QResizeEvent *e)
{
    ConnectionManagerBase::resizeEvent(e);
    int w = lstConnection->width() - 4;
    if (lstConnection->verticalScrollBar()->isVisible())
        w -= lstConnection->verticalScrollBar()->width();
    lstConnection->setColumnWidth(0, w);
}

void ConnectionManager::addClient()
{
    NewProtocol pDlg(this);
    if (pDlg.exec())
        fill();
}

Client *ConnectionManager::currentClient()
{
    QListViewItem *item = lstConnection->currentItem();
    if (item == NULL)
        return NULL;
    unsigned n = 0;
    int index = -1;
    for (QListViewItem *i = lstConnection->firstChild(); i; i = i->nextSibling(), n++){
        if (i == item){
            index = n;
            break;
        }
    }
    if (index == -1)
        return NULL;
    return getContacts()->getClient(index);
}

void ConnectionManager::removeClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    delete client;
    fill();
}

void ConnectionManager::upClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    getContacts()->moveClient(client, true);
    fill(client);
}

void ConnectionManager::downClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    getContacts()->moveClient(client, false);
    fill(client);
}

void ConnectionManager::updateClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    unsigned n;
    for (n = 0; n < getContacts()->nClients(); n++){
        if (getContacts()->getClient(n) == client)
            break;
    }
    if (n >= getContacts()->nClients())
        return;
    Command cmd;
    cmd->id = CmdSetup;
    cmd->menu_id = CmdClient + n;
    Event e(EventCommandExec, cmd);
    e.process();
}

#ifndef WIN32
#include "manager.moc"
#endif

