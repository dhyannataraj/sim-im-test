/***************************************************************************
                          statuswnd.cpp  -  description
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

#include "statuswnd.h"
#include "core.h"
#include "ballonmsg.h"

#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qframe.h>

StatusLabel::StatusLabel(QWidget *parent, Client *client, unsigned id)
        : QLabel(parent)
{
    m_client = client;
    m_bBlink = false;
    m_id = id;
    m_timer = NULL;
    m_bError = false;
    setPict();
}

void StatusLabel::setPict()
{
    const char *icon;
    const char *text;
    if (m_client->getState() == Client::Connecting){
        if (m_timer == NULL){
            m_timer = new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
            m_timer->start(1000);
            m_bBlink = false;
        }
        Protocol *protocol = m_client->protocol();
        text = I18N_NOOP("Connecting");
        unsigned status;
        if (m_bBlink){
            icon = "online";
            status = m_client->getManualStatus();
        }else{
            icon = "offline";
            status = STATUS_OFFLINE;
        }
        if (protocol){
            for (const CommandDef *cmd = protocol->statusList(); cmd->text; cmd++){
                if (cmd->id == status){
                    icon = cmd->icon;
                    break;
                }
            }
        }
    }else{
        if (m_timer){
            delete m_timer;
            m_timer = NULL;
        }
        if ((m_client->getState() == Client::Error) || (m_client->getState() == Client::AuthError)){
            icon = "error";
            text = I18N_NOOP("Error");
        }else{
            Protocol *protocol = m_client->protocol();
            const CommandDef *cmd = protocol->description();
            icon = cmd->icon;
            text = cmd->text;
            for (cmd = protocol->statusList(); cmd->text; cmd++){
                if (cmd->id == m_client->getStatus()){
                    icon = cmd->icon;
                    text = cmd->text;
                    break;
                }
            }
        }
    }
    QPixmap pict = Pict(icon);
    QString tip = CorePlugin::m_plugin->clientName(m_client);
    tip += "\n";
    tip += i18n(text);
    setPixmap(pict);
    QToolTip::add(this, tip);
    resize(pict.width(), pict.height());
    setFixedSize(pict.width(), pict.height());
};

void StatusLabel::timeout()
{
    m_bBlink = !m_bBlink;
    setPict();
}

void *StatusLabel::processEvent(Event *e)
{
    if ((e->type() == EventClientChanged) &&
            ((Client*)(e->param()) == m_client)){
        if (m_client->getState() == Client::Error){
            if (!m_bError && *m_client->errorString.c_str()){
                raiseWindow(topLevelWidget());
                BalloonMsg::message(i18n(m_client->errorString.c_str()), this);
            }
            m_bError = true;
        }else{
            m_bError = false;
        }
        setPict();
    }
    if (e->type() == EventIconChanged)
        setPict();
    return NULL;
}

void StatusLabel::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == RightButton){
        ProcessMenuParam mp;
        mp.id = m_id;
        mp.param = (void*)winId();
        mp.key	 = 0;
        Event eMenu(EventProcessMenu, &mp);
        QPopupMenu *popup = (QPopupMenu*)eMenu.process();
        if (popup)
            popup->popup(me->globalPos());
    }
}

StatusWnd::StatusWnd()
{
    setFrameStyle(NoFrame);
    WindowDef wnd;
    wnd.widget = this;
    wnd.bDown  = true;
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_lay = new QHBoxLayout(this);
    m_lay->setMargin(1);
    m_lay->setSpacing(2);
    m_lay->addStretch();
    addClients();
    Event e(EventAddStatus, &wnd);
    e.process();
}

void StatusWnd::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == RightButton){
        Command cmd;
        cmd->id = MenuConnections;
        Event e(EventGetMenu, &cmd);
        QPopupMenu *popup = (QPopupMenu*)(e.process());
        if (popup)
            popup->popup(me->globalPos());
    }
}

void *StatusWnd::processEvent(Event *e)
{
    if (e->type() == EventClientsChanged){
        addClients();
    }
    return NULL;
}

void StatusWnd::addClients()
{
    QObjectList * l = queryList("QWidget");
    QObjectListIt itObject( *l );
    QObject * obj;
    while ( (obj=itObject.current()) != 0 ) {
        ++itObject;
        delete obj;
    }
    delete l;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        QWidget *w = new StatusLabel(this, client, CmdClient + i);
        m_lay->addWidget(w);
        w->show();
    }
    repaint();
}

#ifndef WIN32
#include "statuswnd.moc"
#endif


