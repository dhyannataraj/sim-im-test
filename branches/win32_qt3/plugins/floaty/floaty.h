/***************************************************************************
                          floaty.h  -  description
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

#ifndef _FLOATY_H
#define _FLOATY_H

#include "simapi.h"

typedef struct FloatyUserData
{
    Data	X;
    Data	Y;
} FloatyUserData;

class FloatyWnd;
class CorePlugin;
class QTimer;

class FloatyPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    FloatyPlugin(unsigned);
    virtual ~FloatyPlugin();
    CorePlugin *core;
    bool m_bBlink;
    void startBlink();
protected slots:
    void showPopup();
    void unreadBlink();
protected:
    FloatyWnd *findFloaty(unsigned id);
    virtual void *processEvent(Event*);
    unsigned CmdFloaty;
    unsigned user_data_id;
    QPoint	 popupPos;
    QTimer	 *unreadTimer;
    unsigned popupId;
    friend class FloatyWnd;
};

#endif

