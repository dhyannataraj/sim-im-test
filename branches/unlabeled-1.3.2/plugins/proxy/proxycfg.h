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
#include "proxycfgbase.h"

class ProxyPlugin;
class QTabWidget;

class ProxyConfig : public ProxyConfigBase
{
    Q_OBJECT
public:
    ProxyConfig(QWidget *parent, ProxyPlugin *plugin, QTabWidget *tab);
public slots:
    void apply();
protected slots:
    void typeChanged(int type);
    void authToggled(bool auth);
protected:
    void paintEvent(QPaintEvent*);
    ProxyPlugin *m_plugin;
};

#endif

