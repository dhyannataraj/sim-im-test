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

#include "listview.h"

class UserListBase;
class QTimer;

const unsigned DIV_ITEM = 0;
const unsigned GRP_ITEM = 1;
const unsigned USR_ITEM = 2;

const unsigned DIV_ONLINE  = 0;
const unsigned DIV_OFFLINE = 1;

const unsigned CONTACT_TEXT		= 1;
const unsigned CONTACT_ICONS	= 2;
const unsigned CONTACT_ACTIVE	= 3;
const unsigned CONTACT_STATUS	= 4;

class UserViewItemBase : public QListViewItem
{
public:
    UserViewItemBase(UserListBase *view);
    UserViewItemBase(UserViewItemBase *parent);
    virtual unsigned type() = 0;
    virtual void setup();
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
    GroupItem(UserListBase *view, SIM::Group *grp, bool bOffline);
    GroupItem(UserViewItemBase *view, SIM::Group *grp, bool bOffline);
    unsigned type() { return GRP_ITEM; }
    unsigned long id() { return m_id; }
    void update(SIM::Group *grp, bool bInit=false);
    unsigned m_nContacts;
    unsigned m_nContactsOnline;
    unsigned m_unread;
protected:
    virtual void setOpen(bool bOpen);
    void init(SIM::Group *grp);
    unsigned long m_id;
    bool m_bOffline;
};

class ContactItem : public UserViewItemBase
{
public:
    ContactItem(UserViewItemBase *view, SIM::Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread);
    unsigned type() { return USR_ITEM; }
    unsigned long id() { return m_id; }
    unsigned style() { return m_style; }
    unsigned status() { return m_status; }
    bool update(SIM::Contact *grp, unsigned status, unsigned style, const QString &icons, unsigned unread);
    bool m_bOnline;
    bool m_bBlink;
    unsigned m_unread;
protected:
    virtual QString key(int column, bool ascending) const;
    void init(SIM::Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread);
    unsigned long m_id;
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
    unsigned m_bShowEmpty;
    virtual bool processEvent(SIM::Event*);
    virtual void drawItem(UserViewItemBase *base, QPainter *p, const QColorGroup &cg, int width, int margin);
    virtual int heightItem(UserViewItemBase *base);
    unsigned getUserStatus(SIM::Contact *contact, unsigned &style, QString &icons);
    virtual unsigned getUnread(unsigned contact_id);
    GroupItem *findGroupItem(unsigned id, QListViewItem *p = NULL);
    ContactItem *findContactItem(unsigned id, QListViewItem *p = NULL);
    void addSortItem(QListViewItem *item);
    void addUpdatedItem(QListViewItem *item);
    void addGroupForUpdate(unsigned long id);
    void addContactForUpdate(unsigned long id);
    virtual void deleteItem(QListViewItem *item);
    std::list<QListViewItem*> sortItems;
    std::list<QListViewItem*> updatedItems;
    std::list<unsigned long>	updGroups;
    std::list<unsigned long>	updContacts;
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
    std::list<unsigned> selected;
signals:
    void selectChanged();
    void finished();
protected:
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void drawItem(UserViewItemBase *base, QPainter *p, const QColorGroup &cg, int width, int margin);
    bool isSelected(unsigned id);
    bool isGroupSelected(unsigned id);
    int drawIndicator(QPainter *p, int x, QListViewItem *item, bool bState, const QColorGroup &cg);
};

#endif

