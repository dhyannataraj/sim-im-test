/***************************************************************************
                          kabcsync.h  -  description
                             -------------------
    begin                : Mon Dec 02 2002
    copyright            : (C) 2002 by Vladimir Shutoff (shutoff@mail.ru)
    copyright            : (C) 2002 by Stanislav Klyuhin (crz@hot.ee)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KABCSYNC_H
#define _KABCSYNC_H 	1

#include "defs.h"

#ifdef USE_KDE
#ifdef HAVE_KABC

#include <qobject.h>
#include <qstring.h>
#include <kabc/addressee.h>
#include <kabc/addressbook.h>

#include "client.h"

using namespace KABC;

class KabcSync:public QObject
{
    Q_OBJECT
public:
    KabcSync(void);
    ~KabcSync();
protected:
    bool m_bOpen;
    AddressBook* m_pAB;
public:
    bool open(void);
    void close(void);
    void processUser(SIMUser&);
protected:
    Addressee addresseeFromUser(SIMUser&,Addressee*);
    QString& getCountry(unsigned short code);
public slots:
    void processEvent(ICQEvent*);
};

extern KabcSync* pSyncher;

#endif
#endif
#endif

