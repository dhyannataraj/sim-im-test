/***************************************************************************
                          icqsearch.h  -  description
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

#ifndef _ICQSEARCH_H
#define _ICQSEARCH_H

#include "simapi.h"
#include "icqsearchbase.h"

class ICQSearchResult;
class ICQClient;
class QWizard;
class QLineEdit;

class ICQSearch : public ICQSearchBase, public EventReceiver
{
    Q_OBJECT
public:
    ICQSearch(ICQClient *client);
    ~ICQSearch();
signals:
    void goNext();
protected slots:
    void resultFinished();
    void changed();
    void search();
    void startSearch();
    void currentChanged(QWidget*);
    void textChanged(const QString&);
    void randomFind();
    void addContact();
    void sendMessage();
protected:
    virtual void *processEvent(Event *e);
    virtual void showEvent(QShowEvent *e);
    void fillGroup();
    string getString(QLineEdit*);
    void setFindText();
    bool m_bRandomSearch;
    unsigned m_randomUin;
    QString m_name;
    ICQSearchResult *m_result;
    QWizard	*m_wizard;
    ICQClient *m_client;
};

#endif

