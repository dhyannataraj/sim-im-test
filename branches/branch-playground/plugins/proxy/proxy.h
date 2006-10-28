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
#include <qvaluelist.h>

const unsigned PROXY_NONE	= 0;
const unsigned PROXY_SOCKS4	= 1;
const unsigned PROXY_SOCKS5 = 2;
const unsigned PROXY_HTTPS	= 3;

typedef struct ProxyData
{
    SIM::Data	Client;
    SIM::Data	Clients;
    SIM::Data	Type;
    SIM::Data	Host;
    SIM::Data	Port;
    SIM::Data	Auth;
    SIM::Data	User;
    SIM::Data	Password;
    SIM::Data	Default;
    SIM::Data	NoShow;
    bool	bInit;
    ProxyData();
    ProxyData(const ProxyData&);
    ProxyData(const char *cfg);
    ~ProxyData();
    bool operator == (const ProxyData&) const;
    ProxyData& operator = (const ProxyData&);
    ProxyData& operator = (ConfigBuffer *cfg);
} ProxyData;

class Proxy;
class Listener;

class ProxyPlugin : public SIM::Plugin, public SIM::EventReceiver
{
public:
    ProxyPlugin(unsigned, ConfigBuffer*);
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
    QValueList<Proxy*>	proxies;
    ProxyData data;
    void clientData(SIM::TCPClient*, ProxyData &data);
    static const SIM::DataDef *proxyData;
    unsigned ProxyErr;
    QString clientName(SIM::TCPClient *client);
protected:
    virtual void *processEvent(SIM::Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QString getConfig();
};

#endif

