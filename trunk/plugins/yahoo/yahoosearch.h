/***************************************************************************
                          yahoosearch.h  -  description
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

#ifndef _YAHOOSEARCH_H
#define _YAHOOSEARCH_H

#include "simapi.h"
#include "yahoosearchbase.h"

class YahooClient;
class YahooResult;
class QWizard;

class YahooSearch : public YahooSearchBase, public EventReceiver
{
    Q_OBJECT
public:
    YahooSearch(YahooClient *client);
    ~YahooSearch();
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
    QWizard		*m_wizard;
    YahooResult *m_result;
    YahooClient *m_client;
};

#endif

