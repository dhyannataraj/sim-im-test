/***************************************************************************
                          osdconfig.h  -  description
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

#ifndef _OSDCONFIG_H
#define _OSDCONFIG_H

#include "simapi.h"
#include "osdconfigbase.h"

class OSDPlugin;
class OSDIface;

class OSDConfig : public OSDConfigBase
{
    Q_OBJECT
public:
    OSDConfig(QWidget *parent, void *data, OSDPlugin *plugin);
public slots:
    void apply(void *data);
    void apply();
protected:
    OSDIface  *m_iface;
    OSDPlugin *m_plugin;
};

#endif

