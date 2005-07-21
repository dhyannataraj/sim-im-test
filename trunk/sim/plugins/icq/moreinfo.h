/***************************************************************************
                          moreinfo.h  -  description
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

#ifndef _MOREINFO_H
#define _MOREINFO_H

#include "simapi.h"
#include "moreinfobase.h"

class ICQClient;

class MoreInfo : public MoreInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    MoreInfo(QWidget *parent, struct ICQUserData *data, unsigned contact, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
    void goUrl();
    void urlChanged(const QString&);
    void setLang(int);
    void birthDayChanged();
protected:
    void *processEvent(Event*);
    void fill();
    struct ICQUserData *m_data;
    unsigned  m_contact;
    ICQClient *m_client;
};

#endif

