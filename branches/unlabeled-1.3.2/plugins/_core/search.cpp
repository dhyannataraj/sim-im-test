/***************************************************************************
                          search.cpp  -  description
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

#include "search.h"
#include "usercfg.h"
#include "core.h"
#include "listview.h"
#include "nonim.h"
#include "searchall.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qobjectlist.h>

const unsigned RESULT_ID	= 0x1000;

SearchDialog::SearchDialog()
{
    SET_WNDPROC("search")
    setIcon(Pict("find"));
    setButtonsPict(this);
    setCaption(caption());
	m_current = NULL;
	m_bAdd = true;
	setAdd(false);
	btnOptions->setIconSet(*Icon("1downarrow"));
	btnAdd->setIconSet(*Icon("add"));
	connect(wndCondition, SIGNAL(aboutToShow(QWidget*)), this, SLOT(aboutToShow(QWidget*)));
	fillClients();
	clientActivated(0);
	connect(cmbClients, SIGNAL(activated(int)), this, SLOT(clientActivated(int)));
	m_result = new ListView(wndResult);
	m_result->addColumn(i18n("Nick"));
	m_result->addColumn(i18n("First name"));
	m_result->addColumn(i18n("Last name"));
	m_result->addColumn(i18n("E-mail"));
	m_result->setShowSortIndicator(true);
	m_result->setExpandingColumn(3);
	m_result->setFrameShadow(QFrame::Sunken);
	m_result->setLineWidth(1);
	wndResult->addWidget(m_result, RESULT_ID);
	wndResult->raiseWidget(RESULT_ID);
	aboutToShow(wndCondition->visibleWidget());
}

SearchDialog::~SearchDialog()
{
    saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::resizeEvent(QResizeEvent *e)
{
	SearchBase::resizeEvent(e);
	m_result->adjustColumn();
	if (isVisible())
		saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::moveEvent(QMoveEvent *e)
{
	SearchBase::moveEvent(e);
	if (isVisible())
		saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::closeEvent(QCloseEvent *e)
{
	SearchBase::closeEvent(e);
	emit finished();
}

void SearchDialog::setAdd(bool bAdd)
{
	if (m_bAdd == bAdd)
		return;
	m_bAdd = bAdd;
	QString text;
	const QIconSet *icon = NULL;
	if (m_bAdd){
		icon = Icon("add");
		text = i18n("&Add");
	}else{
		icon = Icon("find");
		text = i18n("&Search");
	}
	btnSearch->setText(text);
	if (icon)
		btnSearch->setIconSet(*icon);
}

void SearchDialog::fillClients()
{
	cmbClients->clear();
	cmbClients->insertItem(Pict("find"), i18n("All networks"));
	wndCondition->addWidget(new SearchAll(wndCondition), cmbClients->count());
	cmbClients->insertItem(Pict("nonim"), i18n("Non-IM contact"));
	wndCondition->addWidget(new NonIM(wndCondition), cmbClients->count());
}

void SearchDialog::clientActivated(int n)
{
	wndCondition->raiseWidget(n + 1);
}

void SearchDialog::textChanged(const QString&)
{
	bool bEnable = false;
	QObjectList *l = topLevelWidget()->queryList("QLineEdit");
	QObjectListIt it(*l);
	QObject *obj;
	while ((obj=it.current()) != NULL){
		QLineEdit *edit = static_cast<QLineEdit*>(obj);
		if (edit->isEnabled()){
			if (!edit->text().isEmpty()){
				const QValidator *v = edit->validator();
				if (v){
					QString text = edit->text();
					int pos = 0;
					if (v->validate(text, pos) == QValidator::Acceptable){
						bEnable = true;
					}else{
						bEnable = false;
						break;
					}
				}else{
					bEnable = true;
				}
			}
		}
		++it;
    }
	delete l;     		
	btnSearch->setEnabled(bEnable);
}

void SearchDialog::aboutToShow(QWidget *w)
{
	if (m_current){
		QObjectList *l = topLevelWidget()->queryList("QLineEdit");
		QObjectListIt it(*l);
		QObject *obj;
		while ((obj=it.current()) != NULL){
			disconnect(obj, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
			++it;
	    }
		delete l;     		
	}
	m_current = w;
	QObjectList *l = topLevelWidget()->queryList("QLineEdit");
	QObjectListIt it(*l);
	QObject *obj;
	while ((obj=it.current()) != NULL){
		connect(obj, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
		++it;
    }
	delete l;
	textChanged("");
}

#ifndef WIN32
#include "search.moc"
#endif

