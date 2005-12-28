/***************************************************************************
                          jabber_ssl.h  -  description
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

#ifndef _JABBER_SSL_H
#define _JABBER_SSL_H

#include "socket.h"

#ifdef USE_OPENSSL

class JabberSSL : public SSLClient
{
public:
    JabberSSL(Socket*);
protected:
    virtual bool initSSL();
};

#endif
#endif

