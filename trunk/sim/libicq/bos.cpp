/***************************************************************************
                          bos.cpp  -  description
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

const unsigned short ICQ_SNACxBOS_REQUESTxRIGHTS     = 0x0002;
const unsigned short ICQ_SNACxBOS_RIGHTSxGRANTED     = 0x0003;
const unsigned short ICQ_SNACxBOS_ADDxVISIBLExLIST   = 0x0005;
const unsigned short ICQ_SNACxBOS_REMxVISIBLExLIST   = 0x0006;
const unsigned short ICQ_SNACxBOS_ADDxINVISIBLExLIST = 0x0007;
const unsigned short ICQ_SNACxBOS_REMxINVISIBLExLIST = 0x0008;

void ICQClientPrivate::snac_bos(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxBOS_RIGHTSxGRANTED:
        log(L_DEBUG, "BOS rights granted");
        break;
    default:
        log(L_WARN, "Unknown bos family type %04X", type);
    }
}

void ICQClientPrivate::bosRequest()
{
    snac(ICQ_SNACxFAM_BOS, ICQ_SNACxBOS_REQUESTxRIGHTS);
    sendPacket();
}

