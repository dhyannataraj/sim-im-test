/***************************************************************************
                          proxyerror.h  -  description
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

#ifndef _PROXYERROR_H
#define _PROXYERROR_H

#include "simapi.h"
#include "socket.h"
#include "proxyerrorbase.h"

class ProxyConfig;
class ProxyPlugin;

class ProxyError : public ProxyErrorBase, public EventReceiver
{
    Q_OBJECT
public:
    ProxyError(ProxyPlugin *plugin, TCPClient *client, const char *msg);
    ~ProxyError();
signals:
    void apply();
protected:
    void *processEvent(Event*);
    virtual void accept();
    ProxyPlugin *m_plugin;
    TCPClient	*m_client;
};

#endif

