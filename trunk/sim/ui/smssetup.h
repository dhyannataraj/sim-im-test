/***************************************************************************
                          smssetup.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _SMSSETUP_H
#define _SMSSETUP_H

#include "defs.h"
#include "smssetupbase.h"

class ICQUser;

class SMSSetup : public SMSSetupBase
{
    Q_OBJECT
public:
    SMSSetup(QWidget*);
public slots:
    void apply(ICQUser*);
};

#endif

