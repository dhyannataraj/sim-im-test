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

#include <qpixmap.h>
#include <qlineedit.h>

DeclineDlg::DeclineDlg(Message *msg)
: DeclineDlgBase(NULL, NULL, false, WDestructiveClose)
{
    m_msg = msg;
    SET_WNDPROC("decline")
    setIcon(Pict("file"));
    setButtonsPict(this);
    setCaption(caption());
}

DeclineDlg::~DeclineDlg()
{
}

void DeclineDlg::accept()
{
	string reason;
	reason = edtReason->text().utf8();
	messageDecline md;
	md.msg    = m_msg;
	md.reason = reason.c_str();
	Event e(EventMessageDecline, &md);
	e.process();
	DeclineDlgBase::accept();
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

