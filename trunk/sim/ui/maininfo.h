/***************************************************************************
                          maininfo.h  -  description
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

#ifndef _MAININFO_H
#define _MAININFO_H

#include "defs.h"
#include "maininfobase.h"
#include "client.h"

class ICQUser;
class EMailInfo;

class MainInfo : public MainInfoBase
{
    Q_OBJECT
public:
    MainInfo(QWidget *p, bool readOnly=false);
public slots:
    void apply(ICQUser *u);
    void load(ICQUser *u);
    void save(ICQUser *u);
protected slots:
    void setButtons(int);
    void addEmail();
    void editEmail();
    void removeEmail();
    void defaultEmail();
protected:
    EMailInfo *currentMail();
    void addString(QStringList &list, QString str);
    void reloadList();
    EMailPtrList mails;
    bool bReadOnly;
};

#endif

