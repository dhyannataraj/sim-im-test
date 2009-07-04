/***************************************************************************
                          menucfg.h  -  description
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

#ifndef _MENUCFG_H
#define _MENUCFG_H

#include "ui_menucfgbase.h"

struct ActionUserData;

class MenuConfig : public QWidget, public Ui::MenuConfigBase
{
    Q_OBJECT
public:
    MenuConfig(QWidget *parent, ActionUserData *data);
    virtual ~MenuConfig();
public Q_SLOTS:
    void apply(void*);
    void itemSelectionChanged();
    void add();
    void edit();
    void remove();
protected:
    ActionUserData *m_data;
};

#endif

