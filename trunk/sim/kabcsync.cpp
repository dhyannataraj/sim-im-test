/***************************************************************************
                          kabcsync.cpp  -  description
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
 
#include "kabcsync.h"
 
KabcSync::KabcSync(void):QObject(),m_bOpen(false)
{
 
}
 
KabcSync::~KabcSync()
{

}


bool KabcSync::open(void)
{

}

void KabcSync::close(void)
{
	m_bOpen=false;
}

Addressee KabcSync::addresseeFromUser(SIMUser& u, Addressee* oldPers)
{

}
