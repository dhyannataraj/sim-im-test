/***************************************************************************
                          windock.h  -  description
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

#ifndef _WINDOCK_H
#define _WINDOCK_H

#include "simapi.h"

typedef struct WinDockData
{
    unsigned long	AutoHide;
} WinDocData;

class WinDockPlugin : public Plugin, public EventReceiver
{
public:
    WinDockPlugin(unsigned, const char*);
    virtual ~WinDockPlugin();
protected:
    virtual void *processEvent(Event*);
    virtual string getConfig();
    QWidget *getMainWindow();
    unsigned CmdAutoHide;
    PROP_BOOL(AutoHide);
    WinDockData data;
};

#endif

