/***************************************************************************
                          journalsearch.h  -  description
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

#ifndef _JOURNALSEARCH_H
#define _JOURNALSEARCH_H

#include "simapi.h"
#include "journalsearchbase.h"

class LiveJournalClient;
class JournalResult;
class QWizard;

class JournalSearch : public JournalSearchBase
{
    Q_OBJECT
public:
    JournalSearch(LiveJournalClient *client);
    ~JournalSearch();
signals:
    void goNext();
protected slots:
    void textChanged(const QString&);
    void search();
    void startSearch();
protected:
    void showEvent(QShowEvent *e);
    void changed();
    QWizard	  *m_wizard;
    JournalResult *m_result;
    LiveJournalClient *m_client;
};

#endif

