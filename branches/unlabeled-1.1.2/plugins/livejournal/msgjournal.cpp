/***************************************************************************
                          msgjournal.cpp  -  description
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

#include "msgjournal.h"
#include "livejournal.h"
#include "msgedit.h"
#include "userwnd.h"
#include "ballonmsg.h"
#include "toolbtn.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>

MsgJournal::MsgJournal(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit  = parent;
    m_wnd   = new MsgJournalWnd(m_edit->m_frame);
    connect(m_wnd, SIGNAL(finished()), this, SLOT(frameDestroyed()));
    m_edit->m_layout->insertWidget(0, m_wnd);
    m_wnd->show();
    JournalMessage	*m = static_cast<JournalMessage*>(msg);
    m_ID	= m->getID();
    m_oldID = m->id();
    m_time  = m->getTime();
    m_wnd->edtSubj->setText(m->getSubject());
    m_wnd->cmbSecurity->setCurrentItem(m->getPrivate());
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact){
        clientData *data;
        ClientDataIterator it(contact->clientData);
        while ((data = ++it) != NULL){
            if ((m_client.empty() && (data->Sign == LIVEJOURNAL_SIGN)) || (m_client == it.client()->dataName(data))){
                LiveJournalClient *client = static_cast<LiveJournalClient*>(it.client());
                for (unsigned i = 1; i < client->getMoods(); i++){
                    const char *mood = client->getMood(i);
                    if ((mood == NULL) || (*mood == 0))
                        continue;
					QString s = mood;
					QString ts = i18n(mood);
					if (s != ts){
						s += "(";
						s += ts;
						s += ")";
					}
                    m_wnd->cmbMood->insertItem(s);
                }
                m_wnd->cmbMood->setCurrentItem(static_cast<JournalMessage*>(msg)->getMood());
                m_wnd->cmbMood->setMinimumSize(m_wnd->cmbMood->sizeHint());
                break;
            }
        }
    }
    m_edit->m_edit->setTextFormat(RichText);
    QString text = msg->getRichText();
    if (!text.isEmpty()){
        m_edit->m_edit->setText(text);
        m_edit->m_edit->moveCursor(QTextEdit::MoveEnd, false);
        if ((msg->getBackground() != msg->getForeground()) && !LiveJournalPlugin::core->getOwnColors()){
            m_edit->m_edit->setBackground(msg->getBackground());
            m_edit->m_edit->setForeground(msg->getForeground(), true);
        }
    }
    connect(m_edit->m_edit, SIGNAL(emptyChanged(bool)), this, SLOT(emptyChanged(bool)));
    emptyChanged(m_edit->m_edit->isEmpty());
    m_edit->m_edit->setParam(m_edit);
}

MsgJournal::~MsgJournal()
{
    if (m_wnd)
        delete m_wnd;
}

void MsgJournal::init()
{
    m_wnd->edtSubj->setFocus();
}

void MsgJournal::emptyChanged(bool bEmpty)
{
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = bEmpty ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();
}

void *MsgJournal::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if ((cmd->id == CmdDeleteJournalMessage + CmdReceived) && m_ID)
                    cmd->flags &= ~BTN_HIDE;
                return e->param();
            }
            switch (cmd->id){
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return e->param();
            case CmdTranslit:
            case CmdSmile:
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
        if (cmd->param == m_edit){
            if (cmd->id == CmdSend){
				QString msgText = m_edit->m_edit->text();
				if (!msgText.isEmpty())
					send(msgText);
            }else if (cmd->id == CmdDeleteJournalMessage + CmdReceived){
				QWidget *w = m_edit->m_bar;
				Command cmd;
				cmd->id		= CmdDeleteJournalMessage + CmdReceived;
                cmd->param	= m_edit;
                Event eWidget(EventCommandWidget, cmd);
                QWidget *btnRemove = (QWidget*)(eWidget.process());
                if (btnRemove)
					w = btnRemove;
				BalloonMsg::ask(NULL, i18n("Remove record from journal?"), w, SLOT(removeRecord(void*)), NULL, NULL, this);
			}
            return e->param();
        }
    }
    return NULL;
}

void MsgJournal::removeRecord(void*)
{
	send("");
}

void MsgJournal::send(const QString& msgText)
{
					JournalMessage *msg = new JournalMessage;
					msg->setText(msgText);
					msg->setContact(m_edit->m_userWnd->id());
					msg->setClient(m_client.c_str());
					msg->setFlags(MESSAGE_RICHTEXT);
					msg->setID(m_ID);
					msg->setOldID(m_oldID);
					msg->setTime(m_time);
					msg->setForeground(m_edit->m_edit->foreground().rgb() & 0xFFFFFF);
					msg->setBackground(m_edit->m_edit->background().rgb() & 0xFFFFFF);
					msg->setFont(LiveJournalPlugin::core->getEditFont());
					msg->setSubject(m_wnd->edtSubj->text());
					msg->setPrivate(m_wnd->cmbSecurity->currentItem());
					msg->setMood(m_wnd->cmbMood->currentItem());
					MsgSend ms;
					ms.edit = m_edit;
					ms.msg  = msg;
					Event e(EventRealSendMessage, &ms);
					e.process();
}

void MsgJournal::frameDestroyed()
{
    m_wnd = NULL;
}

MsgJournalWnd::MsgJournalWnd(QWidget *parent)
        : MsgJournalBase(parent)
{
}

MsgJournalWnd::~MsgJournalWnd()
{
    finished();
}

#ifndef WIN32
#include "msgjournal.moc"
#endif

