/***************************************************************************
                          warndlg.h  -  description
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

#ifndef _WARNDLG_H
#define _WARNDLG_H

#include "simapi.h"
#include "warndlgbase.h"
#include "icqclient.h"

class WarnDlg : public WarnDlgBase
{
    Q_OBJECT
public:
    WarnDlg(QWidget *parent, ICQUserData *data, ICQClient *client);
public slots:
    void apply();
protected:
    ICQUserData	*m_data;
    ICQClient	*m_client;
};

#endif

