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

MsgReceived::MsgReceived(MsgEdit *parent, Message *msg, bool bOpen)
        : QObject(parent), EventReceiver(HighPriority - 1)
{
    m_id	  = msg->id();
    m_contact = msg->contact();
    m_client  = msg->client();
    m_edit    = parent;
    m_bOpen	  = bOpen;
    m_msg	  = msg;
    m_type	  = msg->baseType();

    if (m_bOpen){
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
            m_edit->m_edit->setForeground(msg->getForeground(), true);
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
    }else{
        connect(m_edit->m_edit, SIGNAL(textChanged()), m_edit, SLOT(setInput()));
    }
}

void *MsgReceived::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        unsigned id = cmd->bar_grp;
        if (cmd->param == m_edit){
            MessageDef *mdef = NULL;
            CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(m_type);
            if (msgCmd)
                mdef = (MessageDef*)(msgCmd->param);
            if (mdef && mdef->cmdReceived){
                for (const CommandDef *d = mdef->cmdReceived; d->text; d++){
                    if (d->popup_id && (d->popup_id == cmd->menu_id)){
                        Message *msg = History::load(m_id, m_client.c_str(), m_contact);
                        if (msg){
                            CommandDef c = *cmd;
                            c.param = msg;
                            m_edit->execCommand(&c);
                        }
                        return e->param();
                    }
                }
            }

            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                Message *msg = History::load(m_id, m_client.c_str(), m_contact);
                if (msg){
                    CommandDef c = *cmd;
                    c.id   -= CmdReceived;
                    c.param = msg;
                    m_edit->execCommand(&c);
                }
                return e->param();
            }
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= 0x1000) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                switch (cmd->id - CmdReceived){
                case CmdMsgQuote:
                case CmdMsgForward:{
                        CommandDef c = *cmd;
                        Message *msg = m_msg;
                        if (msg == NULL)
                            msg = History::load(m_id, m_client.c_str(), m_contact);
                        if (msg){
                            c.id   -= CmdReceived;
                            c.param = msg;
                            Event e(EventCheckState, &c);
                            if (e.process())
                                cmd->flags &= ~BTN_HIDE;
                            if (m_msg == NULL)
                                delete msg;
                        }
                        return e->param();
                    }
                }
                MessageDef *mdef = NULL;
                CommandDef *msgCmd = CorePlugin::m_plugin->messageTypes.find(m_type);
                if (msgCmd)
                    mdef = (MessageDef*)(msgCmd->param);
                if (mdef && mdef->cmdReceived){
                    for (const CommandDef *d = mdef->cmdReceived; d->text; d++){
                        if (d->id + CmdReceived == cmd->id){
                            if (d->flags & COMMAND_CHECK_STATE){
                                Message *msg = m_msg;
                                if (msg == NULL)
                                    msg = History::load(m_id, m_client.c_str(), m_contact);
                                if (msg){
                                    CommandDef c = *d;
                                    c.param = msg;
                                    Event e(EventCheckState, &c);
                                    if (e.process())
                                        cmd->flags &= ~BTN_HIDE;
                                    if (m_msg == NULL)
                                        delete msg;
                                }
                            }else{
                                cmd->flags &= ~BTN_HIDE;
                            }
                            return e->param();
                        }
                    }
                }
                return e->param();
            }
            if (cmd->id == CmdMsgAnswer){
                e->process(this);
                cmd->flags |= BTN_HIDE;
                if (CorePlugin::m_plugin->getContainerMode() == 0)
                    cmd->flags &= ~BTN_HIDE;
                return e->param();
            }

            if (m_bOpen){
                switch (cmd->id){
                case CmdTranslit:
                case CmdSmile:
                case CmdSend:
                case CmdSendClose:
                    e->process(this);
                    cmd->flags |= BTN_HIDE;
                    return e->param();
                case CmdNextMessage:
                    e->process(this);
                    cmd->flags |= BTN_HIDE;
                    if (CorePlugin::m_plugin->getContainerMode() == 0)
                        cmd->flags &= ~BTN_HIDE;
                    return e->param();;
                }
            }
        }
    }
    if (e->type() == EventMessageDeleted){
        Message *msg = (Message*)(e->param());
        if (msg->id() == m_id)
            QTimer::singleShot(0, m_edit, SLOT(goNext()));
    }
    return NULL;
}

void MsgReceived::init()
{
    m_msg = NULL;
}

#ifndef WIN32
#include "msgrecv.moc"
#endif

