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
#include "jabberclient.h"
#include "stl.h"

class JabberClient;
class JabberBrowser;
class GroupRadioButton;

typedef struct ItemInfo
{
    string	jid;
    string	node;
    string	id;
} ItemInfo;

typedef struct AgentSearch
{
    string			jid;
    string			node;
    string			id_info;
    string			id_search;
    QString			condition;
    unsigned		fill;
    vector<string>	fields;
    string			type;
} AgentSearch;

class JabberAdd : public JabberAddBase, public EventReceiver
{
    Q_OBJECT
public:
    JabberAdd(JabberClient *client, QWidget *parent);
    ~JabberAdd();
signals:
    void setAdd(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
    void showError(const QString&);
    void setColumns(const QStringList&, int);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
protected slots:
    void radioToggled(bool);
    void browserDestroyed();
    void browserClick();
    void add(unsigned);
    void search();
	void searchStop();
protected:
    void *processEvent(Event*);
    void setBrowser(bool bBrowser);
    void showEvent(QShowEvent*);
    void startSearch();
    void checkDone();
    void addAttr(const char *name, const QString &label);
    void addAttrs();
    void addSearch(const char *jid, const char *node, const char *features, const char *type);
    JabberClient	*m_client;
    JabberBrowser	*m_browser;
    bool			m_bBrowser;
    QString			m_first;
    QString			m_last;
    QString			m_nick;
    QString			m_mail;
    string			m_id_browse;
    string			m_id_disco;
    list<ItemInfo>	m_disco_items;
    list<AgentSearch>	m_agents;
    vector<string>		m_fields;
    vector<QString>		m_labels;
    unsigned			m_nFields;
};

#endif

