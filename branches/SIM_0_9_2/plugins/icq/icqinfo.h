/***************************************************************************
                          icqinfo.h  -  description
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

#ifndef _ICQINFO_H
#define _ICQINFO_H

#include "simapi.h"
#include "icqinfobase.h"

class ICQClient;

class ICQInfo : public ICQInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    ICQInfo(QWidget *parent, struct ICQUserData*, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
protected:
    void *processEvent(Event *e);
    void fill();
    struct ICQUserData *m_data;
    bool m_bInit;
    ICQClient *m_client;
};

#endif

