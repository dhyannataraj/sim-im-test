/***************************************************************************
                          userbox.h  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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
class CToolBar;
class QToolButton;
class QMenuData;
class QPopupMenu;
class QSplitter;
class QStatusBar;
class QFrame;
class QProgressBar;

class CToolButton;
class UserBox;
class UserInfo;
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
    virtual void layoutTabs();
signals:
    void showUserPopup(int id, QPoint p);
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void paintLabel(QPainter *p, const QRect &rc, QTab *t, bool bFocus) const;
};

typedef struct UserBox_Data
{
    unsigned long	GrpId;
    unsigned long	CurrentUser;
    short			Left;
    short			Top;
    short			Width;
    short			Height;
    char*			ToolbarDock;
    short			ToolbarOffset;
    short			ToolbarY;
    bool			History;
    bool			UserInfo;
} UserBox_Data;

class UserBox : public QMainWindow
{
    Q_OBJECT
public:
    UserBox(unsigned long GrpId=0);
    ~UserBox();
    PROP_ULONG(GrpId)
    PROP_ULONG(CurrentUser)
    PROP_SHORT(Left)
    PROP_SHORT(Top)
    PROP_SHORT(Width)
    PROP_SHORT(Height)
    PROP_STR(ToolbarDock)
    PROP_SHORT(ToolbarOffset)
    PROP_SHORT(ToolbarY)
    PROP_BOOL(History)
    PROP_BOOL(UserInfo)
    void setShow();
    bool isShow();
    void save(QFile &s);
    bool load(QFile &s, string &nextPart);
    unsigned long currentUser();
    bool haveUser(unsigned long uin);
    QString containerName();
    unsigned count();
    UserView	*users;
    virtual void setBackgroundPixmap(const QPixmap&);
    ICQMessage	*currentMessage();
    void hideToolbar();
signals:
    void messageReceived(ICQMessage*);
public slots:
    void quit();
    void typeChanged(int);
    void destroyChild(int);
    void selectedUser(int);
    bool closeUser(unsigned long uin);
    void showUser(unsigned long uin, int function, unsigned long param=0);
    void showUserPopup(int id, QPoint p);
    void messageRead(ICQMessage*);
    void setGroupButtons();
    void showUsers(bool, unsigned long);
    void showProgress(int n);
protected slots:
    void toolBarChanged(QToolBar *toolbar);
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
    void showEncodingPopup();
    void setUserEncoding(int);
    void slotMessageReceived(ICQMessage*);
    void modeChanged(bool);
    void accelActivated(int id);
    void setOnTop();
protected:
    UserBox_Data data;
    void getToolbarPosition();
    void showEvent(QShowEvent *e);
    void resizeEvent(QResizeEvent *e);
    void moveEvent(QMoveEvent *e);
    unsigned long msgShowId;
    list<MsgEdit*> wnds;
    CToolBar	*toolbar;
    QSplitter	*vSplitter;
    QSplitter  *tabSplitter;
    QStatusBar *status;
    QPopupMenu *menuUser;
    QPopupMenu *menuType;
    QPopupMenu *menuEncoding;
    QVBoxLayout *lay;
    QFrame		*frm;
    QFrame		*frmUser;
    QVBoxLayout *layUser;
    QPopupMenu	*menuGroup;
    QProgressBar *progress;
    void removeChilds();
    MsgEdit *getWnd(int id);
    MsgEdit *getChild(unsigned long uin, bool bCreate=true);
    virtual bool event(QEvent *e);
    void adjustUserMenu(bool rescan);
    void statusChanged(unsigned long uin);
    void adjustPos();
    void adjustToolbar();
    UserTabBar  *tabs;
    MsgEdit     *curWnd;
    UserInfo	*infoWnd;
    HistoryView *historyWnd;
    friend class UserTabBar;
    TransparentTop *transparent;
    int infoPage;
};

#endif


