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

#define cap_id   "\x82\x22\x44\x45\x53\x54\x00\x00"
#define cap_none "\x00\x00\x00\x00\x00\x00\x00\x00"
#define cap_str  "\xbc\xd2\x00\x04\xac\x96\xdd\x96"

#define cap_mid  "\x4c\x7f\x11\xd1"
#define cap_mstr "\x4f\xe9\xd3\x11"

const capability arrCapabilities[] =
{
	"\x09\x46\x13\x44" cap_mid cap_id,					// CAP_ISICQ
	"\x97\xb1\x27\x51\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x92",	// CAP_RTF
	"\x10\xcf\x40\xd1" cap_mstr cap_str,					// CAP_STR_2002
	"\x09\x46\x13\x4e" cap_mid cap_id,					// CAP_IS_2002
	"\x97\xb1\x27\x51\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x00",	// CAP_SIM
	"\xa0\xe9\x3f\x37" cap_mstr cap_str,					// CAP_STR_2001
	"\x2e\x7a\x64\x75\xfa\xdf\x4d\xc8\x88\x6f\xea\x35\x95\xfd\xb6\xdf",	// CAP_IS_2001
	"\x56\x3f\xc8\x09\x0b\x6f\x41\xbd\x9f\x79\x42\x26\x09\xdf\xa2\xf3",	// CAP_IS_WEB
	"\x97\xb1\x27\x51\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x09",	// CAP_TRILLIAN
	"\xf2\xe7\xc7\xf4\xfe\xad\x4d\xfb\xb2\x35\x36\x79\x8b\xdf\x00\x00",	// CAP_TRIL_CRYPT
	"\x09\x49\x13\x49" cap_mid cap_id,					// CAP_LICQ
	"\xdd\x16\xf2\x02\x84\xe6\x11\xd4\x90\xdb\x00\x10\x4b\x9b\x4b\x7d",	// CAP_MACICQ
	"\x74\x8f\x24\x20\x62\x87\x11\xd1" cap_id,				// CAP_AIM_CHAT
	"\x00"
};

const capability *ICQClient::capabilities = arrCapabilities;

#ifndef VERSION
#define VERSION  "0.1"
#endif

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

