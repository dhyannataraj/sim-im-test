/***************************************************************************
                          smssetup.h  -  description
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

#ifndef _SMSSETUP_H
#define _SMSSETUP_H

#include "smssetupbase.h"
#include "simapi.h"

class SMSClient;

class SMSSetup : public SMSSetupBase
{
    Q_OBJECT
public:
    SMSSetup(QWidget*, SMSClient*);
signals:
    void okEnabled(bool);
public slots:
    void init();
    void apply();
    void apply(Client*, void*);
protected:
    SMSClient *m_client;
};

#endif

