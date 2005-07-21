/***************************************************************************
                          ignorelist.h  -  description
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

#ifndef _IGNORELIST_H
#define _IGNORELIST_H

#include "simapi.h"
#include "ignorelistbase.h"

class QListViewItem;

class IgnoreList : public IgnoreListBase, public EventReceiver
{
    Q_OBJECT
public:
    IgnoreList(QWidget *parent);
protected slots:
    void deleteItem(QListViewItem*);
    void dragStart();
    void dragEnter(QMimeSource*);
    void drop(QMimeSource*);
protected:
    void *processEvent(Event*);
    void removeItem(QListViewItem*);
    void updateItem(QListViewItem*, Contact*);
    QListViewItem *findItem(Contact*);
};

#endif

