/***************************************************************************
                          warndlg.cpp  -  description
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

#include "warndlg.h"
#include "ballonmsg.h"
#include "icqclient.h"
#include "icqmessage.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qcheckbox.h>
#include <qtimer.h>

WarnDlg::WarnDlg(QWidget *parent, ICQUserData *data, ICQClient *client)
        : WarnDlgBase(parent, NULL, false, WDestructiveClose)
{
    SET_WNDPROC("warn")
    setIcon(Pict("error"));
    setButtonsPict(this);
    setCaption(caption());
    m_client  = client;
    m_data    = data;
    m_msg     = NULL;
    m_contact = 0;
    Contact *contact;
    if (m_client->findContact(client->screen(data).c_str(), NULL, false, contact))
        m_contact = contact->id();
    lblInfo->setText(lblInfo->text().replace(QRegExp("\\%1"), m_client->screen(data).c_str()));
    chkAnon->setChecked(m_client->getWarnAnonimously());
}

WarnDlg::~WarnDlg()
{
    if (m_msg){
        Event e(EventMessageCancel, m_msg);
        e.process();
    }
}

void WarnDlg::accept()
{
    m_msg = new WarningMessage;
    m_msg->setClient(m_client->dataName(m_data).c_str());
    m_msg->setContact(m_contact);
    m_msg->setAnonymous(chkAnon->isChecked());
    m_client->setWarnAnonimously(chkAnon->isChecked());
    if (!((Client*)m_client)->send(m_msg, m_data)){
        delete m_msg;
        m_msg = NULL;
        showError(I18N_NOOP("Send failed"));
        return;
    }
    buttonOk->setEnabled(false);
}

void WarnDlg::showError(const char *error)
{
    buttonOk->setEnabled(true);
    BalloonMsg::message(i18n(error), buttonOk);
}

void *WarnDlg::processEvent(Event *e)
{
    if (e->type() == EventMessageSent){
        Message *msg = (Message*)(e->param());
        if (msg == m_msg){
            m_msg = NULL;
            const char *err = msg->getError();
            if (err && *err){
                showError(msg->getError());
            }else{
                QTimer::singleShot(0, this, SLOT(close()));
            }
        }
    }
    return NULL;
}

#ifndef WIN32
#include "warndlg.moc"
#endif

