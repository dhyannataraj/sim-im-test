/***************************************************************************
                          msggen.cpp  -  description
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

#include "msggen.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "userlist.h"
#include "core.h"

#include <qtimer.h>
#include <qtoolbutton.h>
#include <qaccel.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qregexp.h>

#ifdef USE_KDE
#include <kcolordialog.h>
#include <kfontdialog.h>
#else
#include <qcolordialog.h>
#include <qfontdialog.h>
#endif

MsgGen::MsgGen(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit   = parent;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText("");
        m_edit->m_edit->setReadOnly(false);
    }
    m_edit->m_edit->setTextFormat(RichText);
    if (msg->getFlags() & MESSAGE_INSERT){
        QString text = msg->getPlainText();
        m_edit->m_edit->insert(text, false, true, true);
    }else{
        QString text = msg->getRichText();
        if (!text.isEmpty()){
            m_edit->m_edit->setText(text);
            m_edit->m_edit->moveCursor(QTextEdit::MoveEnd, false);
            if ((msg->getBackground() != msg->getForeground()) && !CorePlugin::m_plugin->getOwnColors()){
                m_edit->m_edit->setBackground(msg->getBackground());
                m_edit->m_edit->setForeground(msg->getForeground(), true);
            }
        }
    }
    connect(m_edit->m_edit, SIGNAL(emptyChanged(bool)), this, SLOT(emptyChanged(bool)));
    emptyChanged(m_edit->m_edit->isEmpty());
    m_edit->m_edit->setParam(m_edit);
}

void MsgGen::init()
{
    m_edit->m_edit->setFocus();
}

void MsgGen::emptyChanged(bool bEmpty)
{
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = bEmpty ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();
}

void *MsgGen::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                return e->param();
            }
            switch (cmd->id){
            case CmdTranslit:
            case CmdSmile:
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return e->param();
            case CmdNextMessage:
            case CmdMsgAnswer:
                e->process(this);
                cmd->flags |= BTN_HIDE;
                return e->param();
            }
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            if (!msgText.isEmpty()){
                string s;
                s = msgText.local8Bit();
                log(L_DEBUG, "Send: %s", s.c_str());
                Message *msg = new Message;
                msg->setText(msgText);
                msg->setContact(m_edit->m_userWnd->id());
                msg->setClient(m_client.c_str());
                msg->setFlags(MESSAGE_RICHTEXT);
                msg->setForeground(m_edit->m_edit->foreground().rgb() & 0xFFFFFF);
                msg->setBackground(m_edit->m_edit->background().rgb() & 0xFFFFFF);
                msg->setFont(CorePlugin::m_plugin->getEditFont());
                m_edit->sendMessage(msg);
            }
            return e->param();
        }
    }
    return NULL;
}

#ifndef WIN32
#include "msggen.moc"
#endif

