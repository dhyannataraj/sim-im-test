/***************************************************************************
                          toolsetup.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TOOLSETUP_H
#define _TOOLSETUP_H

#include "defs.h"

#include "toolsetupbase.h"
#include "toolbtn.h"

class ToolBarSetup : public ToolBarSetupBase
{
    Q_OBJECT
public:
    ToolBarSetup(const ToolBarDef *def, list<unsigned long> *active);
    const ToolBarDef *def;
    list<unsigned long> *m_active;
    static void show(const ToolBarDef *def, list<unsigned long> *active);
protected slots:
    void selectionChanged();
    void addClick();
    void removeClick();
    void upClick();
    void downClick();
    void applyClick();
    void okClick();
protected:
    list<unsigned long> active;
    bool bDirty;
    void addButton(QListBox *lst, const ToolBarDef *d);
    void setButtons();
};

#endif

