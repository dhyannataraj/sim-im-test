/***************************************************************************
                          msggen.h  -  description
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

#ifndef _MSGGEN_H
#define _MSGGEN_H

#include "simapi.h"

class MsgEdit;

class MsgGen : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgGen(MsgEdit *parent, Message *msg);
protected slots:
    void init();
	void emptyChanged(bool bEmpty);
protected:
    void *processEvent(Event*);
    string m_client;
	bool   m_bCanSend;
    MsgEdit	*m_edit;
};

#endif

