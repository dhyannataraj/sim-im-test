/***************************************************************************
                          journalresult.h  -  description
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

#ifndef _JOURNALRESULT_H
#define _JOURNALRESULT_H

#include "simapi.h"
#include "journalresultbase.h"

class LiveJournalClient;
class QWizard;

class JournalResult : public JournalResultBase
{
    Q_OBJECT
public:
    JournalResult(QWidget *parent, LiveJournalClient *client);
    ~JournalResult();
    void setStatus(const QString &str);
signals:
    void search();
protected:
    void showEvent(QShowEvent*);
    QWizard	  *m_wizard;
    LiveJournalClient *m_client;
};

#endif

