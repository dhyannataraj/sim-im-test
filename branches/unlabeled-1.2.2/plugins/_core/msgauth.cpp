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

#include "msgauth.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "core.h"

#include <qtimer.h>
#include <qtoolbutton.h>

MsgAuth::MsgAuth(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_type   = msg->type();
	m_edit   = parent;
    parent->m_edit->setTextFormat(PlainText);
	parent->m_edit->setReadOnly(false);
    QString text = msg->getPlainText();
    if (!text.isEmpty())
        parent->m_edit->setText(text);
    Command cmd;
    cmd->id    = CmdSend;
	cmd->flags = 0;
    cmd->param = parent;
    Event e(EventCommandChecked, cmd);
    e.process();
}

void MsgAuth::init()
{
    m_edit->m_edit->setFocus();
}

void *MsgAuth::processEvent(Event *e)
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
				cmd->flags &= ~BTN_HIDE;
				return NULL;
			case CmdNextMessage:
			case CmdMsgAnswer:
				cmd->flags |= BTN_HIDE;
				return NULL;
			}
		}
	}
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            AuthMessage *msg = new AuthMessage(m_type);
            msg->setText(msgText);
            msg->setContact(m_edit->m_userWnd->id());
            msg->setClient(m_client.c_str());
            m_edit->sendMessage(msg);
            return e->param();
        }
    }
    return NULL;
}

#ifndef WIN32
#include "msgauth.moc"
#endif

