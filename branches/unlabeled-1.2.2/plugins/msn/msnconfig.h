/***************************************************************************
                          msnconfig.h  -  description
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

#ifndef _MSNCONFIG_H
#define _MSNCONFIG_H

#include "simapi.h"
#include "msnconfigbase.h"

class MSNClient;

class MSNConfig : public MSNConfigBase
{
    Q_OBJECT
public:
    MSNConfig(QWidget *parent, MSNClient *client, bool bConfig);
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(Client*, void*);
protected slots:
    void changed();
    void changed(const QString&);
protected:
    bool m_bConfig;
    MSNClient *m_client;
};

#endif

