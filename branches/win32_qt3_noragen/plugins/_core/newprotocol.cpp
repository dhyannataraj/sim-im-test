/***************************************************************************
                          newprotocol.cpp  -  description
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

#include "newprotocol.h"
#include "connectwnd.h"
#include "core.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtimer.h>

static bool cmp_protocol(Protocol *p1, Protocol *p2)
{
    const CommandDef *cmd1 = p1->description();
    const CommandDef *cmd2 = p2->description();
    QString s1 = i18n(cmd1->text);
    QString s2 = i18n(cmd2->text);
    return s1 < s2;
}

NewProtocol::NewProtocol(QWidget *parent)
        : NewProtocolBase(parent, "new_protocol", true)
{
    m_setup  = NULL;
    m_client = NULL;
    m_last   = NULL;
    m_connectWnd = NULL;
    m_bConnected = false;
    m_bConnect = false;
    m_bStart   = (parent == NULL);
    SET_WNDPROC("protocol")
    setIcon(Pict("configure"));
    setButtonsPict(this);
    setCaption(caption());
    helpButton()->hide();
    for (unsigned n = 0;; n++){
        Event e(EventPluginGetInfo, (void*)n);
        pluginInfo *info = (pluginInfo*)e.process();
        if (info == NULL)
            break;
        if (info->info == NULL){
            Event e(EventLoadPlugin, info->name);
            e.process();
            if (info->info && !(info->info->flags & (PLUGIN_PROTOCOL & ~PLUGIN_NOLOAD_DEFAULT))){
                Event e(EventUnloadPlugin, info->name);
                e.process();
            }
        }
        if ((info->info == NULL) || !(info->info->flags & (PLUGIN_PROTOCOL & ~PLUGIN_NOLOAD_DEFAULT)))
            continue;
        info->bDisabled = false;
        Event eApply(EventApplyPlugin, info->name);
        eApply.process();
    }
    Protocol *protocol;
    ContactList::ProtocolIterator it;
    while ((protocol = ++it) != NULL){
        const CommandDef *cmd = protocol->description();
        if (cmd == NULL)
            continue;
        m_protocols.push_back(protocol);
    }
    sort(m_protocols.begin(), m_protocols.end(), cmp_protocol);
    for (unsigned i = 0; i < m_protocols.size(); i++){
        const CommandDef *cmd = m_protocols[i]->description();
        cmbProtocol->insertItem(Pict(cmd->icon), i18n(cmd->text));
    }
    connect(cmbProtocol, SIGNAL(activated(int)), this, SLOT(protocolChanged(int)));
    cmbProtocol->setCurrentItem(0);
    protocolChanged(0);
    connect(this, SIGNAL(selected(const QString&)), this, SLOT(pageChanged(const QString&)));
}

NewProtocol::~NewProtocol()
{
    if (m_connectWnd)
        delete m_connectWnd;
    if (m_setup)
        delete m_setup;
    if (m_client)
        delete m_client;
    for (unsigned n = 0;; n++){
        Event e(EventPluginGetInfo, (void*)n);
        pluginInfo *info = (pluginInfo*)e.process();
        if (info == NULL)
            break;
        if ((info->info == NULL) ||
                !(info->info->flags & (PLUGIN_PROTOCOL & ~PLUGIN_NOLOAD_DEFAULT)))
            continue;
        unsigned i;
        for (i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (client->protocol()->plugin() == info->plugin)
                break;
        }
        if (i < getContacts()->nClients())
            continue;
        info->bDisabled = true;
        Event eApply(EventApplyPlugin, info->name);
        eApply.process();
        Event eUnload(EventUnloadPlugin, info->name);
        eUnload.process();
    }
}

void NewProtocol::protocolChanged(int n)
{
    if (m_last){
        removePage(m_last);
        delete m_last;
        m_last = NULL;
    }
    if (m_connectWnd){
        removePage(m_connectWnd);
        delete m_connectWnd;
        m_connectWnd = NULL;
    }
    if (m_setup){
        removePage(m_setup);
        delete m_setup;
        m_setup = NULL;
    }
    if (m_client){
        delete m_client;
        m_client = NULL;
    }
    if ((n < 0) || (n >= (int)(m_protocols.size())))
        return;
    Protocol *protocol = m_protocols[n];
    m_client = protocol->createClient(NULL);
    if (m_client == NULL)
        return;
    m_setup = m_client->setupWnd();
    if (m_setup == NULL){
        delete m_client;
        m_client = NULL;
        return;
    }
    connect(m_setup, SIGNAL(okEnabled(bool)), this, SLOT(okEnabled(bool)));
    connect(this, SIGNAL(apply()), m_setup, SLOT(apply()));
    addPage(m_setup, i18n(protocol->description()->text));
    m_connectWnd = new ConnectWnd(m_bStart);
    addPage(m_connectWnd, i18n(protocol->description()->text));
    if (m_bStart){
        m_last = new QWidget;
        addPage(m_last, i18n(protocol->description()->text));
    }
    setNextEnabled(currentPage(), true);
    setIcon(Pict(protocol->description()->icon));
    Event e(EventRaiseWindow, this);
    e.process();
}

void NewProtocol::okEnabled(bool bEnable)
{
    setNextEnabled(m_setup, bEnable);
}

void NewProtocol::pageChanged(const QString&)
{
    if (currentPage() == m_connectWnd){
        emit apply();
        m_bConnect = true;
        unsigned status = CorePlugin::m_plugin->getManualStatus();
        if (status == STATUS_OFFLINE)
            status = STATUS_ONLINE;
        m_client->setStatus(status, false);
        m_connectWnd->setConnecting(true);
        setBackEnabled(m_connectWnd, false);
        setNextEnabled(currentPage(), false);
        setFinishEnabled(m_connectWnd, false);
    }
    if (m_last && (currentPage() == m_last)){
        setFinishEnabled(m_connectWnd, false);
        cancelButton()->show();
        backButton()->show();
        finishButton()->hide();
        showPage(protocolPage);
        protocolChanged(0);
    }
}

void NewProtocol::reject()
{
    if (m_bConnect){
        m_client->setStatus(STATUS_OFFLINE, false);
        setBackEnabled(m_connectWnd, true);
        m_bConnect = false;
        back();
        return;
    }
    QWizard::reject();
}

void NewProtocol::loginComplete()
{
    if (m_client == NULL)
        return;
    m_bConnect = false;
    m_bConnected = true;
    m_client->setStatus(CorePlugin::m_plugin->getManualStatus(), true);
    m_connectWnd->setConnecting(false);
    setNextEnabled(currentPage(), true);
    setFinishEnabled(m_connectWnd, true);
    getContacts()->addClient(m_client);
    m_client = NULL;
    cancelButton()->hide();
    backButton()->hide();
    Event e(EventSaveState);
    e.process();
}

void *NewProtocol::processEvent(Event *e)
{
    if (m_client == NULL)
        return NULL;
    if (m_bConnect){
        clientErrorData *d;
        switch (e->type()){
        case EventClientChanged:
            if (m_client->getState() == Client::Connected){
                QTimer::singleShot(0, this, SLOT(loginComplete()));
                return NULL;
            }
            break;
        case EventClientError:
            d = (clientErrorData*)(e->param());
            if (d->client == m_client){
                m_connectWnd->setErr(i18n(d->err_str),
                                     (d->code == AuthError) ? m_client->protocol()->description()->accel : NULL);
                m_bConnect = false;
                m_client->setStatus(STATUS_OFFLINE, false);
                setBackEnabled(m_connectWnd, true);
                setFinishEnabled(m_connectWnd, false);
                return e->param();
            }
            break;
        }
    }
    return NULL;
}

#ifndef WIN32
#include "newprotocol.moc"
#endif

