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
class ICQGroup;
class QListViewItem;

class UserInfo : public UserInfoBase
{
    Q_OBJECT
public:
    UserInfo(unsigned long uin, unsigned short grpId, int page);
    unsigned long Uin() { return m_nUin; }
    unsigned short GrpId() { return m_nGrpId; }
    void raiseWidget(int id);
signals:
    void saveInfo(ICQUser*);
    void saveInfo(ICQGroup*);
    void loadInfo(ICQUser*);
    void loadInfo(ICQGroup*);
protected slots:
    void selectionChanged();
    void update();
    void saveInfo();
    void processEvent(ICQEvent *e);
protected:
    void setTitle();
    void setIcon();
    bool inSave;
    void addWidget(PAGEPROC *pageProc, int, const QString&, const char*, int param=0);
    unsigned long m_nUin;
    unsigned short m_nGrpId;
    void loadInfo();
    bool raiseWidget(QListViewItem *i, unsigned id);
    QListViewItem *itemMain;
};

#endif

