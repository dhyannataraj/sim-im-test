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
#include "toolbtn.h"
#include "socket.h"

#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qframe.h>
#include <qtoolbutton.h>

StatusLabel::StatusLabel(QWidget *parent, Client *client, unsigned id)
        : QLabel(parent)
{
    m_client = client;
    m_bBlink = false;
    m_id = id;
    m_timer = NULL;
    setPict();
}

void StatusLabel::setPict()
{
    string icon;
    const char *text;
    if (m_client->getState() == Client::Connecting){
        if (getSocketFactory()->isActive()){
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

            Protocol *protocol = m_client->protocol();
            const CommandDef *cmd = protocol->description();
            icon = cmd->icon;
            int n = icon.find('_');
            if (n > 0)
                icon = icon.substr(0, n);
            icon += "_inactive";
            text = I18N_NOOP("Inactive");
        }
    }else{
        if (m_timer){
            delete m_timer;
            m_timer = NULL;
        }
        if (m_client->getState() == Client::Error){
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
    QPixmap pict = Pict(icon.c_str());
    QString tip = CorePlugin::m_plugin->clientName(m_client);
    tip += "\n";
    tip += i18n(text);
    setPixmap(pict);
    QToolTip::add(this, tip);
    resize(pict.width(), pict.height());
    setFixedSize(pict.width(), pict.height());
}

void StatusLabel::timeout()
{
    m_bBlink = !m_bBlink;
    setPict();
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
        if (popup){
            QPoint pos = CToolButton::popupPos(this, popup);
            popup->popup(pos);
        }
    }
}

StatusFrame::StatusFrame(QWidget *parent)
        : QFrame(parent), EventReceiver(LowPriority + 1)
{
    setFrameStyle(NoFrame);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_frame = new QFrame(this);
    m_frame->show();
    m_lay = new QHBoxLayout(m_frame);
    m_lay->setMargin(1);
    m_lay->setSpacing(2);
    m_lay->addStretch();
    addClients();
}

void StatusFrame::mousePressEvent(QMouseEvent *me)
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

void *StatusFrame::processEvent(Event *e)
{
    CommandDef *cmd;
    switch (e->type()){
    case EventSocketActive:{
            {
                QObjectList *l = queryList("StatusLabel");
                QObjectListIt itObject(*l);
                QObject *obj;
                while ((obj=itObject.current()) != NULL) {
                    ++itObject;
                    StatusLabel *lbl = static_cast<StatusLabel*>(obj);
                    lbl->setPict();
                }
                delete l;
            }
            break;
        }
    case EventCheckState:
        cmd = (CommandDef*)(e->param());
        if ((cmd->menu_id == MenuStatusWnd) && (cmd->id == CmdStatusWnd)){
            unsigned n = 0;
            {
                QObjectList *l = queryList("StatusLabel");
                QObjectListIt itObject(*l);
                QObject *obj;
                while ((obj=itObject.current()) != NULL) {
                    ++itObject;
                    StatusLabel *lbl = static_cast<StatusLabel*>(obj);
                    if (lbl->x() + lbl->width() > width())
                        n++;
                }
                delete l;
            }
            CommandDef *cmds = new CommandDef[n + 1];
            memset(cmds, 0, sizeof(CommandDef) * (n + 1));
            QObjectList *l = queryList("StatusLabel");
            QObjectListIt itObject(*l);
            QObject *obj;
            n = 0;
            while ((obj=itObject.current()) != NULL) {
                ++itObject;
                StatusLabel *lbl = static_cast<StatusLabel*>(obj);
                if (lbl->x() + lbl->width() > width()){
                    cmds[n].id = 1;
                    cmds[n].text = "_";
                    cmds[n].text_wrk = strdup(CorePlugin::m_plugin->clientName(lbl->m_client).utf8());
                    cmds[n].popup_id = lbl->m_id;
                    if (lbl->m_client->getState() == Client::Error){
                        cmds[n].icon = "error";
                    }else{
                        Protocol *protocol = lbl->m_client->protocol();
                        const CommandDef *cmd = protocol->description();
                        cmds[n].icon = cmd->icon;
                        for (cmd = protocol->statusList(); cmd->text; cmd++){
                            if (cmd->id == lbl->m_client->getStatus()){
                                cmds[n].icon = cmd->icon;
                                break;
                            }
                        }
                    }
                    n++;
                }
            }
            delete l;
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return e->param();
        }
        break;
    case EventClientsChanged:
        addClients();
        break;
    case EventClientChanged:{
            StatusLabel *lbl = findLabel((Client*)(e->param()));
            if (lbl)
                lbl->setPict();
            break;
        }
    case EventIconChanged:{
            QObjectList *l = queryList("StatusLabel");
            QObjectListIt itObject(*l);
            QObject *obj;
            while ((obj=itObject.current()) != NULL) {
                ++itObject;
                static_cast<StatusLabel*>(obj)->setPict();
            }
            delete l;
            break;
        }
    }
    return NULL;
}

void StatusFrame::addClients()
{
    list<StatusLabel*> lbls;
    QObjectList* l = m_frame->queryList("StatusLabel");
    QObjectListIt itObject(*l);
    QObject *obj;
    while ((obj=itObject.current()) != NULL){
        ++itObject;
        lbls.push_back(static_cast<StatusLabel*>(obj));
    }
    delete l;
    for (list<StatusLabel*>::iterator it = lbls.begin(); it != lbls.end(); ++it)
        delete *it;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        QWidget *w = new StatusLabel(m_frame, client, CmdClient + i);
        m_lay->addWidget(w);
        w->show();
    }
    adjustPos();
    repaint();
}

