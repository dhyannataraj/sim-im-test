/***************************************************************************
                          msgurl.cpp  -  description
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

#include "msgurl.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "userlist.h"
#include "core.h"

MsgUrl::MsgUrl(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit   = parent;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText("");
        m_edit->m_edit->setReadOnly(false);
    }
    m_edit->m_edit->setTextFormat(PlainText);
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    Command cmd;
    cmd->id    = CmdUrlInput;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    CToolEdit *edtUrl = (CToolEdit*)(e.process());
    if (edtUrl){
        connect(edtUrl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
        edtUrl->setText(static_cast<UrlMessage*>(msg)->getUrl());
        if (edtUrl->text().isEmpty()){
            string url;
            Event e(EventGetURL, &url);
            e.process();
            if (!url.empty()){
				url = url.substr(1);
				int n = url.find('\"');
				if (n > 0){
					string u = url.substr(0, n);
					edtUrl->setText(QString::fromLocal8Bit(u.c_str()));
					url = url.substr(n + 1);
					n = url.find('\"');
					if (n > 0)
						url = url.substr(n + 1);
				}
				n = url.find('\"');
				if (n > 0){
					url = url.substr(0, n);
					m_edit->m_edit->setText(QString::fromLocal8Bit(url.c_str()));
				}
			}
        }
        urlChanged(edtUrl->text());
    }
}

void MsgUrl::init()
{
    if (!m_edit->topLevelWidget()->isActiveWindow() || m_edit->topLevelWidget()->isMinimized())
        return;
    Command cmd;
    cmd->id    = CmdUrlInput;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    CToolEdit *edtUrl = (CToolEdit*)(e.process());
    if (edtUrl && edtUrl->text().isEmpty()){
        edtUrl->setFocus();
        return;
    }
    m_edit->m_edit->setFocus();
}

void MsgUrl::urlChanged(const QString &str)
{
    Command cmd;
    cmd->id = CmdSend;
    cmd->flags = str.isEmpty() ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();
}

void *MsgUrl::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if (cmd->id == CmdUrlInput)
                    cmd->flags &= ~BTN_HIDE;
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
            QString urlText;
            Command cmd;
            cmd->id    = CmdUrlInput;
            cmd->param = m_edit;
            Event eUrl(EventCommandWidget, cmd);
            CToolEdit *edtUrl = (CToolEdit*)(eUrl.process());
            if (edtUrl)
                urlText = edtUrl->text();
            if (!urlText.isEmpty()){
                UrlMessage *msg = new UrlMessage;
                msg->setContact(m_edit->m_userWnd->id());
                msg->setText(msgText);
                msg->setUrl(urlText);
                msg->setClient(m_client.c_str());
                m_edit->sendMessage(msg);
            }
            return e->param();
        }
    }
    return NULL;
}

#ifndef WIN32
#include "msgurl.moc"
#endif

