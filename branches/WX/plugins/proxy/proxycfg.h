/***************************************************************************
                          proxycfg.h  -  description
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

#ifndef _PROXYCFG_H
#define _PROXYCFG_H

#include "simapi.h"
#include "proxy.h"
#include "proxycfgbase.h"

#include <vector>
using namespace std;

class ProxyPlugin;
class QTabWidget;

class ProxyConfig : public ProxyConfigBase, public EventReceiver
{
    Q_OBJECT
public:
    ProxyConfig(QWidget *parent, ProxyPlugin *plugin, QTabWidget *tab, Client *client);
public slots:
    void apply();
protected slots:
    void clientChanged(int client);
    void typeChanged(int type);
    void authToggled(bool auth);
protected:
    void paintEvent(QPaintEvent*);
    void *processEvent(Event*);
    void fillClients();
    void fill(ProxyData*);
    void get(ProxyData*);
    vector<ProxyData> m_data;
    Client *m_client;
    ProxyPlugin *m_plugin;
    unsigned m_current;
};

#endif

