/***************************************************************************
                          yahoo.h  -  description
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

#ifndef _YAHOO_H
#define _YAHOO_H

#include "simapi.h"

class YahooProtocol : public Protocol
{
public:
    YahooProtocol(Plugin *plugin);
    ~YahooProtocol();
    Client	*createClient(const char *cfg);
    const CommandDef *description();
    const CommandDef *statusList();
    virtual const DataDef *userDataDef();
};

class YahooPlugin : public Plugin
{
public:
    YahooPlugin(unsigned);
    virtual ~YahooPlugin();
    static unsigned YahooPacket;
protected:
    Protocol *m_protocol;
};

#endif

