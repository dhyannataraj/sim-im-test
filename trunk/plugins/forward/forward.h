/***************************************************************************
                          forward.h  -  description
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

#ifndef _FORWARD_H
#define _FORWARD_H

#include "cfg.h"
#include "event.h"
#include "plugins.h"

struct ForwardUserData
{
    SIM::Data	Phone;
    SIM::Data	Send1st;
    SIM::Data	Translit;
};

class CorePlugin;

class ForwardPlugin : public SIM::Plugin, public SIM::EventReceiver
{
public:
    ForwardPlugin(unsigned);
    virtual ~ForwardPlugin();
    unsigned long user_data_id;
protected:
    CorePlugin	*core;
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool processEvent(SIM::Event *e);
};

#endif

