/***************************************************************************
                          msgfile.cpp  -  description
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

#include "msgfile.h"
#include "toolbtn.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "core.h"

#include <qtimer.h>
#include <qtoolbutton.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

MsgFile::MsgFile(CToolCustom *parent, Message *msg)
        : QLineEdit(parent)
{
    m_client = msg->client();
    for (QWidget *p = parent->parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("MsgEdit")){
            m_edit = static_cast<MsgEdit*>(p);
            break;
        }
    }
    m_edit->m_edit->setTextFormat(PlainText);
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    parent->addWidget(this);
    QToolButton *btn = new QToolButton(parent);
    btn->setIconSet(*Icon("fileopen"));
    btn->setTextLabel(i18n("Select file"));
    parent->addWidget(btn);
    btn->show();
    connect(btn, SIGNAL(clicked()), this, SLOT(selectFile()));
    parent->setText(i18n("File:"));
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    Command cmd;
    cmd->id    = CmdSend;
    cmd->param = m_edit;
    Event e(EventCommandWidget, cmd);
    btnSend = (QToolButton*)(e.process());
    setText(static_cast<FileMessage*>(msg)->getFile());
    changed(text());
    show();
}

void MsgFile::init()
{
    setFocus();
}

void MsgFile::changed(const QString &str)
{
    if (btnSend == NULL)
        return;
    btnSend->setEnabled(!str.isEmpty());
}

void MsgFile::selectFile()
{
    QString s = text();
#ifdef WIN32
    s.replace(QRegExp("\\\\"), "/");
#endif
    QStringList lst = QFileDialog::getOpenFileNames(QString::null, QString::null, topLevelWidget());
    if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].find(' ') >= 0))){
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
            *it = QString("\"") + *it + QString("\"");
        }
    }
    setText(lst.join(" "));
}

void *MsgFile::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            QString file = text();
            if (!file.isEmpty()){
                FileMessage *msg = new FileMessage;
                msg->setText(msgText);
                msg->setFile(file);
                msg->setContact(m_edit->m_userWnd->id());
                msg->setClient(m_client.c_str());
                m_edit->sendMessage(msg);
            }
            return e->param();
        }
    }
    return NULL;
}

#ifndef WIN32
#include "msgfile.moc"
#endif

