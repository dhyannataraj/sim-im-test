/***************************************************************************
                          ping.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "icqclient.h"
#include "icqprivate.h"
#include "log.h"

const unsigned short ICQ_SNACxPING_UNKNOWN = 0x0002;

void ICQClientPrivate::snac_ping(unsigned short type, unsigned short)
{
    switch (type){
    case  ICQ_SNACxPING_UNKNOWN:
        break;
    default:
        log(L_WARN, "Unknown ping family type %04X", type);
    }
}

