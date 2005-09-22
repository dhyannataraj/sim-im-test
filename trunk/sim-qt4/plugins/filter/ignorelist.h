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
#include <QDialog>

class Q3ListViewItem;

class IgnoreList : public QDialog, public Ui::IgnoreListBase, public EventReceiver
{
    Q_OBJECT
public:
    IgnoreList(QWidget *parent);
protected slots:
    void deleteItem(Q3ListViewItem*);
    void dragStart();
    void dragEnter(QMimeSource*);
    void drop(QMimeSource*);
protected:
    void *processEvent(Event*);
    void removeItem(Q3ListViewItem*);
    void updateItem(Q3ListViewItem*, Contact*);
    Q3ListViewItem *findItem(Contact*);
};

#endif

