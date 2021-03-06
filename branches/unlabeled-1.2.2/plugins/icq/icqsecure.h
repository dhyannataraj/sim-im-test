/***************************************************************************
                          icqsecure.h  -  description
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

#ifndef _ICQSECURE_H
#define _ICQSECURE_H

#include "simapi.h"
#include "icqsecurebase.h"

class ICQClient;
class ListView;

class ICQSecure : public ICQSecureBase, public EventReceiver
{
    Q_OBJECT
public:
    ICQSecure(QWidget *parent, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
    void hideIpToggled(bool);
protected:
    virtual void *processEvent(Event*);
    void fill();
    void setListView(ListView*);
    void fillListView(ListView*, unsigned offs);
    ICQClient	*m_client;
};

#endif

