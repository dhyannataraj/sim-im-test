/***************************************************************************
                          alertmsg.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _ALERTMSG_H
#define _ALERTMSG_H

#include "defs.h"
#include "alertmsgbase.h"

class QTimer;

class AlertMsgDlg : public AlertMsgBase
{
    Q_OBJECT
public:
    AlertMsgDlg(QWidget *p, unsigned long uin);
protected slots:
    void tick();
protected:
    QTimer *timer;
    int timeLeft;
};

#endif

