/***************************************************************************
                          userwnd.cpp  -  description
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

#include "userwnd.h"
#include "msgedit.h"
#include "msgview.h"
#include "toolbtn.h"
#include "userlist.h"
#include "core.h"
#include "container.h"
#include "history.h"

#include <qtoolbar.h>
#include <qtimer.h>

static DataDef userWndData[] =
    {
        { "EditHeight", DATA_ULONG, 1, 0 },
        { "EditBar", DATA_ULONG, 7, 0 },
        { "MessageType", DATA_ULONG, 1, MessageGeneric },
        { NULL, 0, 0, 0 }
    };

UserWnd::UserWnd(unsigned id, const char *cfg, bool bReceived)
        : QSplitter(Horizontal, NULL)
{
    load_data(userWndData, &data, cfg);
    m_id = id;
    m_bResize = false;
    m_bClosed = false;
    m_bTyping = false;
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_hSplitter = new QSplitter(Horizontal, this);
    m_splitter = new QSplitter(Vertical, m_hSplitter);
    m_list = NULL;
    m_view = NULL;

    if (cfg == NULL)
        memcpy(data.editBar, CorePlugin::m_plugin->data.editBar, sizeof(data.editBar));

    m_bBarChanged = true;
    if (CorePlugin::m_plugin->getContainerMode())
        bReceived = false;
    m_edit = new MsgEdit(m_splitter, this, bReceived);
    restoreToolbar(m_edit->m_bar, data.editBar);
    m_edit->m_bar->show();
    m_bBarChanged = false;

    connect(m_edit, SIGNAL(toolBarPositionChanged(QToolBar*)), this, SLOT(toolbarChanged(QToolBar*)));
    connect(CorePlugin::m_plugin, SIGNAL(modeChanged()), this, SLOT(modeChanged()));
    connect(m_edit, SIGNAL(heightChanged(int)), this, SLOT(editHeightChanged(int)));
    modeChanged();

    if (data.MessageType == 0)
        return;
    CommandDef *cmd = CorePlugin::m_plugin->messageTypes.find(data.MessageType);
    if (cmd == NULL)
        return;
    MessageDef *def = (MessageDef*)(cmd->param);
    Message *msg = def->create(NULL);
    setMessage(msg);
    delete msg;
}

UserWnd::~UserWnd()
{
    emit closed(this);
    free_data(userWndData, &data);
    Contact *contact = getContacts()->contact(id());
    if (contact && contact->getTemporary()){
        m_id = 0;
        delete contact;
    }
}

string UserWnd::getConfig()
{
    return save_data(userWndData, &data);
}

QString UserWnd::getName()
{
    Contact *contact = getContacts()->contact(m_id);
    return contact->getName();
}

const char *UserWnd::getIcon()
{
    Contact *contact = getContacts()->contact(m_id);
    unsigned long status = STATUS_UNKNOWN;
    unsigned style;
    const char *statusIcon;
    void *data;
    Client *client = m_edit->client(data);
    if (client){
        client->contactInfo(data, status, style, statusIcon);
    }else{
        contact->contactInfo(style, statusIcon);
    }
    return statusIcon;
}

void UserWnd::modeChanged()
{
    if (CorePlugin::m_plugin->getContainerMode()){
        if (m_view == NULL)
            m_view = new MsgView(m_splitter, m_id);
        m_splitter->moveToFirst(m_view);
        m_splitter->setResizeMode(m_edit, QSplitter::KeepSize);
        m_view->show();
        int editHeight = getEditHeight();
        if (editHeight == 0)
            editHeight = CorePlugin::m_plugin->getEditHeight();
        if (editHeight){
            QValueList<int> s;
            s.append(1);
            s.append(editHeight);
            m_bResize = true;
            m_splitter->setSizes(s);
            m_bResize = false;
        }
    }else{
        if (m_view){
            delete m_view;
            m_view = NULL;
        }
    }
}

void UserWnd::editHeightChanged(int h)
{
    if (!m_bResize && CorePlugin::m_plugin->getContainerMode()){
        setEditHeight(h);
        CorePlugin::m_plugin->setEditHeight(h);
    }
}

void UserWnd::toolbarChanged(QToolBar*)
{
    if (m_bBarChanged)
        return;
    saveToolbar(m_edit->m_bar, data.editBar);
    memcpy(CorePlugin::m_plugin->data.editBar, data.editBar, sizeof(data.editBar));
}

void UserWnd::setMessage(Message *msg)
{
    bool bSetFocus = false;
    if (topLevelWidget() && topLevelWidget()->inherits("Container")){
        Container *container = static_cast<Container*>(topLevelWidget());
        container->setMessageType(msg->type());
        if (container->wnd() == this)
            bSetFocus = true;
    }

    data.MessageType = msg->type();
    m_edit->setMessage(msg, bSetFocus);

    if ((m_view == NULL) || (msg->id() == 0))
        return;
    if (m_view->findMessage(msg))
        return;
    m_view->addMessage(msg);
}

void UserWnd::setStatus(const QString &status)
{
    m_status = status;
    emit statusChanged(this);
}

void UserWnd::showListView(bool bShow, bool bAdd)
{
    if (bShow){
        if (m_list == NULL){
            m_list = new UserList(m_hSplitter);
            m_hSplitter->setResizeMode(m_list, QSplitter::KeepSize);
            connect(m_list, SIGNAL(selectChanged()), this, SLOT(selectChanged()));
            if (bAdd)
                m_list->selected.push_back(m_id);
        }
        m_list->show();
        emit multiplyChanged();
        return;
    }
    if (m_list == NULL)
        return;
    delete m_list;
    m_list = NULL;
    emit multiplyChanged();
}

void UserWnd::selectChanged()
{
    emit multiplyChanged();
}

void UserWnd::closeEvent(QCloseEvent *e)
{
    QSplitter::closeEvent(e);
    m_bClosed = true;
    QTimer::singleShot(0, topLevelWidget(), SLOT(wndClosed()));
}

void UserWnd::markAsRead()
{
    if (m_view == NULL)
        return;
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ){
        if ((*it).contact != m_id){
            ++it;
            continue;
        }
        Message *msg = History::load((*it).id, (*it).client.c_str(), (*it).contact);
        CorePlugin::m_plugin->unread.erase(it);
        if (msg){
            Event e(EventMessageRead, msg);
            e.process();
            delete msg;
        }
        it = CorePlugin::m_plugin->unread.begin();
    }
}

#ifndef WIN32
#include "userwnd.moc"
#endif

