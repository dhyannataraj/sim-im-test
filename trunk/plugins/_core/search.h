/***************************************************************************
                          search.h  -  description
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

#ifndef _SEARCH_H
#define _SEARCH_H

#include "simapi.h"
#include "stl.h"

#include <qmainwindow.h>

class CorePlugin;
class ListView;
class SearchBase;
class QStatusBar;
class QTimer;

typedef struct ClientWidget
{
    Client		*client;
    QWidget		*widget;
} ClientWidget;

class SearchDialog : public QMainWindow, public EventReceiver
{
    Q_OBJECT
public:
    SearchDialog();
    ~SearchDialog();
public slots:
    void setAdd(bool bAdd);
    void clientActivated(int);
    void aboutToShow(QWidget*);
    void resultShow(QWidget*);
    void resultDestroyed();
    void textChanged(const QString&);
    void toggled(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
    void showError(const QString&);
signals:
    void finished();
    void add(unsigned grp_id);
    void search();
protected slots:
    void searchClick();
    void addGroup(int);
    void setColumns(const QStringList&, int);
    void addItem(const QStringList&);
    void searchDone();
    void update();
protected:
    vector<ClientWidget>	m_widgets;
    void		setStatus();
    void		setAddButton();
    ListView	*m_result;
    QWidget		*m_current;
    QWidget		*m_currentResult;
    QWidget		*m_active;
    void		*processEvent(Event*);
    void		resizeEvent(QResizeEvent*);
    void		moveEvent(QMoveEvent*);
    void		closeEvent(QCloseEvent*);
    void		fillClients();
    void		attach(QWidget*);
    void		detach(QWidget*);
    bool		checkSearch(QWidget*, bool&);
    bool		m_bAdd;
    bool		m_bColumns;
    unsigned	m_id;
    unsigned	m_result_id;
    SearchBase	*m_search;
    QStatusBar	*m_status;
    QTimer		*m_update;
};

#endif

