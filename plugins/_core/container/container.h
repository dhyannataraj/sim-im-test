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

#include "contacts/contact.h"
#include "containercontroller.h"
#include "icontainer.h"
#include "iuserwnd.h"
#include "avatarbar.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QStatusBar>
#include <QTabBar>
#include <QPixmap>
#include <QLabel>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QByteArray>
#include <QEvent>
#include <QMoveEvent>
#include <QMouseEvent>
#include <QTabBar>
#include <QVBoxLayout>
#include <QList>
#include <QShortcut>
#include <QSharedPointer>
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

//struct ContainerData
//{
//    SIM::Data	Id;
//    SIM::Data	Windows;
//    SIM::Data	ActiveWindow;
//    SIM::Data	geometry[5];
//    SIM::Data	barState[7];
//    SIM::Data	StatusSize;
//    SIM::Data	WndConfig;
//};


class UserTabBar : public QTabBar
{
    Q_OBJECT
public:
    UserTabBar(QWidget *parent);
    void raiseTab(unsigned id);
    UserWnd *wnd(unsigned id);
    int tab(unsigned contactid);
    UserWnd *currentWnd();
    std::list<UserWnd*> windows();
    void removeTab(unsigned id);
    void changeTab(unsigned id);
    void setHighlighted(unsigned id, bool bState);
    void setCurrent(unsigned i);
    unsigned current();
    bool isHighlighted(int  /*UserWnd *wnd*/);

public slots:
    void slotRepaint();

protected:
    virtual void layoutTabs();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    UserWnd* wndForTab(int tab);
};

class ContainerController;
class Container : public QMainWindow, public IContainer
{
    Q_OBJECT
public:
	static const int WndType = QVariant::UserType + 1;

    Container(unsigned id);
    virtual ~Container();
    virtual QString name();
    QByteArray getState();
    bool isReceived() { return m_bReceived; }
    void setReceived(bool bReceived) { m_bReceived = bReceived; }
    void setNoSwitch(bool bState);
    void setMessageType(unsigned id);
    void contactChanged(const SIM::ContactPtr& contact);
    int id() const;

    virtual void raiseUserWnd(int wndId);

    virtual void setController(ContainerController* controller);

    bool m_bNoRead;

signals:
    void closed();

public slots:
    void addUserWnd(IUserWnd*);
    void removeUserWnd(int wndId);

    void contactSelected(int contactId);
    void toolbarChanged(QToolBar*);
    void statusChanged(int);
    void accelActivated();
    void statusChanged(UserWnd*);
    void modeChanged();
    void wndClosed();
    void flash();
    void setReadMode();

protected slots:
    void messageSendRequested(const SIM::MessagePtr& message);

protected:
    void init();
    virtual void resizeEvent(QResizeEvent*);
    virtual void moveEvent(QMoveEvent*);
    virtual bool event(QEvent*);
    virtual void closeEvent(QCloseEvent* e);
    void showBar();
    void setupAccel();
    QShortcut* makeShortcut(unsigned int key, unsigned int id);
    void loadState();
    IUserWnd *wnd(int id);

    bool                m_bInit;
    bool                m_bInSize;
    bool                m_bStatusSize;
    bool                m_bBarChanged;
    bool                m_bReceived;
    bool                m_bNoSwitch;
    //ToolBar            *m_bar;
    QDockWidget*         m_avatar_window;
    AvatarBar* m_avatarBar;
    QSplitter* m_tabSplitter;
    QTabBar* m_tabBar;
    QStatusBar* m_status;
    QStackedWidget* m_wnds;
    QList<QShortcut*> m_shortcuts;
    QList<IUserWnd*> m_children;
    QFrame *m_frame;
    QVBoxLayout* m_layout;
    int m_id;
    ContainerController* m_controller;
};

typedef QSharedPointer<Container> ContainerPtr;

#endif

