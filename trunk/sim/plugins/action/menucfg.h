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

#include "menucfgbase.h"
#include "simapi.h"
//Added by qt3to4:
#include <QResizeEvent>
#include <QDialog>

class Q3ListViewItem;

class MenuConfig : public QDialog, public Ui::MenuConfigBase
{
    Q_OBJECT
public:
    MenuConfig(QWidget *parent, struct ActionUserData *data);
    virtual ~MenuConfig();
public slots:
    void apply(void*);
    void selectionChanged(Q3ListViewItem*);
    void add();
    void edit();
    void remove();
protected:
    void resizeEvent(QResizeEvent *e);
    struct ActionUserData	*m_data;
};

#endif

