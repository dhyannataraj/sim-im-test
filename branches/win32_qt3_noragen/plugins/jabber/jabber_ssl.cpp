/***************************************************************************
                          jabber_ssl.cpp  -  description
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

#include "simapi.h"

#ifdef USE_OPENSSL

#include <openssl/ssl.h>
#include "jabber_ssl.h"

JabberSSL::JabberSSL(Socket *s)
        : SSLClient(s)
{
}

bool JabberSSL::initSSL()
{
    mpCTX = SSL_CTX_new(TLSv1_method());
    if (mpCTX == NULL)
        return false;
    mpSSL = SSL_new(pCTX);
    if(!mpSSL)
        return false;
    return true;
}

#endif

