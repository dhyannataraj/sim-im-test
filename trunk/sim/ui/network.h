/***************************************************************************
                          network.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _NETWORK_H
#define _NETWORK_H

#include "defs.h"
#include "networkbase.h"

class ICQUser;

class NetworkSetup : public NetworkBase
{
    Q_OBJECT
public:
    NetworkSetup(QWidget *p);
public slots:
    void apply(ICQUser*);
protected slots:
    void proxyChanged(int);
    void proxyChanged(bool);
};

#endif

