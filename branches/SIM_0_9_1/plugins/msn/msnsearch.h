/***************************************************************************
                          msnsearch.h  -  description
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

#ifndef _MSNSEARCH_H
#define _MSNSEARCH_H

#include "simapi.h"
#include "msnsearchbase.h"

class MSNClient;
class MSNResult;
class QWizard;

class MSNSearch : public MSNSearchBase, public EventReceiver
{
    Q_OBJECT
public:
    MSNSearch(MSNClient *client);
    ~MSNSearch();
signals:
    void goNext();
protected slots:
    void textChanged(const QString&);
    void search();
    void startSearch();
protected:
    void *processEvent(Event*);
    void showEvent(QShowEvent *e);
    void changed();
    void fillGroup();
    QWizard	  *m_wizard;
    MSNResult *m_result;
    MSNClient *m_client;
};

#endif

