/***************************************************************************
                          usertbl.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _USERTBL_H
#define _USERTBL_H 1

#include "defs.h"

#include <list>

using namespace std;
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif

#include <qlistview.h>

class QPopupMenu;
class ICQEvent;
class ICQUser;
class ContactList;
class UserTblItem;
class QDragObject;
class TransparentBg;

class UserTbl : public QListView
{
    Q_OBJECT
public:
    UserTbl(QWidget*, const char *name=NULL);
    bool isEmpty();
    bool sender;
    void fillList(ContactList &list);
    void fillList(list<unsigned long> &uins);
signals:
    void changed();
public slots:
    void erase();
    void action(int id);
    void addSearch(ICQEvent*);
    void addUser(unsigned long uin, const char *alias);
    void addUin(unsigned long uin);
    void deleteUin(unsigned long uin);
protected slots:
    void processEvent(ICQEvent*);
    void dragStart();
protected:
    QPopupMenu *menuTable;
#if QT_VERSION < 300
    QPoint mousePressPos;
#endif
    UserTblItem *findItem(unsigned long uin);
    void fillFromUser(UserTblItem*, ICQUser*);
    void fillFromSearch(UserTblItem*, ICQEvent*);
    void addGroup(unsigned short grpId);
    unsigned short actionGroup();
    QListViewItem *actionItem;
    virtual void resizeEvent(QResizeEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *e);
    virtual void contentsDragEnterEvent(QDragEnterEvent*);
    virtual void contentsDropEvent(QDropEvent *e);
    virtual void viewportContextMenuEvent(QContextMenuEvent *e);
    virtual void startDrag();
    virtual void paintEmptyArea(QPainter *p, const QRect &r);
    virtual QDragObject *dragObject();
    TransparentBg *transparent;
    friend class UserTblItem;
};

#endif
