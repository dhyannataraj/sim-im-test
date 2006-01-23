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

#include "jabberclient.h"
#include "simapi.h"
#include "jabberaddbase.h"
#include "stl.h"

class JabberClient;
class JabberBrowser;
class GroupRadioButton;

typedef struct ItemInfo
{
    std::string	jid;
    std::string	node;
    std::string	id;
} ItemInfo;

typedef struct AgentSearch
{
    std::string		jid;
    std::string		node;
    std::string		id_info;
    std::string		id_search;
    QString			condition;
    unsigned		fill;
    std::vector<std::string>	fields;
    std::string		type;
} AgentSearch;

class JabberAdd : public JabberAddBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    JabberAdd(JabberClient *client, QWidget *parent);
    ~JabberAdd();
signals:
    void setAdd(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
protected slots:
    void radioToggled(bool);
    void browserDestroyed();
    void browserClick();
    void search();
    void searchStop();
    void searchMail(const QString&);
    void searchName(const QString&, const QString&, const QString&);
    void createContact(const QString&, unsigned tmpFlags, SIM::Contact *&contact);
    void createContact(unsigned tmpFlags, SIM::Contact *&contact);
protected:
    void *processEvent(SIM::Event*);
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
    std::string		m_id_browse;
    std::string		m_id_disco;
    std::list<ItemInfo>		m_disco_items;
    std::list<AgentSearch>	m_agents;
    std::vector<std::string>m_fields;
    std::vector<QString>	m_labels;
    unsigned			m_nFields;
};

#endif

