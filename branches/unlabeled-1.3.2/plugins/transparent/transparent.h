/***************************************************************************
                          transparent.h  -  description
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

#ifndef _TRANSPARENT_H
#define _TRANSPARENT_H

#include "simapi.h"

typedef struct TransparentData
{
    unsigned long	Transparency;
#ifdef WIN32
    unsigned long	IfInactive;
#endif
} TransparentData;

class QTimer;
class TransparentTop;

class TransparentPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    TransparentPlugin(unsigned, const char*);
    virtual ~TransparentPlugin();
    PROP_ULONG(Transparency);
#ifdef WIN32
    PROP_BOOL(IfInactive);
#endif
    void	setState();
public slots:
    void tick();
	void tickMouse();
    void topDestroyed();
protected:
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool eventFilter(QObject*, QEvent*);
    QWidget *getMainWindow();
    bool    m_bState;
    virtual void *processEvent(Event *e);
#ifdef WIN32
    unsigned startTime;
    QTimer   *timer;
	bool	 m_bHaveMouse;
	bool	 m_bActive;
#else
    TransparentTop  *top;
#endif
    TransparentData data;
};

#endif

