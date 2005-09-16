/***************************************************************************
                          userview.h  -  description
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

#ifndef _USERVIEW_H
#define _USERVIEW_H

#include "simapi.h"
#include "userlist.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QDialog>

class CorePlugin;
class IntLineEdit;
class QTimer;
class TipLabel;

typedef struct BlinkCount
{
    unsigned long	id;
    unsigned		count;
} BlinkCount;

class UserView : public UserListBase, public QDialog
{
    Q_OBJECT
public:
    UserView();
    ~UserView();
    CorePlugin *m_plugin;
    IntLineEdit *edtGroup;
    IntLineEdit *edtContact;
    Q3ListViewItem *mTipItem;
    bool	 m_bUnreadBlink;
public slots:
    void editGroupEnter();
    void editContactEnter();
    void editEscape();
    void renameGroup();
    void renameContact();
    void deleteGroup(void*);
    void deleteContact(void*);
    void joinContacts(void*);
    void cancelJoinContacts(void*);
    void showTip();
    void hideTip();
    void tipDestroyed();
    void searchTipDestroyed();
    void blink();
    void unreadBlink();
    void doDrop();
    void doClick();
    void dragScroll();
protected:
    virtual void fill();
    virtual void drawItem(UserViewItemBase *base, QPainter *p, const QColorGroup &cg, int width, int margin);
    virtual int heightItem(UserViewItemBase *base);
    virtual unsigned getUnread(unsigned contact_id);
    virtual ProcessMenuParam *getMenu(Q3ListViewItem *item);
    void *processEvent(Event*);
    void paintEmptyArea(QPainter *p, const QRect &r);
    bool eventFilter(QObject *obj, QEvent *e);
    void setGroupMode(unsigned mode, bool bFirst=false);
    void keyPressEvent(QKeyEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    void focusOutEvent(QFocusEvent*);
    void sortAll();
    void sortAll(Q3ListViewItem*);
    void dragEvent(QDropEvent *e, bool isDrop);
    void search(list<Q3ListViewItem*> &items);
    void search(Q3ListViewItem*, list<Q3ListViewItem*> &items);
    void stopSearch();
    void showTip(Q3ListViewItem*);
    Q3DragObject *dragObject();
    virtual void deleteItem(Q3ListViewItem *item);
    list<BlinkCount> blinks;
    unsigned long m_dropContactId;
    Q3ListViewItem *m_dropItem;
    Q3ListViewItem *m_current;
    TipLabel *m_tip;
    TipLabel *m_searchTip;
    bool	 m_bBlink;
    QTimer   *tipTimer;
    QTimer	 *blinkTimer;
    QTimer	 *unreadTimer;
    QString	 m_search;
    bool	 m_bRemoveHistory;
    Q3ListViewItem *m_searchItem;
};

#endif

