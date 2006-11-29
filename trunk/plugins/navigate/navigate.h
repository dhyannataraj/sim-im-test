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

#include "sim_export.h"

#include "cfg.h"
#include "event.h"
#include "plugins.h"

struct NavigateData
{
#ifdef WIN32
    SIM::Data NewWindow;
#else
    SIM::Data Browser;
    SIM::Data Mailer;
#endif
#ifdef USE_KDE
    SIM::Data UseKDE;
#endif
};

class NavigatePlugin : public SIM::Plugin, public SIM::EventReceiver
{
public:
    NavigatePlugin(unsigned, Buffer *name);
    virtual ~NavigatePlugin();
protected:
    virtual bool processEvent(SIM::Event *e);
    QString parseUrl(const QString &text);
    unsigned long CmdMail;
    unsigned long CmdMailList;
    unsigned long CmdCopyLocation;
    unsigned long MenuMail;
#ifdef WIN32
    PROP_BOOL(NewWindow);
#else
    PROP_STR(Browser);
    PROP_STR(Mailer);
#endif
#ifdef USE_KDE
    PROP_BOOL(UseKDE);
#endif
    virtual QCString getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    NavigateData data;
    friend class NavCfg;
};

#endif

