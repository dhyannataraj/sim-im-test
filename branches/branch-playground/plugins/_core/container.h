/***************************************************************************
                          container.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CONTAINER_H
#define _CONTAINER_H

#include "cfg.h"
#include "event.h"
#include "message.h"

#include <QMainWindow>
#include <QDockWidget>
#include <qstatusbar.h>
#include <qtabbar.h>
#include <qpixmap.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QStackedWidget>
#include <Q3CString>
#include <QEvent>
#include <QMoveEvent>
#include <QMouseEvent>
#include <list>

const unsigned NEW_CONTAINER	= (unsigned)(-1);
const unsigned GRP_CONTAINER	= 0x80000000;

class UserWnd;
class UserTabBar;
class QSplitter;
class CToolBar;
class QWidgetStack;
class CorePlugin;
class Container;
class Q3Accel;

struct ContainerData
{
    SIM::Data	Id;
    SIM::Data	Windows;
    SIM::Data	ActiveWindow;
    SIM::Data	geometry[5];
    SIM::Data	barState[7];
    SIM::Data	StatusSize;
    SIM::Data	WndConfig;
};

class ContainerStatus : public QStatusBar
{
    Q_OBJECT
public:
    ContainerStatus(QWidget *parent);
signals:
    void sizeChanged(int);
protected:
    void resizeEvent(QResizeEvent*);
};

class UserWnd;
class UserTab
{
public:
    UserTab(UserWnd *wnd, bool bBold, int tabid);
    UserWnd	*wnd() { return m_wnd; }
    bool setBold(bool bState);
    bool isBold() { return m_bBold; }
	int getID() { return m_id;}
protected:
	int m_id;
    UserWnd	*m_wnd;
    bool	m_bBold;
    friend class UserTabBar;
};

class UserTabBar : public QTabBar
{
    Q_OBJECT
public:
    UserTabBar(QWidget *parent);
    virtual ~UserTabBar();
    void raiseTab(unsigned id);
    UserWnd *wnd(unsigned id);
    UserWnd *currentWnd();
    std::list<UserWnd*> windows();
    void removeTab(unsigned id);
    void changeTab(unsigned id);
    void setBold(unsigned id, bool bState);
    void setCurrent(unsigned i);
	int addTab(UserWnd* wnd, QString const& text);
    unsigned current();
    bool isBold(UserWnd *wnd);
public slots:
    void slotRepaint();
protected:
    virtual void layoutTabs();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
	std::list<UserTab*> m_tabs;
//    virtual void paintLabel(QPainter *p, const QRect &rc, QTab *t, bool bFocus) const;
};

class Container : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
	static const int WndType = QVariant::UserType + 1;

    Container(unsigned id, const char *cfg = NULL);
    ~Container();
    QString name();
    UserWnd *wnd(unsigned id);
    UserWnd *wnd();
    std::list<UserWnd*> windows();
    Q3CString getState();
    bool isReceived() { return m_bReceived; }
    void setReceived(bool bReceived) { m_bReceived = bReceived; }
    void setNoSwitch(bool bState);
    void setMessageType(unsigned id);
    void contactChanged(SIM::Contact *contact);
    PROP_ULONG(Id);
    PROP_STR(Windows);
    PROP_ULONG(ActiveWindow);
    PROP_ULONG(StatusSize);
    PROP_STRLIST(WndConfig);
    bool m_bNoRead;
    void init();
public slots:
    void addUserWnd(UserWnd*, bool bRaise);
    void removeUserWnd(UserWnd*);
    void raiseUserWnd(UserWnd*);
    void contactSelected(int);
    void toolbarChanged(QToolBar*);
    void statusChanged(int);
    void accelActivated(int);
    void statusChanged(UserWnd*);
    void modeChanged();
    void wndClosed();
    void flash();
    void setReadMode();
protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void moveEvent(QMoveEvent*);
    virtual bool event(QEvent*);
    virtual bool processEvent(SIM::Event*);
    void showBar();
    void setupAccel();
    ContainerData	data;
    bool			m_bInit;
    bool			m_bInSize;
    bool			m_bStatusSize;
    bool			m_bBarChanged;
    bool			m_bReceived;
    bool			m_bNoSwitch;
    CToolBar		*m_bar;
    QDockWidget		m_avatar_window;
    QLabel			m_avatar_label;
    QSplitter		*m_tabSplitter;
    UserTabBar		*m_tabBar;
    ContainerStatus	*m_status;
    QStackedWidget	*m_wnds;
    Q3Accel			*m_accel;
    std::list<UserWnd*> m_childs;
};

#endif

