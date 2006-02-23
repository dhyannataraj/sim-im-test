/***************************************************************************
                          jidsearch.h  -  description
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

#ifndef _JIDSEARCH_H
#define _JIDSEARCH_H

#include "simapi.h"
#include "jidsearchbase.h"

class JabberClient;
class JIDAdvSearch;

class JIDSearch : public JIDSearchBase, public EventReceiver
{
    Q_OBJECT
public:
    JIDSearch(QWidget *parent, JabberClient *client, const QString &jid, const QString &m_node, const char *type);
    QString m_jid;
    QString m_node;
signals:
    void setAdd(bool);
    void showClient(Client*);
    void showResult(QWidget*);
    void addResult(QWidget*);
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
protected slots:
    void browserClicked();
    void advancedClicked();
    void search();
    void searchStop();
    void createContact(const QString&, unsigned tmpFlags, Contact *&contact);
protected:
    void *processEvent(Event*);
    void showEvent(QShowEvent*);
    string		 m_search_id;
    string		 m_type;
    bool		 m_bInit;
    bool		 m_bAdv;
    JIDAdvSearch *m_adv;
    JabberClient *m_client;
};

#endif

