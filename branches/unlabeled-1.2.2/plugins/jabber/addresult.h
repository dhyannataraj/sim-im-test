/***************************************************************************
                          addresult.h  -  description
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

#ifndef _ADDRESULT_H
#define _ADDRESULT_H

#include "simapi.h"
#include "addresultbase.h"

class JabberClient;
class QListViewItem;

class AddResult : public AddResultBase, public EventReceiver
{
    Q_OBJECT
public:
    AddResult(JabberClient *client);
    ~AddResult();
    void showSearch(bool);
    void setSearch(JabberClient *client, const char *search_id);
	void setText(const QString &text);
signals:
    void finished();
    void search();
protected slots:
    void dragStart();
    void doubleClicked(QListViewItem*);
protected:
    unsigned m_nFound;
    QString m_id;
    QString m_host;
    string m_searchId;
    unsigned EventSearch;
    unsigned EventSearchDone;
    QString foundStatus();
    virtual void *processEvent(Event *e);
    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);
    Contact *createContact(unsigned tmpFlags, struct JabberUserData **data = NULL);
    JabberClient *m_client;
};

#endif

