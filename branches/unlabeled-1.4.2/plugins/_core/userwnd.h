/***************************************************************************
                          userwnd.h  -  description
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

#ifndef _USERWND_H
#define _USERWND_H

#include "simapi.h"
#include <qsplitter.h>

class MsgEdit;
class MsgView;
class ListView;
class CorePlugin;
class QToolBar;
class UserList;

typedef struct UserWndData
{
    Data	EditHeight;
    Data	editBar[7];
    Data	MessageType;
} UserWndData;

class UserWnd : public QSplitter
{
    Q_OBJECT
public:
    UserWnd(unsigned id, const char *cfg, bool bReceived);
    ~UserWnd();
    string getConfig();
    unsigned id() { return m_id; }
    QString getName();
    QString getLongName();
    const char *getIcon();
    void setMessage(Message*&);
    void setStatus(const QString&);
    void showListView(bool bShow);
    QString status() { return m_status; }
    UserList	*m_list;
    void markAsRead();
    bool isClosed() { return m_bClosed; }
    bool m_bTyping;
signals:
    void closed(UserWnd*);
    void statusChanged(UserWnd*);
    void multiplyChanged();
protected slots:
    void modeChanged();
    void editHeightChanged(int);
    void toolbarChanged(QToolBar*);
    void selectChanged();
protected:
    PROP_ULONG(EditHeight);
    void closeEvent(QCloseEvent*);
    MsgView		*m_view;
    MsgEdit		*m_edit;
    QSplitter	*m_hSplitter;
    QSplitter	*m_splitter;
    QString		m_status;
    bool		m_bResize;
    bool		m_bClosed;
    bool		m_bBarChanged;
    unsigned	m_id;
    UserWndData	data;
    friend class MsgEdit;
};

#endif

