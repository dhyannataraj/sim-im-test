/***************************************************************************
                          userview.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _USERVIEW_H
#define _USERVIEW_H 1

#include "defs.h"
#include "cfg.h"

#include <qwidget.h>
#include <qlistview.h>
#ifdef WIN32
#include "qt3/qtooltip.h"
#define QToolTip MyQToolTip
#else
#include <qtooltip.h>
#endif
#include <qimage.h>

class ICQContactList;
class ICQUser;
class ICQGroup;
class UserView;
class QPopupMenu;
class KPopupMenu;
class ICQEvent;
class ICQMessage;
class QLineEdit;
class QDragObject;
class TransparentTop;
class TransparentBg;

class UserViewItemBase : public QListViewItem
{
public:
    UserViewItemBase(UserView *parent);
    UserViewItemBase(UserViewItemBase *parent);
    virtual int type() = 0;
    virtual QString key(int column, bool) const;
    void paint(QPainter *p, QString s, const QColorGroup &cg, bool bSeparator, bool bEnabled = true, int *w=NULL, int *width=NULL);
    virtual void paintFocus ( QPainter *, const QColorGroup & cg, const QRect & r );
};

class UserViewItem : public UserViewItemBase
{
public:
    UserViewItem(ICQUser *u, UserView *parent);
    UserViewItem(ICQUser *u, UserViewItemBase *parent);
    ~UserViewItem();
    int type();
    int width(const QFontMetrics &fm, const QListView *lv, int c) const;
    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
    void update(ICQUser *u, bool bFirst=false);
    unsigned m_uin;
    unsigned m_status;
    unsigned m_itemState;
    unsigned nBlink;
    bool m_bItalic, m_bUnderline, m_bStrikeOut;
    bool m_bInvisible, m_bBirthday, m_bMobile, m_bPhone, m_bPhoneBusy, m_bPager;
    bool m_bSecure;
    friend class UserView;
};

class GroupViewItem : public UserViewItemBase
{
public:
    GroupViewItem(ICQGroup *u, unsigned ord, UserView *parent);
    int type();
    void setOpen(bool);
    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
    void changeCounter(bool bInc);
    unsigned short Id() { return m_id; }
protected:
    unsigned m_nOnline;
    unsigned short m_id;
    friend class UserView;
};

class DivItem : public UserViewItemBase
{
public:
    DivItem(const QString &name, const QString &key, UserView *parent);
    int type();
    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
};

class UserView : public QListView, public QToolTip
{
    Q_OBJECT
public:
    UserView(QWidget *parent, bool bList=false, bool bFill=true, WFlags flags = 0);
    bool bFloaty;
    bool bList;
    bool hasChecked();
    void check(unsigned long uin);
    void fillChecked(ICQMessage*);
    void fill();
    bool bStaticBg;
public slots:
    void setShowOffline(bool);
    void setGroupMode(bool);
    void refresh();
    void pressedUp();
signals:
    void checked();
protected slots:
    void bgChanged();
    void processEvent(ICQEvent*);
    void doubleClick(QListViewItem*);
    void grpFunction(int);
    void editEscape();
    void editEnter();
    void messageRead(ICQMessage*);
    void messageReceived(ICQMessage*);
    void dragStart();
    void blink();
    void iconChanged();
    void itemClicked(QListViewItem*);
    void accelActivated(int);
    void clearGroupMenu();
protected:
    bool hasChecked(QListViewItem*);
    void fillChecked(QListViewItem*, ICQMessage*);
    void setGrpCheck(QListViewItem*);

    void blink(QListViewItem*);
    virtual void userChanged() {}

    virtual bool eventFilter(QObject*, QEvent*);
    virtual void styleChange(QStyle &old);
    virtual void paintEmptyArea(QPainter *p, const QRect &rc);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *e);

    virtual void contentsDragEnterEvent(QDragEnterEvent *e);
    virtual void contentsDragMoveEvent(QDragMoveEvent *);
    virtual void contentsDropEvent(QDropEvent *e);

    virtual void viewportContextMenuEvent(QContextMenuEvent *e);
    virtual QDragObject *dragObject();
    virtual void startDrag();

    bool isUserShow(ICQUser*);

    void dragEvent(QDropEvent *e, bool isDrop);
    void callUserFunction(unsigned long uin, const QString &url, bool bUrl);

    void updateUser(unsigned long uin, bool bFull);

    QListViewItem *mPressedItem;
    QLineEdit *edtGroup;
    QPoint mousePressPos;
    KPopupMenu *menuGroup;

    QImage  bgPict;
    QImage	bgPictScale;
    void drawImage(QPainter *p, int x, int y, int w, int h, int imgX, int imgY, int imgW, int imgH);

    unsigned m_counts[3];
    unsigned short grp_id;
    void incStateCount(unsigned state);
    void decStateCount(unsigned state);

    void setOpen(bool bOpen);

    void clear();

    void addUserItem(ICQUser*);

    UserViewItem  *findUserItem(unsigned long uin);
    GroupViewItem *findGroupItem(unsigned short grp_id);

    UserViewItem *findUserItem(const QPoint p);
    ICQUser *findUser(QPoint p);

    void setGroupExpand(unsigned short grp_id, bool bState);

    bool m_bGroupMode;
    bool m_bShowOffline;
    virtual void maybeTip ( const QPoint & );

    TransparentBg *transparent;

    friend class UserViewItemBase;
    friend class UserViewItem;
    friend class GroupViewItem;
};

typedef struct UserFload_Data
{
    unsigned long	Uin;
    unsigned short	Left;
    unsigned short	Top;
} UserFloat_Data;

class UserFloat : public UserView
{
    Q_OBJECT
public:
    UserFloat();
    ~UserFloat();
    PROP_ULONG(Uin)
    PROP_USHORT(Left)
    PROP_USHORT(Top)
    bool setUIN(unsigned long uin);
    void save(QFile &s);
    bool load(QFile &s, string &nextPart);
    virtual void setBackgroundPixmap(const QPixmap&);
protected:
    UserFloat_Data data;
    void userChanged();
    bool bMoveMode;
    QPoint mousePos;
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    TransparentTop *transparent;
};

#endif

