/***************************************************************************
                          msgdialog.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _MSGDIALOG_H
#define _MSGDIALOG_H

#include "defs.h"
#include "msgdialogbase.h"
#include <string>

using namespace std;

class ICQUser;
class ICQGroup;
struct UserSettings;

class MsgDialog : public MsgDialogBase
{
    Q_OBJECT
public:
    MsgDialog(QWidget *p, unsigned long status, bool bReadOnly=false);
public slots:
    void apply(ICQUser *u);
    void load(ICQUser *u);
    void save(ICQUser *u);
    void load(ICQGroup *g);
    void save(ICQGroup *g);
protected slots:
    void overrideChanged(bool);
protected:
    void setup(ICQUser *u, unsigned offs);
    void setup(ICQGroup *g, unsigned offs);
    void save(UserSettings *settings);
    unsigned long status;
};

#endif

