/***************************************************************************
                          aimconfig.h  -  description
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

#ifndef _AIMCONFIG_H
#define _AIMCONFIG_H

#include "simapi.h"
#include "aimconfigbase.h"
#include "icqclient.h"

class AIMConfig : public AIMConfigBase
{
    Q_OBJECT
public:
    AIMConfig(QWidget *parent, ICQClient *client, bool bConfig);
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(Client*, void*);
protected slots:
    void changed();
    void changed(const QString&);
    void regClick();
protected:
    bool m_bConfig;
    ICQClient *m_client;
};

#endif

