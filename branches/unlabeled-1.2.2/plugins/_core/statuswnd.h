/***************************************************************************
                          statuswnd.h  -  description
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

#ifndef _STATUSWND_H
#define _STATUSWND_H

#include "simapi.h"

#include <qframe.h>
#include <qlabel.h>

class CorePlugin;

class QHBoxLayout;
class QTimer;

class StatusLabel : public QLabel
{
    Q_OBJECT
public:
    StatusLabel(QWidget *parent, Client *client, unsigned id);
protected slots:
    void timeout();
protected:
    void mousePressEvent(QMouseEvent *e);
    void setPict();
    QTimer		*m_timer;
    Client		*m_client;
    unsigned	m_id;
    bool		m_bBlink;
	friend class StatusWnd;
};

class StatusWnd : public QFrame, public EventReceiver
{
    Q_OBJECT
public:
    StatusWnd();
protected slots:
    void addClients();
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void *processEvent(Event *e);
	StatusLabel *findLabel(Client*);
    QHBoxLayout	*m_lay;
};


#endif

