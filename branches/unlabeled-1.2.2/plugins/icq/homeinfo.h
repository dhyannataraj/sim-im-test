/***************************************************************************
                          homeinfo.h  -  description
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

#ifndef _HOMEINFO_H
#define _HOMEINFO_H

#include "simapi.h"
#include "homeinfobase.h"

class ICQClient;

class HomeInfo : public HomeInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    HomeInfo(QWidget *parent, struct ICQUserData*, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
protected:
    void *processEvent(Event*);
    void fill();
    struct ICQUserData *m_data;
	unsigned m_contact;
    ICQClient *m_client;
};

#endif

