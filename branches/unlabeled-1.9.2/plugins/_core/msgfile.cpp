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
#include <qregexp.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

MsgFile::MsgFile(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client   = msg->client();
    m_edit     = parent;
    m_bCanSend = false;
    if (m_edit->m_edit->isReadOnly()){
        m_edit->m_edit->setText("");
        m_edit->m_edit->setReadOnly(false);
    }
    m_edit->m_edit->setTextFormat(PlainText);
    QString t = msg->getPlainText();
    if (!t.isEmpty())
        m_edit->m_edit->setText(t);

    Command cmd;
    cmd->id		= CmdFileName;
    cmd->param	= parent;
    Event eWidget(EventCommandWidget, cmd);
    CToolEdit *edtName = (CToolEdit*)(eWidget.process());
    if (edtName){
        connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
        edtName->setText(static_cast<FileMessage*>(msg)->getFile());
    }
    changed(static_cast<FileMessage*>(msg)->getFile());
}

void MsgFile::init()
{
    Command cmd;
    cmd->id		= CmdFileName;
    cmd->param	= m_edit;
    Event eWidget(EventCommandWidget, cmd);
    CToolEdit *edtName = (CToolEdit*)(eWidget.process());
    if (edtName){
        if (edtName->text().isEmpty()){
            selectFile();
            return;
        }
        edtName->setFocus();
    }
}

void MsgFile::changed(const QString &str)
{
    if (m_bCanSend != str.isEmpty())
        return;
    m_bCanSend = !str.isEmpty();
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = m_bCanSend ? 0 : COMMAND_DISABLED;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();
}

void MsgFile::selectFile()
{
    Command cmd;
    cmd->id		= CmdFileName;
    cmd->param	= m_edit;
    Event eWidget(EventCommandWidget, cmd);
    CToolEdit *edtName = (CToolEdit*)(eWidget.process());
    if (edtName == NULL)
        return;
    QString s = edtName->text();
#ifdef WIN32
    s.replace(QRegExp("\\\\"), "/");
#endif
    QStringList lst = QFileDialog::getOpenFileNames(QString::null, QString::null, m_edit->topLevelWidget());
    if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].find(' ') >= 0))){
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
#ifdef WIN32
            (*it).replace(QRegExp("/"), "\\");
#endif
            *it = QString("\"") + *it + QString("\"");
        }
#ifdef WIN32
    }else{
        for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
            QString &s = *it;
            s.replace(QRegExp("/"), "\\");
        }
#endif
    }
    edtName->setText(lst.join(" "));
}

void *MsgFile::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                if (cmd->id == CmdFileName)
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
        if (cmd->param == m_edit){
            if (cmd->id == CmdSend){
                Command cmd;
                cmd->id		= CmdFileName;
                cmd->param	= m_edit;
                Event eWidget(EventCommandWidget, cmd);
                CToolEdit *edtName = (CToolEdit*)(eWidget.process());
                if (edtName == NULL)
                    return NULL;
                QString msgText = m_edit->m_edit->text();
                QString file = edtName->text();
                QStringList files;
                QString f;
                for (int i = 0; i < (int)file.length(); i++){
                    if (file[i] == '\"'){
                        f = trim(f);
                        if (!f.isEmpty())
                            files.append(f);
                        f = "";
                        for (i++; i < (int)file.length(); i++){
                            if (file[i] == '\"')
                                break;
                            f += file[i];
                        }
                        f = trim(f);
                        if (!f.isEmpty())
                            files.append(f);
                        f = "";
                        continue;
                    }
                    f += file[i];
                }
                f = trim(f);
                if (!f.isEmpty())
                    files.append(f);
                file = "";
                for (QStringList::Iterator it = files.begin(); it != files.end(); ++it){
                    if (!file.isEmpty())
                        file += ";";
                    file += quoteChars(*it, ";");
                }
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
            if (cmd->id == CmdFileName){
                selectFile();
                return e->param();
            }
        }
    }
    return NULL;
}

#ifndef WIN32
#include "msgfile.moc"
#endif

