/***************************************************************************
                          listsec.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _LISTSEC_H
#define _LISTSEC_H

#include "defs.h"
#include "listsecbase.h"
#include "client.h"

class ListSecurity : public ListSecurityBase
{
    Q_OBJECT
public:
    ListSecurity(QWidget*);
public slots:
    void apply(ICQUser*);
protected slots:
    void processEvent(ICQEvent*);
    void tblChanged();
protected:
    void init();
    virtual bool checkUser(ICQUser *u) = 0;
    virtual void changeUser(ICQUser *u, bool bSet) = 0;

    list<unsigned long> oldValue;
};

class VisibleListSetup : public ListSecurity
{
    Q_OBJECT
public:
    VisibleListSetup(QWidget*);
protected:
    virtual bool checkUser(ICQUser *u);
    virtual void changeUser(ICQUser *u, bool bSet);
};

class InvisibleListSetup : public ListSecurity
{
    Q_OBJECT
public:
    InvisibleListSetup(QWidget*);
protected:
    virtual bool checkUser(ICQUser *u);
    virtual void changeUser(ICQUser *u, bool bSet);
};

class IgnoreListSetup : public ListSecurity
{
    Q_OBJECT
public:
    IgnoreListSetup(QWidget*);
protected:
    virtual bool checkUser(ICQUser *u);
    virtual void changeUser(ICQUser *u, bool bSet);
};

#endif

