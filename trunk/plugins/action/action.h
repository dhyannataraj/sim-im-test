/***************************************************************************
                          action.h  -  description
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

#ifndef _ACTION_H
#define _ACTION_H

#include "simapi.h"

class CorePlugin;

typedef struct ActionUserData
{
    char	*OnLine;
    char	*Status;
    void	*Message;
    void	*Menu;
} ActionUserData;

class ActionPlugin : public Plugin, public EventReceiver
{
public:
    ActionPlugin(unsigned);
    virtual ~ActionPlugin();
    CorePlugin	*core;
    unsigned action_data_id;
protected:
    void *processEvent(Event*);
    QWidget *createConfigWindow(QWidget *parent);
};

#endif

