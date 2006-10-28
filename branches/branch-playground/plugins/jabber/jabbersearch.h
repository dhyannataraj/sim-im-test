/***************************************************************************
                          jabbersearch.h  -  description
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

#ifndef _JABBERSEARCH_H
#define _JABBERSEARCH_H

#include "simapi.h"
#include "stl.h"
#include "qchildwidget.h"

#include <qpushbutton.h>

class JabberClient;

class QGridLayout;

class HelpButton : public QPushButton
{
    Q_OBJECT
public:
    HelpButton(const QString &help, QWidget *parent);
protected slots:
    void click();
protected:
    QString m_help;
};

class JabberSearch : public QChildWidget
{
    Q_OBJECT
public:
    JabberSearch(QWidget *parent = NULL, const char *name = NULL);
    void init(QWidget *receiver, JabberClient *client, const char *jid, const char *node, const QString &name, bool bRegister);
    bool canSearch();
    QString condition(QWidget *w);
    const QString &id() { return m_jid; }
    void addWidget(struct JabberAgentInfo *data);
    JabberClient	*m_client;
    QString		m_jid;
    QString		m_node;
    QString			m_title;
protected slots:
    void setSize();
protected:
    virtual void		createLayout();
    QString				i18(const char *text);
    QString				m_name;
    QString				m_instruction;
    QString				m_label;
    QWidget				*m_receiver;
    QString			        m_key;
    bool				m_bDirty;
    bool				m_bXData;
    bool				m_bFirst;
    bool				m_bRegister;
    std::list<QWidget*>		m_required;
    std::vector<QWidget*>	m_widgets;
    std::vector<QWidget*>	m_labels;
    std::vector<QWidget*>	m_descs;
};

class JIDAdvSearch;

class JIDJabberSearch : public JabberSearch
{
    Q_OBJECT
public:
    JIDJabberSearch(QWidget *parent = NULL, const char *name = NULL);
    void setAdvanced(JIDAdvSearch *adv);
protected:
    void createLayout();
    JIDAdvSearch *m_adv;
};

#endif

