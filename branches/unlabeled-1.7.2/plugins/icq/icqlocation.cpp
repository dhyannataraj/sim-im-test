/***************************************************************************
                          icqlocation.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "icqclient.h"

#ifdef WIN32
#include <windows.h>
#endif

const unsigned short ICQ_SNACxLOC_ERROR             = 0x0001;
const unsigned short ICQ_SNACxLOC_REQUESTxRIGHTS    = 0x0002;
const unsigned short ICQ_SNAXxLOC_RIGHTSxGRANTED    = 0x0003;
const unsigned short ICQ_SNACxLOC_SETxUSERxINFO     = 0x0004;
const unsigned short ICQ_SNACxLOC_REQUESTxUSERxINFO = 0x0005;   
const unsigned short ICQ_SNACxLOC_LOCATIONxINFO     = 0x0006;   
const unsigned short ICQ_SNACxLOC_REQUESTxDIRxINFO  = 0x000B;   
const unsigned short ICQ_SNACxLOC_DIRxINFO          = 0x000C;   

static bool extractInfo(TlvList &tlvs, unsigned id, char **data)
{
	const char *info = NULL;
	Tlv *tlv = tlvs(id);
	if (tlv)
		info = *tlv;
	return set_str(data, info);
}

void ICQClient::snac_location(unsigned short type, unsigned short seq)
{
	Contact *contact;
	ICQUserData *data;
    switch (type){
    case ICQ_SNAXxLOC_RIGHTSxGRANTED:
        log(L_DEBUG, "Location rights granted");
        break;
    case ICQ_SNACxLOC_ERROR:
        break;
	case ICQ_SNACxLOC_LOCATIONxINFO:
		break;
	case ICQ_SNACxLOC_DIRxINFO:
		data = findInfoRequest(seq, contact);
		if (data){
			bool bChanged = false;
			m_socket->readBuffer.incReadPos(4);
			TlvList tlvs(m_socket->readBuffer);
			bChanged |= extractInfo(tlvs, 0x01, &data->FirstName);
			bChanged |= extractInfo(tlvs, 0x02, &data->LastName);
			bChanged |= extractInfo(tlvs, 0x03, &data->MiddleName);
			bChanged |= extractInfo(tlvs, 0x04, &data->Maiden);
			bChanged |= extractInfo(tlvs, 0x07, &data->State);
			bChanged |= extractInfo(tlvs, 0x08, &data->City);
			bChanged |= extractInfo(tlvs, 0x0C, &data->Nick);
			bChanged |= extractInfo(tlvs, 0x0D, &data->Zip);
			bChanged |= extractInfo(tlvs, 0x21, &data->Address);
			if (bChanged){
				Event e(EventContactChanged, contact);
//				e.process();
			}
		}
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

#define cap_id   0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define cap_str  0xbc, 0xd2, 0x00, 0x04, 0xac, 0x96, 0xdd, 0x96

#define cap_mid  0x4c, 0x7f, 0x11, 0xd1
#define cap_mstr 0x4f, 0xe9, 0xd3, 0x11

const capability arrCapabilities[] =
    {
        // CAP_DIRECT
        { 0x09, 0x46, 0x13, 0x44, cap_mid, cap_id },
        // CAP_RTF
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92 },
        // CAP_SRV_RELAY
        { 0x09, 0x46, 0x13, 0x49, cap_mid, cap_id },
        // CAP_UTF
        { 0x09, 0x46, 0x13, 0x4e, cap_mid, cap_id },
        // CAP_TYPING
        { 0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd,
          0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3 },
        // CAP_AIM_SUPPORT
        { 0x09, 0x46, 0x13, 0x4d, cap_mid, cap_id },
        // CAP_SIM
        { 'S', 'I', 'M', ' ', 'c', 'l', 'i', 'e',
          'n', 't', ' ', ' ', 0, 0, 0, 0 },
        // CAP_STR_2001
        { 0xa0, 0xe9, 0x3f, 0x37, cap_mstr, cap_str },
        // CAP_IS_2001
        { 0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8,
          0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf },
        // CAP_TRILLIAN
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x09 },
        // CAP_TRIL_CRYPT
        { 0xf2, 0xe7, 0xc7, 0xf4, 0xfe, 0xad, 0x4d, 0xfb,
          0xb2, 0x35, 0x36, 0x79, 0x8b, 0xdf, 0x00, 0x00 },
        // CAP_MACICQ
        { 0xdd, 0x16, 0xf2, 0x02, 0x84, 0xe6, 0x11, 0xd4,
          0x90, 0xdb, 0x00, 0x10, 0x4b, 0x9b, 0x4b, 0x7d },
        // CAP_AIM_CHAT
        { 0x74, 0x8f, 0x24, 0x20, 0x62, 0x87, 0x11, 0xd1, cap_id },
        // CAP_AIM_BUDDYCON
        { 0x09, 0x46, 0x13, 0x46, cap_mid, cap_id },
        // CAP_AIM_IMIMAGE
        { 0x09, 0x46, 0x13, 0x45, cap_mid, cap_id },
        // CAP_AIM_SENDFILE
        { 0x09, 0x46, 0x13, 0x43, cap_mid, cap_id },
        // CAP_MICQ
        { 'm', 'I', 'C', 'Q', ' ', '©', 'R', '.',
          'K', ' ', '.', ' ', 0, 0, 0, 0 },
        // CAP_LICQ
        { 'L', 'i', 'c', 'q', ' ', 'c', 'l', 'i',
          'e', 'n', 't', ' ', 0, 0, 0, 0 },
        // CAP_SIMOLD
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x00 },
        // CAP_NULL
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };

const capability *ICQClient::capabilities = arrCapabilities;

#ifndef VERSION
#define VERSION  "0.1"
#endif

static unsigned char get_ver(const char *&v)
{
    if (v == NULL)
        return 0;
    char c = atol(v);
    v = strchr(v, '.');
    if (v)
        v++;
    return c;
}

void ICQClient::sendCapability()
{
    Buffer cap;
    capability c;
    memcpy(c, capabilities[CAP_SIM], sizeof(c));
    const char *ver = VERSION;
    unsigned char *pack_ver = c + sizeof(capability) - 4;
    *(pack_ver++) = get_ver(ver);
    *(pack_ver++) = get_ver(ver);
    *(pack_ver++) = get_ver(ver);
    unsigned char os_ver;
#ifdef WIN32
    os_ver = 0x80;
#else
    os_ver = 0;
#endif
    *(pack_ver++) = os_ver;
    if (m_bAIM){
        cap.pack((char*)capabilities[CAP_AIM_CHAT], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_BUDDYCON], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_IMIMAGE], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_SENDFILE], sizeof(capability));
    }else{
        cap.pack((char*)capabilities[CAP_DIRECT], sizeof(capability));
        cap.pack((char*)capabilities[CAP_SRV_RELAY], sizeof(capability));
        if (getSendFormat() <= 1)
            cap.pack((char*)capabilities[CAP_UTF], sizeof(capability));
        if (getSendFormat() == 0)
            cap.pack((char*)capabilities[CAP_RTF], sizeof(capability));
    }
    if (getTypingNotification())
        cap.pack((char*)capabilities[CAP_TYPING], sizeof(capability));
    cap.pack((char*)capabilities[CAP_AIM_SUPPORT], sizeof(capability));
    cap.pack((char*)c, sizeof(c));
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    if (m_bAIM){
        m_socket->writeBuffer.tlv(0x0001, "text/aolrtf; charset=\"us-ascii\"");
        m_socket->writeBuffer.tlv(0x0002, "\x00", 1);
    }
    m_socket->writeBuffer.tlv(0x0005, cap);
    if (m_bAIM)
        m_socket->writeBuffer.tlv(0x0006, "\x00\x04\x00\x02\x00\x02", 6);
    sendPacket();
}

void ICQClient::fetchProfile(ICQUserData *data)
{
	snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxUSERxINFO, true);
	m_socket->writeBuffer << (unsigned short)0x0001;
	m_socket->writeBuffer.packScreen(screen(data).c_str());
	sendPacket();
	snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxDIRxINFO, true);
	m_socket->writeBuffer.packScreen(screen(data).c_str());
	sendPacket();
	m_info_req.insert(INFO_REQ_MAP::value_type(m_nMsgSequence, screen(data)));
}

void ICQClient::fetchProfiles()
{
				Contact *contact;
				ContactList::ContactIterator itc;
				while ((contact = ++itc) != NULL){
					ICQUserData *data;
					ClientDataIterator itd(contact->clientData, this);
					while ((data = (ICQUserData*)(++itd)) != NULL){
						if (data->Uin || data->ProfileFetch)
							continue;
						fetchProfile(data);
					}
				}
}

ICQUserData *ICQClient::findInfoRequest(unsigned short seq, Contact *&contact)
{
	INFO_REQ_MAP::iterator it = m_info_req.find(seq);
	if (it == m_info_req.end()){
		log(L_WARN, "Info req %u not found", seq);
		return NULL;
	}
	string screen = (*it).second;
	m_info_req.erase(it);
	return findContact(screen.c_str(), NULL, false, contact);
}


