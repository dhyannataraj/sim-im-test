/***************************************************************************
                          aiminfo.h  -  description
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

#ifndef _AIMINFO_H
#define _AIMINFO_H

#include "simapi.h"
#include "aiminfobase.h"

class ICQClient;

class AIMInfo : public AIMInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    AIMInfo(QWidget *parent, struct ICQUserData*, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
protected:
    void *processEvent(Event *e);
    void fill();
    struct ICQUserData *m_data;
    bool m_bInit;
	unsigned  m_contact;
    ICQClient *m_client;
};

#endif

