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
#include "additem.h"

#include <qpushbutton.h>
#include <qlineedit.h>

MenuConfig::MenuConfig(QWidget *parent, struct ActionUserData *data)
        : MenuConfigBase(parent)
{
    m_data   = data;

    lstMenu->addColumn(i18n("Item"));
    lstMenu->addColumn(i18n("Program"));
    lstMenu->setExpandingColumn(1);
    lstMenu->adjustColumn();
    connect(lstMenu, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
	connect(btnEdit, SIGNAL(clicked()), this, SLOT(edit()));
	connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
	selectionChanged(NULL);
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
	if (lstMenu->currentItem()){
		btnEdit->setEnabled(true);
		btnRemove->setEnabled(true);
	}else{
		btnEdit->setEnabled(false);
		btnRemove->setEnabled(false);
	}
}

void MenuConfig::add()
{
	AddItem add(topLevelWidget());
	if (add.exec()){
		new QListViewItem(lstMenu, add.edtItem->text(), add.edtPrg->text());
		lstMenu->adjustColumn();
	}
}

void MenuConfig::edit()
{
	QListViewItem *item = lstMenu->currentItem();
	if (item == NULL)
		return;
	AddItem add(topLevelWidget());
	add.edtItem->setText(item->text(0));
	add.edtPrg->setText(item->text(1));
	if (add.exec()){
		item->setText(0, add.edtItem->text());
		item->setText(1, add.edtPrg->text());
		lstMenu->adjustColumn();
	}
}

void MenuConfig::remove()
{
	QListViewItem *item = lstMenu->currentItem();
	if (item == NULL)
		return;
	delete item;
}

void MenuConfig::apply(void*)
{
}

#ifndef WIN32
#include "menucfg.moc"
#endif

