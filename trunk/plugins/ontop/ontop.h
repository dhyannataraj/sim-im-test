/***************************************************************************
                          ontop.h  -  description
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

#ifndef _ONTOP_H
#define _ONTOP_H

#include "simapi.h"

typedef struct OnTopData
{
    unsigned long	OnTop;
    unsigned long	InTask;
    unsigned long	ContainerOnTop;
} OnTopData;

class OnTopPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    OnTopPlugin(unsigned, const char*);
    virtual ~OnTopPlugin();
protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void *processEvent(Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    void getState();
    void setState();
    QWidget *getMainWindow();
    unsigned CmdOnTop;
    PROP_BOOL(OnTop);
    PROP_BOOL(InTask);
    PROP_BOOL(ContainerOnTop);
    OnTopData data;
    friend class OnTopCfg;
};

#endif

