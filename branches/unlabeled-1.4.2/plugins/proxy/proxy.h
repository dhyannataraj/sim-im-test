/***************************************************************************
                          proxy.h  -  description
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

#ifndef _PROXY_H
#define _PROXY_H

#include "simapi.h"
#include "stl.h"

const unsigned PROXY_NONE	= 0;
const unsigned PROXY_SOCKS4	= 1;
const unsigned PROXY_SOCKS5 = 2;
const unsigned PROXY_HTTPS	= 3;
const unsigned PROXY_HTTP	= 4;

typedef struct ProxyData
{
    char			*Client;
    void			*Clients;
    unsigned long	Type;
    char			*Host;
    unsigned long	Port;
    unsigned		Auth;
    char			*User;
    char			*Password;
    unsigned		Default;
	unsigned		NoShow;
    bool			bInit;
    ProxyData();
    ProxyData(const ProxyData&);
    ProxyData(const char *cfg);
    ~ProxyData();
    bool operator == (const ProxyData&) const;
    ProxyData& operator = (const ProxyData&);
    ProxyData& operator = (const char *cfg);
} ProxyData;

class Proxy;

class ProxyPlugin : public Plugin, public EventReceiver
{
public:
    ProxyPlugin(unsigned, const char*);
    virtual ~ProxyPlugin();
    PROP_STRLIST(Clients);
    PROP_ULONG(Type);
    PROP_STR(Host);
    PROP_USHORT(Port);
    PROP_BOOL(Auth);
	PROP_BOOL(NoShow);
    PROP_STR(User);
    PROP_STR(Password);
    unsigned ProxyPacket;
    list<Proxy*> proxies;
    ProxyData data;
    void clientData(Client*, ProxyData &data);
    static const DataDef *proxyData;
    unsigned ProxyErr;
protected:
    virtual void *processEvent(Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
};

#endif

