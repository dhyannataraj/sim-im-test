/***************************************************************************
                          maininfo.h  -  description
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

#ifndef _MAININFO_H
#define _MAININFO_H

#include "simapi.h"
#include "maininfobase.h"

class QListViewItem;

class MainInfo : public MainInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    MainInfo(QWidget *parent, Contact *contact);
protected slots:
    void apply();
    void mailSelectionChanged();
    void phoneSelectionChanged();
    void deleteMail(QListViewItem *item);
    void deletePhone(QListViewItem *item);
    void editMail(QListViewItem *item);
    void editPhone(QListViewItem *item);
    void addMail();
    void editMail();
    void deleteMail();
    void addPhone();
    void editPhone();
    void deletePhone();
protected:
    void fill();
    void *processEvent(Event*);
    void fillPhoneItem(QListViewItem *item, const QString &number, const QString &type, unsigned icon, const QString &proto);
    void fillCurrentCombo();
    void fillEncoding();
    void getEncoding();
    bool m_bInit;
    Contact *m_contact;
};

#endif

