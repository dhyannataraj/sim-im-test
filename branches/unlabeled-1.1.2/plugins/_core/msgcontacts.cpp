/***************************************************************************
                          msgcontacts.cpp  -  description
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

#include "msgcontacts.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "userlist.h"
#include "core.h"

#include <qlayout.h>

MsgContacts::MsgContacts(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit = parent;
    m_list = new UserList(m_edit->m_frame);
    m_edit->m_layout->addWidget(m_list);
    m_edit->m_edit->hide();
    m_edit->m_edit->setTextFormat(QTextEdit::PlainText);
    connect(m_list, SIGNAL(selectChanged()), this, SLOT(changed()));
    ContactsMessage *m = static_cast<ContactsMessage*>(msg);
    QString contacts = m->getContacts();
    while (contacts.length()){
        QString item = getToken(contacts, ';');
        QString url = getToken(item, ',');
        QString proto = getToken(url, ':');
        if (proto == "sim"){
            unsigned contact_id = atol(url.latin1());
            if (getContacts()->contact(contact_id))
                m_list->selected.push_back(contact_id);
        }
    }
    m_list->show();
    changed();
    connect(m_edit, SIGNAL(finished()), this, SLOT(editFinished()));
    connect(m_list, SIGNAL(finished()), this, SLOT(listFinished()));
}

MsgContacts::~MsgContacts()
{
    if (m_edit && m_edit->m_edit)
        m_edit->m_edit->show();
    if (m_list)
        delete m_list;
}

void MsgContacts::editFinished()
{
    m_edit = NULL;
}

void MsgContacts::listFinished()
{
    m_list = NULL;
}

void MsgContacts::changed()
{
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = m_list->selected.empty() ? COMMAND_DISABLED : 0;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();
}

void MsgContacts::init()
{
    m_list->setFocus();
}

void *MsgContacts::processEvent(Event *e)
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
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            QString contacts;
            for (list<unsigned>::iterator it = m_list->selected.begin(); it != m_list->selected.end(); ++it){
                Contact *contact = getContacts()->contact(*it);
                if (contact){
                    if (!contacts.isEmpty())
                        contacts += ";";
                    contacts += QString("sim:%1,%2") .arg(*it) .arg(contact->getName());
                }
            }
            if (!contacts.isEmpty()){
                ContactsMessage *msg = new ContactsMessage;
                msg->setContact(m_edit->m_userWnd->id());
                msg->setContacts(contacts);
                msg->setClient(m_client.c_str());
                m_edit->sendMessage(msg);
            }
            return e->param();
        }
    }
    return NULL;
}


#ifndef WIN32
#include "msgcontacts.moc"
#endif

