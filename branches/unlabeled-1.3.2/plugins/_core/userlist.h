/***************************************************************************
                          userlist.h  -  description
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

#ifndef _USERLIST_H
#define _USERLIST_H

#include "simapi.h"
#include "listview.h"

#include <list>
using namespace std;

class UserListBase;
class QTimer;

const unsigned DIV_ITEM = 0;
const unsigned GRP_ITEM = 1;
const unsigned USR_ITEM = 2;

const unsigned DIV_ONLINE  = 0;
const unsigned DIV_OFFLINE = 1;

const unsigned CONTACT_TEXT		= 1;
const unsigned CONTACT_ICONS	= 2;

class UserViewItemBase : public QListViewItem
{
public:
    UserViewItemBase(UserListBase *view);
    UserViewItemBase(UserViewItemBase *parent);
    virtual unsigned type() = 0;
    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
    virtual void paintFocus(QPainter*, const QColorGroup&, const QRect & );
    int drawText(QPainter *p, int x, int width, const QString &text);
    void drawSeparator(QPainter *p, int x, int width, const QColorGroup &cg);
};

class DivItem : public UserViewItemBase
{
public:
    DivItem(UserListBase *view, unsigned type);
    unsigned type() { return DIV_ITEM; }
    unsigned state() { return m_type; }
protected:
    unsigned m_type;
    friend class UserListBase;
};

class GroupItem : public UserViewItemBase
{
public:
    GroupItem(UserListBase *view, Group *grp, bool bOffline);
    GroupItem(UserViewItemBase *view, Group *grp, bool bOffline);
    unsigned type() { return GRP_ITEM; }
    unsigned id() { return m_id; }
    void update(Group *grp, bool bInit=false);
    unsigned m_nContacts;
    unsigned m_nContactsOnline;
    unsigned m_unread;
protected:
    virtual void setOpen(bool bOpen);
    void init(Group *grp);
    unsigned m_id;
    bool m_bOffline;
};

class ContactItem : public UserViewItemBase
{
public:
    ContactItem(UserViewItemBase *view, Contact *contact, unsigned status, unsigned style, const char *icons, unsigned unread);
    unsigned type() { return USR_ITEM; }
    unsigned id() { return m_id; }
    unsigned style() { return m_style; }
    unsigned status() { return m_status; }
    bool update(Contact *grp, unsigned status, unsigned style, const char *icons, unsigned unread);
    bool m_bOnline;
    bool m_bBlink;
    unsigned m_unread;
protected:
    virtual QString key(int column, bool ascending) const;
    void init(Contact *contact, unsigned status, unsigned style, const char *icons, unsigned unread);
    unsigned m_id;
    unsigned m_style;
    unsigned m_status;
};

class UserListBase : public ListView
{
    Q_OBJECT
public:
    UserListBase(QWidget *parent);
    ~UserListBase();
    virtual void fill();
protected slots:
    void drawUpdates();
protected:
    unsigned m_groupMode;
    unsigned m_bShowOnline;
    void *processEvent(Event*);
    virtual void drawItem(UserViewItemBase *base, QPainter *p, const QColorGroup &cg, int width, int margin);
    unsigned getUserStatus(Contact *contact, unsigned &style, string &icons);
    virtual unsigned getUnread(unsigned contact_id);
    GroupItem *findGroupItem(unsigned id, QListViewItem *p = NULL);
    ContactItem *findContactItem(unsigned id, QListViewItem *p = NULL);
    void addSortItem(QListViewItem *item);
    void addGroupForUpdate(unsigned long id);
    void addContactForUpdate(unsigned long id);
    virtual void deleteItem(QListViewItem *item);
    list<QListViewItem*> sortItems;
    list<unsigned long>	updGroups;
    list<unsigned long>	updContacts;
    bool m_bDirty;
    bool m_bInit;
    QTimer *updTimer;
    friend class UserViewItemBase;
};

class UserList : public UserListBase
{
    Q_OBJECT
public:
    UserList(QWidget *parent);
    ~UserList();
    list<unsigned> selected;
signals:
    void selectChanged();
protected:
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void drawItem(UserViewItemBase *base, QPainter *p, const QColorGroup &cg, int width, int margin);
    bool isSelected(unsigned id);
    bool isGroupSelected(unsigned id);
    int drawIndicator(QPainter *p, int x, QListViewItem *item, bool bState, const QColorGroup &cg);
};

#endif

