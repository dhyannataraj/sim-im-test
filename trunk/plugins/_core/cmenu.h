/***************************************************************************
                          cmenu.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CMENU_H
#define _CMENU_H

#include "simapi.h"
#include "stl.h"

#ifdef USE_KDE
#include <kpopupmenu.h>
#else
#include "kpopup.h"
#endif

typedef struct CMD
{
    unsigned	id;
    unsigned	base_id;
} CMD;

class EXPORT CMenu : public KPopupMenu
{
    Q_OBJECT
public:
    CMenu(CommandsDef *def);
    ~CMenu();
    void setParam(void *param);
protected slots:
    void showMenu();
    void menuActivated(int);
    void clearMenu();
protected:
    void processItem(CommandDef *s, bool &bSeparator, bool &bFirst, unsigned base_id);
    vector<CMD> m_cmds;
    CommandsDef *m_def;
    void *m_param;
};

#endif

