/***************************************************************************
                          randomchat.h  -  description
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

#ifndef _RANDOMCHAT_H
#define _RANDOMCHAT_H

#include "defs.h"
#include "randomchatbase.h"

class ICQEvent;
class ICQChatInfo;
class QLineEdit;
struct ext_info;

class RandomChat : public RandomChatBase
{
    Q_OBJECT
public:
    RandomChat();
    ~RandomChat();
signals:
    void finished();
protected slots:
    void apply();
    void search();
    void goUrl();
    void goMyUrl();
    void requestChat();
    void sendMessage();
    void sendUrl();
    void textChanged();
    void textChanged(const QString&);
    void toggledAvailable(bool);
    void processEvent(ICQEvent*);
protected:
    bool bSearch;
    unsigned long Uin();
    void closeEvent(QCloseEvent *e);
    void setFindText();
    void adjustDetails();
    void set(QLineEdit *edit, const ext_info *tbl, unsigned short value);
    ICQChatInfo *msg;
};

#endif

