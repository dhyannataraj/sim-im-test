/***************************************************************************
                          autoaway.h  -  description
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

#ifndef _AUTOAWAY_H
#define _AUTOAWAY_H

#include "simapi.h"

typedef struct AutoAwayData
{
    unsigned long	AwayTime;
    unsigned		EnableAway;
    unsigned long	NATime;
    unsigned		EnableNA;
    unsigned		DisableAlert;
} AutoAwayData;

class QTimer;
class CorePlugin;

class AutoAwayPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    AutoAwayPlugin(unsigned, const char*);
    ~AutoAwayPlugin();
    PROP_ULONG(AwayTime);
    PROP_BOOL(EnableAway);
    PROP_ULONG(NATime);
    PROP_BOOL(EnableNA);
    PROP_BOOL(DisableAlert);
protected slots:
    void timeout();
protected:
    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    unsigned getIdleTime();
    bool bAway;
    bool bNA;
    CorePlugin *core;
    unsigned long oldStatus;
    QTimer *m_timer;
    AutoAwayData data;
};

#endif

