/***************************************************************************
                          msgrecv.cpp  -  description
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

#include "msgrecv.h"
#include "msgedit.h"
#include "msgview.h"
#include "toolbtn.h"
#include "history.h"
#include "core.h"
#include "textshow.h"

#include <qaccel.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qpopupmenu.h>

MsgReceived::MsgReceived(CToolCustom *parent, Message *msg)
        : QObject(parent)
{
    m_id = msg->id();
    m_contact = msg->contact();
    m_client = msg->client();

    m_edit = NULL;
    for (QWidget *p = parent->parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("MsgEdit")){
            m_edit = static_cast<MsgEdit*>(p);
            break;
        }
    }

    parent->setText(i18n(" "));

    Event eMenu(EventGetMenuDef, (void*)MenuMsgCommand);
    CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());

    MessageDef *mdef = NULL;
    if (msg->getFlags() & MESSAGE_RECEIVED){
        unsigned type = msg->type();
        for (;;){
            CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(type);
            if (msgCmd == NULL)
                break;
            mdef = (MessageDef*)(msgCmd->param);
            if (mdef->base_type == 0)
                break;
            type = mdef->base_type;
        }
    }

    if (mdef && mdef->cmd){
        unsigned n = 0;
        for (const CommandDef *d = mdef->cmd; d->text; d++, n++){
			if (d->flags & COMMAND_CHECK_STATE){
				CommandDef c = *d;
				c.param = msg;
				Event e(EventCheckState, &c);
				if (e.process() == NULL)
					continue;
			}
            CmdButton *btn;
            btn = new CmdButton(parent, CmdMsgSpecial + n, d->text);
            connect(btn, SIGNAL(command(CmdButton*)), this, SLOT(command(CmdButton*)));
            btn->show();
        }
    }

    CommandsList it(*cmdsMsg, true);
    CommandDef *c;
    while ((c = ++it) != NULL){
        c->param = msg;
        Event e(EventCheckState, c);
        if (!e.process())
            continue;
        CmdButton *btn;
        btn = new CmdButton(parent, c->id, c->text);
        connect(btn, SIGNAL(command(unsigned)), this, SLOT(command(unsigned)));
        btn->show();
    }
    if (((msg->getFlags() & MESSAGE_RECEIVED) == 0) || CorePlugin::m_plugin->getContainerMode()){
        connect(m_edit->m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    }else{
        m_edit->m_edit->setReadOnly(true);
        m_edit->m_edit->setTextFormat(QTextEdit::RichText);
        QString p = msg->presentation();
        if (p.isEmpty())
            p = msg->getRichText();
        Event e(EventEncodeText, &p);
        e.process();
        p = MsgViewBase::parseText(p, CorePlugin::m_plugin->getOwnColors(), CorePlugin::m_plugin->getUseSmiles());
        m_edit->m_edit->setText(p);
        if ((msg->getBackground() != msg->getForeground()) && !CorePlugin::m_plugin->getOwnColors()){
            m_edit->m_edit->setBackground(msg->getBackground());
            m_edit->m_edit->setForeground(msg->getForeground());
        }
        for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
            if (((*it).id == msg->id()) &&
                    ((*it).contact == msg->contact()) &&
                    ((*it).client == msg->client())){
                CorePlugin::m_plugin->unread.erase(it);
                Event eRead(EventMessageRead, msg);
                eRead.process();
                break;
            }
        }
        m_edit->setupNext();
    }
}

void *MsgReceived::processEvent(Event *e)
{
	if (e->type() == EventMessageDeleted){
		Message *msg = (Message*)(e->param());
		if (msg->id() == m_id)
	        QTimer::singleShot(0, m_edit, SLOT(goNext()));
	}
	return NULL;
}

void MsgReceived::command(CmdButton *btn)
{
    unsigned id = btn->id();
    Message *msg = History::load(m_id, m_client.c_str(), m_contact);
    if (msg == NULL)
        return;
    if (id >= CmdMsgSpecial){
        MessageDef *mdef = NULL;
        unsigned type = msg->type();
        for (;;){
            CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(type);
            if (msgCmd == NULL)
                break;
            mdef = (MessageDef*)(msgCmd->param);
            if (mdef->base_type == 0)
                break;
            type = mdef->base_type;
        }
        if (mdef && mdef->cmd){
            unsigned n = id - CmdMsgSpecial;
            for (const CommandDef *d = mdef->cmd; d->text; d++){
                if (n-- == 0){
                    CommandDef cmd = *d;
                    cmd.param = msg;
					cmd.menu_id = 0;
                    if (d->popup_id){
                        Event e(EventGetMenu, &cmd);
                        QPopupMenu *popup = (QPopupMenu*)(e.process());
                        if (popup){
                            QPoint pos = CToolButton::popupPos(btn, popup);
                            popup->popup(pos);
                        }
                        return;
                    }
                    Event eCmd(EventCommandExec, &cmd);
                    eCmd.process();
                    return;
                }
            }
        }
    }
    Command cmd;
    cmd->id = id;
    cmd->menu_id = MenuMsgCommand;
    cmd->param = msg;
    Event e(EventCommandExec, cmd);
    if (e.process())
        QTimer::singleShot(0, m_edit, SLOT(goNext()));
    delete msg;
}

void MsgReceived::init()
{
}

void MsgReceived::textChanged()
{
    QTimer::singleShot(0, m_edit, SLOT(setInput()));
}

CmdButton::CmdButton(CToolCustom *parent, unsigned id, const char *text)
        : QToolButton(parent)
{
    m_id   = id;
    m_text = text;
    QSizePolicy p = sizePolicy();
    p.setHorData(QSizePolicy::Expanding);
    setSizePolicy(p);
    setText();
    connect(this, SIGNAL(clicked()), this, SLOT(click()));
    parent->addWidget(this);
    setAutoRaise(true);
}

void CmdButton::setText()
{
    QString text = i18n(m_text);
    int key = QAccel::shortcutKey(text);
    setAccel(key);
    QString t = text;
    int pos = t.find("<br>");
    if (pos >= 0) t = t.left(pos);
    QToolButton::setTextLabel(t);
    t = text;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    QToolTip::add(this, t);
}

void CmdButton::paintEvent(QPaintEvent*)
{
    QPixmap pict(width(), height());
    QPainter p(&pict);
    QWidget *pw;
    for (pw = parentWidget(); pw; pw = pw->parentWidget()){
        if (pw->backgroundPixmap()){
            p.drawTiledPixmap(0, 0, width(), height(), *pw->backgroundPixmap(), x(), y());
            break;
        }
    }
    if (pw == NULL)
        p.fillRect(0, 0, width(), height(), colorGroup().button());
#if QT_VERSION < 300
    style().drawToolButton(this, &p);
#else
    drawButton(&p);
#endif
    QRect rc(4, 4, width() - 4, height() - 4);
    const QColorGroup &cg = isEnabled() ? palette().active() : palette().disabled();
    p.setPen(cg.text());
    QString text = i18n(m_text);
    p.drawText(rc, AlignLeft | AlignVCenter | ShowPrefix, text);
    p.end();
    p.begin(this);
    p.drawPixmap(0, 0, pict);
    p.end();
}

QSize CmdButton::sizeHint() const
{
    QSize res = QSize(fontMetrics().width(i18n(m_text)) + 8, 22);
    return res;
}

QSize CmdButton::minimumSizeHint() const
{
    return sizeHint();
}

void CmdButton::click()
{
    emit command(this);
}

#ifndef WIN32
#include "msgrecv.moc"
#endif

