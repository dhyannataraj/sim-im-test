/***************************************************************************
                          generalsec.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GENERALSEC_H
#define _GENERALSEC_H

#include "defs.h"
#include "generalsecbase.h"

class ICQUser;

class GeneralSecurity : public GeneralSecurityBase
{
    Q_OBJECT
public:
    GeneralSecurity(QWidget *p);
public slots:
    void apply(ICQUser*);
protected slots:
    void rejectToggled(bool);
};

#endif

