/***************************************************************************
                          homedir.h  -  description
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

#ifndef _HOMEDIR_H
#define _HOMEDIR_H

#include "simapi.h"

class HomeDirPlugin : public Plugin, public EventReceiver
{
public:
    HomeDirPlugin(unsigned base);
    string m_homeDir;
    string defaultPath();
#ifdef WIN32
    bool m_bDefault;
    bool m_bSave;
#endif
protected:
    void *processEvent(Event *e);
    string buildFileName(const char *name);
#ifdef WIN32
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
#endif
};

#endif

