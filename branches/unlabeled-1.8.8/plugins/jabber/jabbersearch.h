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
    JabberSearch(QWidget *receiver, JabberClient *client, const char *jid, const char *node, const QString &name, bool bRegister);
    bool canSearch();
    QString condition(bool &bXSearch);
    const char *id() { return m_jid.c_str(); }
    void addWidget(struct JabberAgentInfo *data);
    JabberClient	*m_client;
    string			m_jid;
    string			m_node;
    QString			m_title;
protected slots:
    void setSize();
protected:
    QString			i18(const char *text);
    QGridLayout		*lay;
    QString			m_name;
    QString			m_instruction;
    QString			m_label;
    QWidget			*m_receiver;
    string			m_key;
    bool			m_bDirty;
    bool			m_bXData;
    bool			m_bFirst;
    bool			m_bRegister;
    list<QWidget*>	m_required;
    vector<QWidget*>	m_widgets;
    vector<QWidget*>	m_labels;
	vector<QWidget*>	m_descs;
};

#endif

