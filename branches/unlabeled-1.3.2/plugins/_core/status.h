/***************************************************************************
                          status.h  -  description
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

#ifndef _STATUS_H
#define _STATUS_H

#include "simapi.h"

class QTimer;
class CorePlugin;

class CommonStatus : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    CommonStatus();
    ~CommonStatus();
protected slots:
    void timeout();
    void setBarStatus();
protected:
    QTimer *m_timer;
    void *processEvent(Event*);
    void rebuildStatus();
    void checkInvisible();
	bool m_bInit;
    bool m_bBlink;
    bool m_bConnected;
};

#endif

