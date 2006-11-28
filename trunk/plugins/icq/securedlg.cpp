/***************************************************************************
                          securedlg.cpp  -  description
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

#include "icons.h"
#include "securedlg.h"
#include "icqclient.h"
#include "icqmessage.h"

#include <qpixmap.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlabel.h>

using namespace SIM;

SecureDlg::SecureDlg(ICQClient *client, unsigned contact, ICQUserData *data)
        : SecureDlgBase(NULL, "securedlg", false, WDestructiveClose)
{
    SET_WNDPROC("secure")
    setIcon(Pict("encrypted"));
    setButtonsPict(this);
    setCaption(caption());
    m_client  = client;
    m_contact = contact;
    m_data    = data;
    m_msg	  = NULL;
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QTimer::singleShot(0, this, SLOT(start()));
}

SecureDlg::~SecureDlg()
{
    if (m_msg)
        EventMessageCancel(m_msg).process();
}

void SecureDlg::start()
{
    m_msg = new Message(MessageOpenSecure);
    m_msg->setContact(m_contact);
    m_msg->setClient(m_client->dataName(m_data));
    m_msg->setFlags(MESSAGE_NOHISTORY);
    if (!static_cast<Client*>(m_client)->send(m_msg, m_data)){
        delete m_msg;
        error(I18N_NOOP("Request secure channel fail"));
    }
}

bool SecureDlg::processEvent(Event *e)
{
    switch(e->type()) {
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        switch(ec->action()) {
            case EventContact::eDeleted: {
                close();
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventMessageSent: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg != m_msg)
            return NULL;
        QString err = msg->getError();
        if (!err.isEmpty()){
            error(err);
        }else{
            m_msg = NULL;
            close();
        }
        return (void*)1;
    }
    default:
        break;
    }
    return NULL;
}

void SecureDlg::error(const QString &err)
{
    QString errText = i18n(err);
    m_msg = NULL;
    lblError->setText(errText);
    btnCancel->setText(i18n("&Close"));
}

#ifndef NO_MOC_INCLUDES
#include "securedlg.moc"
#endif

