/***************************************************************************
                          userinfo.h  -  description
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

#ifndef _USERINFO_H
#define _USERINFO_H

#include "defs.h"
#include "userinfobase.h"
#include "setupdlg.h"

class ICQEvent;
class ICQUser;
class QListViewItem;

class UserInfo : public UserInfoBase
{
    Q_OBJECT
public:
    UserInfo(QWidget *parent, unsigned long uin, int page);
    unsigned long Uin() { return m_nUin; }
signals:
    void saveInfo(ICQUser*);
    void loadInfo(ICQUser*);
protected slots:
    void selectionChanged();
    void update();
    void saveInfo();
    void processEvent(ICQEvent *e);
protected:
    bool inSave;
    void addWidget(PAGEPROC *pageProc, int, const QString&, const char*, int param=0);
    unsigned long m_nUin;
    void loadInfo();
    void raiseWidget(int id);
    bool raiseWidget(QListViewItem *i, unsigned id);
    QListViewItem *itemMain;
};

#endif

