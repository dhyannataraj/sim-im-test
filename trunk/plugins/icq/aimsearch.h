/***************************************************************************
                          aimsearch.h  -  description
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

#ifndef _AIMSEARCH_H
#define _AIMSEARCH_H

#include "simapi.h"
#include "aimsearchbase.h"

class ICQClient;
class ICQSearchResult;
class QWizard;

class AIMSearch : public AIMSearchBase, public EventReceiver
{
    Q_OBJECT
public:
    AIMSearch(ICQClient *client);
    ~AIMSearch();
signals:
    void goNext();
protected slots:
    void search();
    void startSearch();
    void textChanged(const QString&);
    void currentChanged(QWidget*);
    void resultFinished();
protected:
    void showEvent(QShowEvent *e);
    void *processEvent(Event*);
    void fillGroups();
    void changed();
    ICQSearchResult *m_result;
    ICQClient *m_client;
    QWizard	*m_wizard;
};

#endif

