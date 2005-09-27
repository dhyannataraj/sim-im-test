/***************************************************************************
                          declinedlg.cpp  -  description
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

#include "declinedlg.h"

#include <QPixmap>
#include <QLineEdit>

DeclineDlg::DeclineDlg(Message *msg)
        : QDialog( NULL, Qt::WA_DeleteOnClose)
{
    setupUi( this);
    m_msg = msg;
    SET_WNDPROC("decline")
    setIcon(Pict("file").pixmap());
    setButtonsPict(this);
    setCaption(caption());
}

DeclineDlg::~DeclineDlg()
{
}

void DeclineDlg::accept()
{
    string reason;
    reason = static_cast<string>(edtReason->text().toUtf8());
    messageDecline md;
    md.msg    = m_msg;
    md.reason = reason.c_str();
    Event e(EventMessageDecline, &md);
    e.process();
    accept();
}

void *DeclineDlg::processEvent(Event *e)
{
    if (e->type() == EventMessageDeleted){
        Message *msg = (Message*)(e->param());
        if (msg->id() == m_msg->id())
            close();
    }
    return NULL;
}

#ifndef WIN32
#include "declinedlg.moc"
#endif

