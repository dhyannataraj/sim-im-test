/***************************************************************************
                          jabberadd.h  -  description
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

#ifndef _JABBERADD_H
#define _JABBERADD_H

#include "simapi.h"
#include "jabberaddbase.h"

#include <list>
using namespace std;

class JabberClient;
class JabberSearch;
class QWizard;
class QValidator;
class AddResult;

class JabberAdd : public JabberAddBase, public EventReceiver
{
    Q_OBJECT
public:
    JabberAdd(JabberClient *client);
    ~JabberAdd();
signals:
    void goNext();
protected slots:
    void serviceChanged(const QString&);
    void currentChanged(QWidget*);
    void textChanged(const QString&);
    void search();
    void startSearch();
    void addResultFinished();
protected:
    virtual void *processEvent(Event*);
    void showEvent(QShowEvent *e);
    list<JabberSearch*> m_search;
    JabberClient *findClient(const char *host);
    AddResult	 *m_result;
    QWizard		 *m_wizard;
    QValidator	 *m_idValidator;
    JabberClient *m_client;
};

#endif

