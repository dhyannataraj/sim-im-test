/***************************************************************************
                          scuredlg.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SECUREDLG_H
#define _SECUREDLG_H

#include "defs.h"
#include "securedlgbase.h"

class ICQEvent;

class SecureDlg : public SecureDlgBase
{
    Q_OBJECT
public:
    SecureDlg(QWidget *parent, unsigned long uin);
    unsigned long Uin;
protected slots:
    void processEvent(ICQEvent*);
    void sendRequest();
};

#endif

