/***************************************************************************
                          location.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "icqclient.h"
#include "log.h"

const unsigned short ICQ_SNACxLOC_REQUESTxRIGHTS   = 0x0002;
const unsigned short ICQ_SNAXxLOC_RIGHTSxGRANTED   = 0x0003;
const unsigned short ICQ_SNACxLOC_SETxUSERxINFO    = 0x0004;

void ICQClient::snac_location(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNAXxLOC_RIGHTSxGRANTED:
        log(L_DEBUG, "Location rights granted");
        break;
    default:
        log(L_WARN, "Unknown location family type %04X", type);
    }
}

void ICQClient::locationRequest()
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxRIGHTS);
    sendPacket();
}

const capability arrCapabilities[] =
    {
        {
            0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1,
            0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
        },

{
    0x97, 0xB1, 0x27, 0x51, 0x24, 0x3C, 0x43, 0x34,
    0xAD, 0x22, 0xD6, 0xAB, 0xF7, 0x3F, 0x14, 0x92,
},

{
    0x2E, 0x7A, 0x64, 0x75, 0xFA, 0xDF, 0x4D, 0xC8,
    0x88, 0x6F, 0xEA, 0x35, 0x95, 0xFD, 0xB6, 0xDF,
},

{
    0x09, 0x46, 0x13, 0x44, 0x4C, 0x7F, 0x11, 0xD1,
    0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
},

{
    0x97, 0xB1, 0x27, 0x51, 0x24, 0x3C, 0x43, 0x34,
    0xAD, 0x22, 0xD6, 0xAB, 0xF7, 0x3F, 0x14, 0x09,
},

{
    0x09, 0x46, 0x13, 0x4E, 0x4C, 0x7F, 0x11, 0xD1,
    0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,
},

    };

const capability *ICQClient::capabilities = arrCapabilities;

void ICQClient::sendCapability()
{
    Buffer cap(5 * sizeof(capability));
    capability c;
    memmove(c, capabilities[4], sizeof(c));
    char ver[] = VERSION;
    char pack_ver = (atol(ver) + 1) << 6;
    char *p = strchr(ver, '.');
    if (p) pack_ver += atol(p+1);
    c[sizeof(capability)-1] = pack_ver;
    cap.pack((char*)capabilities[0], sizeof(capability));
    cap.pack((char*)capabilities[1], sizeof(capability));
    cap.pack((char*)capabilities[2], sizeof(capability));
    cap.pack((char*)capabilities[3], sizeof(capability));
    cap.pack((char*)c, sizeof(c));
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    sock->writeBuffer.tlv(0x0005, cap);
    sendPacket();
}

