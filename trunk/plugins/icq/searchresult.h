/***************************************************************************
                          searchresult.h  -  description
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

#ifndef _SEARCHRESULT_H
#define _SEARCHRESULT_H

#include "simapi.h"
#include "searchresultbase.h"

#include <qlistview.h>

class ICQClient;

class UserTblItem : public QListViewItem
{
public:
    UserTblItem(QListView *parent, ICQClient *client, struct ICQUserData *data);
    UserTblItem(QListView *parent, unsigned long uin, const QString &alias);
protected:
    void init(ICQClient *client, ICQUserData *data);
    virtual QString key(int column, bool) const;
    unsigned long mUin;
    unsigned mState;
    friend class UserTbl;
};

class ICQSearchResult : public ICQSearchResultBase, public EventReceiver
{
    Q_OBJECT
public:
    ICQSearchResult(QWidget *parent, ICQClient *client);
    ~ICQSearchResult();
    void clear();
    void setRequestId(unsigned short id);
    void setText(const QString &text);
signals:
    void finished();
    void startSearch();
protected slots:
    void dragStart();
    void doubleClicked(QListViewItem*);
protected:
    void showEvent(QShowEvent *e);
    virtual void *processEvent(Event*);
    void setStatus();
    Contact *createContact(unsigned tmpFlags);
    unsigned short m_id;
    unsigned m_nFound;
    ICQClient *m_client;
};

#endif

