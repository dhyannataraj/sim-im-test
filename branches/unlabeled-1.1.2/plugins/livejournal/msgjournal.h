/***************************************************************************
                          msgjournal.h  -  description
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

#ifndef _MSGJOURNAL_H
#define _MSGJOURNAL_H

#include "simapi.h"
#include "msgjournalbase.h"

class MsgEdit;

class MsgJournalWnd : public MsgJournalBase
{
    Q_OBJECT
public:
    MsgJournalWnd(QWidget*);
    ~MsgJournalWnd();
signals:
    void finished();
};

class MsgJournal : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgJournal(MsgEdit *parent, Message *msg);
    ~MsgJournal();
protected slots:
    void init();
    void frameDestroyed();
    void emptyChanged(bool bEmpty);
	void removeRecord(void*);
protected:
    virtual void *processEvent(Event*);
	void		send(const QString&);
    string		m_client;
    unsigned	m_ID;
    unsigned	m_oldID;
    unsigned	m_time;
    MsgEdit		*m_edit;
    MsgJournalWnd	*m_wnd;
};

#endif

