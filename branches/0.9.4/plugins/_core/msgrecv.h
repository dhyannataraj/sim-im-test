/***************************************************************************
                          msgrecv.h  -  description
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

#ifndef _MSGRECV_H
#define _MSGRECV_H

#include "simapi.h"

#include <qtoolbutton.h>

class MsgEdit;

class MsgReceived : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgReceived(MsgEdit *parent, Message *msg, bool bOpen);
public slots:
    void init();
protected:
    void		*processEvent(Event*);
    unsigned	m_type;
    unsigned	m_id;
    unsigned	m_contact;
    string		m_client;
    bool		m_bOpen;
    MsgEdit	    *m_edit;
    Message		*m_msg;
};

#endif

