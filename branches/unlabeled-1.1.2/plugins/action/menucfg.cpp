/***************************************************************************
                          menucfg.cpp  -  description
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

#include "menucfg.h"
#include "listview.h"
#include "action.h"

MenuConfig::MenuConfig(QWidget *parent, struct ActionUserData *data)
        : MenuConfigBase(parent)
{
    m_data   = data;

    lstMenu->addColumn(i18n("Item"));
    lstMenu->addColumn(i18n("Program"));
    lstMenu->setExpandingColumn(1);
    lstMenu->adjustColumn();
    connect(lstMenu, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));
}

MenuConfig::~MenuConfig()
{
}

void MenuConfig::resizeEvent(QResizeEvent *e)
{
    MenuConfigBase::resizeEvent(e);
    lstMenu->adjustColumn();
}

void MenuConfig::selectionChanged(QListViewItem*)
{
}

void MenuConfig::apply(void*)
{
}

#ifndef WIN32
#include "menucfg.moc"
#endif

