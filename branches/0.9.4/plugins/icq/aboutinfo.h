/***************************************************************************
                          aboutinfo.h  -  description
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

#ifndef _ABOUTINFO_H
#define _ABOUTINFO_H

#include "simapi.h"
#include "aboutinfobase.h"

class ICQClient;

class AboutInfo : public AboutInfoBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    AboutInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected:
    void *processEvent(SIM::Event*);
    void fill();
    struct ICQUserData	*m_data;
    unsigned	m_contact;
    ICQClient	*m_client;
};

#endif

