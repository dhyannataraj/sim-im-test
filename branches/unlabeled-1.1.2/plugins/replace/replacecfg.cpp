/***************************************************************************
                          replacecfg.cpp  -  description
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

#include "replacecfg.h"
#include "replace.h"
#include "listview.h"
#include "intedit.h"

#include <qframe.h>
#include <qlayout.h>
#include <qheader.h>

ReplaceCfg::ReplaceCfg(QWidget *parent, ReplacePlugin *plugin)
        : ReplaceCfgBase(parent)
{
    m_plugin = plugin;
    lstKeys->addColumn(i18n("You type"));
    lstKeys->addColumn(i18n("You send"));
    lstKeys->setExpandingColumn(1);
	for (unsigned i = 0; i < m_plugin->getKeys(); i++){
		new QListViewItem(lstKeys, m_plugin->getKey(i), m_plugin->getValue(i));
	}
	new QListViewItem(lstKeys, "", "");
    lstKeys->adjustColumn();
	m_edit = new IntLineEdit(lstKeys->viewport());
	lstKeys->setFocusProxy(m_edit);
	m_col = 0;
	lstKeys->setCurrentItem(lstKeys->firstChild());
	setEdit();
	connect(lstKeys, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
	connect(lstKeys->header(), SIGNAL(sizeChange(int,int,int)), this, SLOT(sizeChange(int,int,int)));
}

ReplaceCfg::~ReplaceCfg()
{
}

void ReplaceCfg::apply()
{
}

void ReplaceCfg::resizeEvent(QResizeEvent *e)
{
    ReplaceCfgBase::resizeEvent(e);
    lstKeys->adjustColumn();
}

void ReplaceCfg::setEdit()
{
	QListViewItem *item = lstKeys->currentItem();
	if (item == NULL){
		m_edit->hide();
	}else{
		QRect rc = lstKeys->itemRect(item);
		if (m_col){
			rc.setLeft(lstKeys->columnWidth(0));
			rc.setWidth(lstKeys->columnWidth(1));
		}else{
			rc.setWidth(lstKeys->columnWidth(0));
		}
		m_edit->move(rc.left(), rc.top());
		m_edit->resize(rc.width(), rc.height());
		m_edit->show();
	}
}

void ReplaceCfg::selectionChanged()
{
	setEdit();
}

void ReplaceCfg::sizeChange(int,int,int)
{
	setEdit();
}

#ifndef WIN32
#include "replacecfg.moc"
#endif

