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

#include "searchbase.h"

class CorePlugin;

class SearchDialog : public SearchBase, public EventReceiver
{
    Q_OBJECT
public:
    SearchDialog();
    ~SearchDialog();
signals:
    void finished();
protected slots:
    void typeChanged(int);
    void goNext();
    void apply();
protected:
    virtual void *processEvent(Event*);
    virtual void closeEvent(QCloseEvent*);
    virtual void moveEvent(QMoveEvent*);
    virtual void resizeEvent(QResizeEvent*);
    vector<Client*> clients;
    void reject();
    void accept();
    void fill();
    Client	*m_client;
    QWidget	*m_widget;
};

#endif

