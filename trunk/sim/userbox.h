/***************************************************************************
                          userbox.h  -  description
                             -------------------
    begin                : Sat Mar 16 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _USERBOX_H
#define _USERBOX_H	1

#include "defs.h"

#include "cfg.h"

#include <qmainwindow.h>
#include <qtabbar.h>

#include <list>
#include <string>

class QVBoxLayout;
class QTabBar;
class QToolBar;
class QToolButton;
class QMenuData;
class QPopupMenu;
class QSplitter;
class QStatusBar;
class QFrame;

class CToolButton;
class UserBox;
class UserInfo;
class MsgView;
class MsgEdit;
class HistoryView;
class PictButton;
class ICQEvent;
class ICQMessage;
class ICQUser;
class TransparentTop;
class UserView;

class UserTab : public QTab
{
public:
    UserTab(MsgEdit *wnd, QTabBar *bar, int index=-1);
    QString alias;
    bool isBold;
    int id;
};

class UserTabBar : public QTabBar
{
    Q_OBJECT
public:
    UserTabBar(QWidget *parent);
signals:
    void showUserPopup(int id, QPoint p);
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void paintLabel(QPainter *p, const QRect &rc, QTab *t, bool bFocus) const;
};

class UserBox : public QMainWindow, public ConfigArray
{
    Q_OBJECT
public:
    UserBox(unsigned long GrpId=0);
    ~UserBox();
    void setShow();
    bool isShow();
    virtual void save(std::ostream &s);
    virtual bool load(std::istream &s, string &nextPart);
    ConfigULong GrpId;
    ConfigULong CurrentUser;
    ConfigShort mLeft;
    ConfigShort mTop;
    ConfigShort mWidth;
    ConfigShort mHeight;
    ConfigString ToolbarDock;
    ConfigShort	 ToolbarOffset;
    ConfigShort  ToolbarY;
    unsigned long currentUser();
    bool haveUser(unsigned long uin);
    QString containerName();
    unsigned count();
    UserView	*users;
public slots:
    void quit();
    void typeChanged(int);
    void destroyChild(int);
    void selectedUser(int);
    bool closeUser(unsigned long uin);
    void showUser(unsigned long uin, int function, unsigned long param=0);
    void showUserPopup(int id, QPoint p);
    void messageRead(ICQMessage*);
    void messageReceived(ICQMessage *msg);
    void setGroupButtons();
    void showUsers(bool, unsigned long);
protected slots:
    void showMessage(unsigned long uin, unsigned long id);
    void processEvent(ICQEvent*);
    void toggleHistory(bool);
    void toggleInfo(bool);
    void toIgnore();
    void hideHistory();
    void showGrpMenu();
    void saveInfo(ICQUser*);
    void moveUser(int);
    void iconChanged();
    void wmChanged();
protected:
    void showEvent(QShowEvent *e);
    list<MsgEdit*> wnds;
    QToolBar	*toolbar;
    QToolButton *btnQuit;
    QToolButton *btnHistory;
    QToolButton *btnInfo;
    PictButton  *btnType;
    PictButton  *btnUser;
    QToolButton	*btnIgnore;
    CToolButton	*btnGroup;
    QSplitter	*vSplitter;
    QSplitter   *splitter;
    MsgView     *msgView;
    QSplitter  *tabSplitter;
    QStatusBar *status;
    QPopupMenu *menuUser;
    QPopupMenu *menuType;
    QVBoxLayout *lay;
    QFrame		*frm;
    QFrame		*frmUser;
    QVBoxLayout *layUser;
    QPopupMenu	*menuGroup;
    void removeChilds();
    MsgEdit *getWnd(int id);
    MsgEdit *getChild(unsigned long uin, bool bCreate=true);
    virtual bool event(QEvent *e);
    void adjustUserMenu(bool rescan);
    void statusChanged(unsigned long uin);
    UserTabBar  *tabs;
    MsgEdit     *curWnd;
    UserInfo	*infoWnd;
    HistoryView *historyWnd;
    friend class UserTabBar;
    TransparentTop *transparent;
};

#endif


