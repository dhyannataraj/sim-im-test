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
#include "stl.h"
#include "jabberclient.h"

#include <qmainwindow.h>

class ListView;
class QListViewItem;
class QStatusBar;
class CToolBar;
class DiscoInfo;

class JabberBrowser : public QMainWindow, public EventReceiver
{
    Q_OBJECT
public:
    JabberBrowser(JabberClient *client);
    ~JabberBrowser();
    void goUrl(const QString &url);
    void save();
    DiscoInfo *m_info;
protected slots:
    void clickItem(QListViewItem*);
    void dragStart();
protected:
    void *processEvent(Event*);
    void setTitle();
    void setNavigation();
    void stop(const QString &err);
    void go(const QString &url);
    void addHistory(const QString &str);
    string		  m_id1;
    string		  m_id2;
    JabberClient *m_client;
    ListView	 *m_list;
    QStatusBar	 *m_status;
    CToolBar	 *m_bar;
    vector<string>	m_history;
    QString		 m_historyStr;
    int			 m_historyPos;
    QString		 m_category;
    QString		 m_type;
    QString		 m_name;
    QString		 m_features;
    friend class DiscoInfo;
};

#endif