StatusLabel *StatusFrame::findLabel(Client *client)
{
    QObjectList* l = m_frame->queryList("StatusLabel");
    QObjectListIt itObject(*l);
    QObject *obj;
    while ((obj=itObject.current()) != NULL){
        ++itObject;
        if (static_cast<StatusLabel*>(obj)->m_client == client){
            delete l;
            return static_cast<StatusLabel*>(obj);
        }
    }
    delete l;
    return NULL;
}

QSize StatusFrame::sizeHint() const
{
    QSize res = m_frame->sizeHint();
    res.setWidth(20);
    return res;
}

QSize StatusFrame::minimumSizeHint() const
{
    QSize res = m_frame->minimumSizeHint();
    res.setWidth(20);
    return res;
}

void StatusFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    adjustPos();
}

void StatusFrame::adjustPos()
{
    QSize s = m_frame->minimumSizeHint();
    m_frame->resize(s);
    m_frame->move(width() > s.width() ? width() - s.width() : 0, 0);
    emit showButton(width() < s.width());
    repaint();
    m_frame->repaint();
    QObjectList* l = m_frame->queryList("StatusLabel");
    QObjectListIt itObject(*l);
    QObject *obj;
    while ((obj=itObject.current()) != NULL){
        ++itObject;
        static_cast<StatusLabel*>(obj)->repaint();
    }
    delete l;
}

static const char * const arrow_h_xpm[] = {
            "9 7 3 1",
            "	    c None",
            ".	    c #000000",
            "+	    c none",
            "..++..+++",
            "+..++..++",
            "++..++..+",
            "+++..++..",
            "++..++..+",
            "+..++..++",
            "..++..+++"};

StatusWnd::StatusWnd()
{
    setFrameStyle(NoFrame);
    m_lay = new QHBoxLayout(this);
    m_frame = new StatusFrame(this);
    m_btn = new QToolButton(this);
    m_btn->setAutoRaise(true);
    m_btn->setPixmap( QPixmap((const char **)arrow_h_xpm));
    m_btn->setMinimumSize(QSize(10, 10));
    m_lay->addWidget(m_frame);
    m_lay->addWidget(m_btn);
    connect(m_frame, SIGNAL(showButton(bool)), this, SLOT(showButton(bool)));
    connect(m_btn, SIGNAL(clicked()), this, SLOT(clicked()));
    WindowDef wnd;
    wnd.widget = this;
    wnd.bDown  = true;
    Event e(EventAddStatus, &wnd);
    e.process();
}

void StatusWnd::showButton(bool bState)
{
    if (bState){
        m_btn->show();
    }else{
        m_btn->hide();
    }
}

void StatusWnd::clicked()
{
    Command cmd;
    cmd->popup_id = MenuStatusWnd;
    cmd->flags    = COMMAND_NEW_POPUP;
    Event e(EventGetMenu, cmd);
    QPopupMenu *popup = (QPopupMenu*)(e.process());
    if (popup){
        QPoint pos = CToolButton::popupPos(m_btn, popup);
        popup->popup(pos);
    }
}

BalloonMsg *StatusWnd::showError(const QString &text, QStringList &buttons, Client *client)
{
    if (!isVisible())
        return NULL;
    StatusLabel *lbl = m_frame->findLabel(client);
    if (lbl == NULL)
        return NULL;
	if (lbl->x() + lbl->width() > width())
		return NULL;
    return new BalloonMsg(NULL, text, buttons, lbl);
}

#ifndef WIN32
#include "statuswnd.moc"
#endif


