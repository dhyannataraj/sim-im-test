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
#include "icqprivate.h"
#include "log.h"

const unsigned short ICQ_SNACxLOC_REQUESTxRIGHTS   = 0x0002;
const unsigned short ICQ_SNAXxLOC_RIGHTSxGRANTED   = 0x0003;
const unsigned short ICQ_SNACxLOC_SETxUSERxINFO    = 0x0004;

void ICQClientPrivate::snac_location(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNAXxLOC_RIGHTSxGRANTED:
        log(L_DEBUG, "Location rights granted");
        break;
    default:
        log(L_WARN, "Unknown location family type %04X", type);
    }
}

void ICQClientPrivate::locationRequest()
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxRIGHTS);
    sendPacket();
}

#define cap_id   0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define cap_str  0xbc, 0xd2, 0x00, 0x04, 0xac, 0x96, 0xdd, 0x96

#define cap_mid  0x4c, 0x7f, 0x11, 0xd1
#define cap_mstr 0x4f, 0xe9, 0xd3, 0x11

const capability arrCapabilities[] =
    {
        // CAP_ISICQ
        { 0x09, 0x46, 0x13, 0x44, cap_mid, cap_id },
        // CAP_RTF
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92 },
        // CAP_STR_2002
        { 0x09, 0x46, 0x13, 0x49, cap_mid, cap_id },
        // CAP_IS_2002
        { 0x09, 0x46, 0x13, 0x4e, cap_mid, cap_id },
        // CAP_SIM
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x00 },
        // CAP_STR_2001
        { 0xa0, 0xe9, 0x3f, 0x37, cap_mstr, cap_str },
        // CAP_IS_2001
        { 0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8,
          0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf },
        // CAP_IS_WEB
        { 0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd,
          0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3 },
        // CAP_TRILLIAN
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x09 },
        // CAP_TRIL_CRYPT
        { 0xf2, 0xe7, 0xc7, 0xf4, 0xfe, 0xad, 0x4d, 0xfb,
          0xb2, 0x35, 0x36, 0x79, 0x8b, 0xdf, 0x00, 0x00 },
        // CAP_LICQ
        { 0x09, 0x49, 0x13, 0x49, cap_mid, cap_id },
        // CAP_MACICQ
        { 0xdd, 0x16, 0xf2, 0x02, 0x84, 0xe6, 0x11, 0xd4,
          0x90, 0xdb, 0x00, 0x10, 0x4b, 0x9b, 0x4b, 0x7d },
        // CAP_AIMCHAT
        { 0x74, 0x8f, 0x24, 0x20, 0x62, 0x87, 0x11, 0xd1, cap_id },
        // CAP_NULL
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };

const capability *ICQClientPrivate::capabilities = arrCapabilities;

#ifndef VERSION
#define VERSION  "0.1"
#endif

void ICQClientPrivate::sendCapability()
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

