/***************************************************************************
                          navigate.h  -  description
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

#ifndef _NAVIGATE_H
#define _NAVIGATE_H

#include "simapi.h"

typedef struct NavigateData
{
#ifdef WIN32
	Data NewWindow;
#else
    Data Browser;
    Data Mailer;
#endif
} NavigateData;

class NavigatePlugin : public Plugin, public EventReceiver
{
public:
    NavigatePlugin(unsigned, const char *name);
    virtual ~NavigatePlugin();
protected:
    virtual void *processEvent(Event*);
    QString parseUrl(const QString &text);
    unsigned CmdMail;
    unsigned CmdMailList;
    unsigned MenuMail;
#ifdef WIN32
	PROP_BOOL(NewWindow);
#else
    PROP_STR(Browser);
    PROP_STR(Mailer);
#endif
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    NavigateData data;
    friend class NavCfg;
};

#endif

