/***************************************************************************
                          icqdirect.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "icqclient.h"

ICQListener::ICQListener(ICQClient *_client)
{
    sock = getSocketFactory()->createServerSocket();
    sock->setNotify(this);
    client = _client;
}

ICQListener::~ICQListener()
{
    if (sock) delete sock;
}

void ICQListener::accept(Socket *s)
{
    delete s;
}

unsigned short ICQListener::port()
{
    if (sock) return sock->port();
    return 0;
}
