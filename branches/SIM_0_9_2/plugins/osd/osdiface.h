/***************************************************************************
                          osdiface.h  -  description
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

#ifndef _OSDIFACE_H
#define _OSDIFACE_H

#include "simapi.h"
#include "osdifacebase.h"

class OSDPlugin;

class OSDIface : public OSDIfaceBase
{
    Q_OBJECT
public:
    OSDIface(QWidget *parent, void *data, OSDPlugin *plugin);
public slots:
    void apply(void *data);
protected slots:
    void bgToggled(bool);
protected:
    OSDPlugin *m_plugin;
};

#endif

