/***************************************************************************
                          jabberbrowser.h  -  description
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

#ifndef _JABBERBROWSER_H
#define _JABBERBROWSER_H

#include "simapi.h"
#include "jabberclient.h"
#include "stl.h"

#include <qmainwindow.h>
#include <qwizard.h>

class ListView;
class QListViewItem;
class QStatusBar;
class CToolBar;
class DiscoInfo;
class JabberWizard;
class AddResult;

class JabberWizard : public QWizard, public EventReceiver
{
    Q_OBJECT
public:
    JabberWizard(QWidget *parent, const char *title, const char *icon, JabberClient *client, const char *jid, const char *node, const char *type);
    JabberSearch *m_search;
    AddResult	 *m_result;
    void initTitle();
protected slots:
    void setNext();
    void search();
    void textChanged(const QString&);
    void slotSelected(const QString&);
protected:
    void *processEvent(Event *e);
    string m_type;
    string m_id;
};

class JabberBrowser : public QMainWindow, public EventReceiver
{
    Q_OBJECT
public:
    JabberBrowser(JabberClient *client);
    ~JabberBrowser();
    void goUrl(const QString &url, const QString &node);
    void save();
    DiscoInfo *m_info;
protected slots:
    void clickItem(QListViewItem*);
    void dragStart();
    void showSearch();
    void showReg();
    void showConfig();
protected:
    void *processEvent(Event*);
    void setTitle();
    void setNavigation();
    void stop(const QString &err);
    void go(const QString &url, const QString &node);
    void addHistory(const QString &str);
    bool haveFeature(const char*);
    string		  m_id1;
    string		  m_id2;
    JabberClient *m_client;
    ListView	 *m_list;
    QStatusBar	 *m_status;
    CToolBar	 *m_bar;
    vector<string>	m_history;
    vector<string>	m_nodes;
    QString		 m_historyStr;
    int			 m_historyPos;
    QString		 m_category;
    QString		 m_type;
    QString		 m_name;
    QString		 m_features;
    JabberWizard	*m_search;
    JabberWizard	*m_reg;
    JabberWizard	*m_config;
    string		 m_search_id;
    string		 m_reg_id;
    string		 m_config_id;
    friend class DiscoInfo;
};

#endif

