/***************************************************************************
                          service.cpp  -  description
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

#include <time.h>
#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

const unsigned short ICQ_SNACxSRV_READYxCLIENT  = 0x0002;
const unsigned short ICQ_SNACxSRV_READYxSERVER	= 0x0003;
const unsigned short ICQ_SNACxSRV_REQxRATExINFO = 0x0006;
const unsigned short ICQ_SNACxSRV_RATExINFO     = 0x0007;
const unsigned short ICQ_SNACxSRV_RATExACK      = 0x0008;
const unsigned short ICQ_SNACxSRV_RATExCHANGE   = 0x000A;
const unsigned short ICQ_SNACxSRV_GETxUSERxINFO = 0x000E;
const unsigned short ICQ_SNACxSRV_NAMExINFO     = 0x000F;
const unsigned short ICQ_SNACxSRV_NAMExINFOxACK = 0x0011;
const unsigned short ICQ_SNACxSRV_MOTD          = 0x0013;
const unsigned short ICQ_SNACxSRV_IMxICQ 	= 0x0017;
const unsigned short ICQ_SNACxSRV_ACKxIMxICQ    = 0x0018;
const unsigned short ICQ_SNACxSRV_SETxSTATUS    = 0x001E;

void ICQClient::snac_service(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxSRV_RATExCHANGE:
        break;
    case ICQ_SNACxSRV_RATExINFO:
        log(L_DEBUG, "Rate info");
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_RATExACK);
        writeBuffer << 0x00010002L << 0x00030004L << 0x0005;
        sendPacket();
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
        sendPacket();
        listsRequest();
        locationRequest();
        buddyRequest();
        messageRequest();
        bosRequest();
        break;
    case ICQ_SNACxSRV_MOTD:
        log(L_DEBUG, "Motd");
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_REQxRATExINFO);
        sendPacket();
        break;
    case ICQ_SNACxSRV_ACKxIMxICQ:
        log(L_DEBUG, "Ack im icq");
        break;
    case ICQ_SNACxSRV_NAMExINFO:{
            unsigned long uin = readBuffer.unpackUin();
            if (uin == 0){
                char n;
                readBuffer >> n;
                readBuffer.incReadPos(n);
                uin = readBuffer.unpackUin();
            }
            if (uin != Uin()){
                log(L_WARN, "No my name info (%lu)", uin);
                break;
            }
            readBuffer.incReadPos(4);
            TlvList tlv(readBuffer);
            Tlv *tlvIP = tlv(0x000A);
            if (tlvIP)
                RealIP = htonl((unsigned long)(*tlvIP));
            log(L_DEBUG, "Name info");
            bool bSend = true;
            if (needPhonebookUpdate){
                sendInfoUpdate();
                needPhonebookUpdate = false;
                bSend = false;
            }
            if (needPhoneStatusUpdate){
                sendPhoneStatus();
                needPhoneStatusUpdate = false;
                bSend = false;
            }
            break;
        }
    case ICQ_SNACxSRV_READYxSERVER:
        log(L_DEBUG, "Server ready");
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_IMxICQ);
        writeBuffer << 0x00010003L;
        writeBuffer << 0x00130002L;
        writeBuffer << 0x00020001L;
        writeBuffer << 0x00030001L;
        writeBuffer << 0x00150001L;
        writeBuffer << 0x00040001L;
        writeBuffer << 0x00060001L;
        writeBuffer << 0x00090001L;
        writeBuffer << 0x000a0001L;
        writeBuffer << 0x000b0001L;
        sendPacket();
        break;
    default:
        log(L_WARN, "Unknown service family type %04X", type);
    }
}

void ICQClient::sendLogonStatus()
{
    IP = 0;
    char *host;
    unsigned short port;
    if (getLocalAddr(host, port))
    	IP = inet_addr(host);

    log(L_DEBUG, "Logon status");
    if (inInvisible()) sendVisibleList();
    sendContactList();

    unsigned long now;
    time((time_t*)&now);

    Buffer directInfo(25);
    directInfo
    << (unsigned long)0
    << (unsigned long)0

    << (char)0x01	// Mode
    << (char)0x00
    << (char)ICQ_TCP_VERSION

    << DCcookie
    << 0x00000050L
    << 0x00000003L
    << PhoneBookTime()
    << PhoneStatusTime()
    << PhoneBookTime()
    << (unsigned short) 0x0000;

    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    writeBuffer.tlv(0x0006, fullStatus(m_nLogonStatus));
    writeBuffer.tlv(0x0008, 0);
    writeBuffer.tlv(0x000C, directInfo);

    sendPacket();

    if (!inInvisible()) sendInvisibleList();
    m_state = Logged;
    OnlineTime = now;

    uStatus = m_nLogonStatus & 0xFF;
    ICQEvent e(EVENT_STATUS_CHANGED, Uin);
    process_event(&e);

    processMsgQueue();

    lastInfoRequestTime = now;
    lastPhoneRequestTime = now;
    lastResponseRequestTime = now;
}

void ICQClient::setInvisible(bool bInvisible)
{
    if (inInvisible() == bInvisible) return;
    inInvisible = bInvisible;
    if (m_state != Logged) return;
    if (inInvisible()) sendVisibleList();
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    writeBuffer.tlv(0x0006, fullStatus(uStatus));
    sendPacket();
    if (!inInvisible()) sendInvisibleList();
    ICQEvent e(EVENT_STATUS_CHANGED, Uin);
    process_event(&e);
}

void ICQClient::sendStatus(unsigned long status)
{
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    writeBuffer.tlv(0x0006, fullStatus(status));
    sendPacket();
    uStatus = status & 0xFF;
    ICQEvent e(EVENT_STATUS_CHANGED, Uin);
    process_event(&e);
}

void ICQClient::sendClientReady()
{
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_READYxCLIENT);
    writeBuffer
    << 0x00010003L << 0x0110047BL
    << 0x00130002L << 0x0110047BL
    << 0x00020001L << 0x0101047BL
    << 0x00030001L << 0x0110047BL
    << 0x00150001L << 0x0110047BL
    << 0x00040001L << 0x0110047BL
    << 0x00060001L << 0x0110047BL
    << 0x00090001L << 0x0110047BL
    << 0x000A0001L << 0x0110047BL
    << 0x000B0001L << 0x0110047BL;

    sendPacket();
}

void ICQClient::sendUpdate(Buffer &b, unsigned long t1, unsigned long t2, unsigned long t3)
{
    Buffer directInfo(25);
    directInfo
    << (unsigned long)0
    << (unsigned long)0

    << (char)0x01	// Mode
    << (char)0x00
    << (char)ICQ_TCP_VERSION;

    directInfo
    << DCcookie
    << 0x00000050L
    << 0x00000003L
    << t1
    << t2
    << t3
    << (unsigned short) 0x0000;

    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    writeBuffer.tlv(0x0006, fullStatus(uStatus));
    writeBuffer.tlv(0x000C, directInfo);

    writeBuffer.tlv(0x11, b);
    writeBuffer.tlv(0x12, (unsigned short)0x0000);
    sendPacket();
}

bool ICQClient::updatePhoneBook()
{
    if (m_state != Logged) return false;
    unsigned long now;
    time((time_t*)&now);
    Buffer b;
    b << (char)2 << (unsigned long)htonl(now);
    b << (unsigned short)0x0200 << 0x01000100L;
    b.pack((char*)PHONEBOOK_SIGN, 0x10);
    b << (unsigned short)0;
    b << (unsigned long)htonl(now);
    b << (char)0;
    sendUpdate(b, PhoneBookTime(), PhoneStatusTime(), now);
    PhoneBookTime = now;
    needPhonebookUpdate = true;
    return true;
}

void ICQClient::sendInfoUpdate()
{
    Buffer b;
    b << (char)1 << (unsigned long)htonl(PhoneBookTime());
    sendUpdate(b, PhoneBookTime(), PhoneStatusTime(), PhoneBookTime());
    ICQEvent e(EVENT_INFO_CHANGED, Uin);
    sendMessageRequest();
    process_event(&e);
}

bool ICQClient::updatePhoneStatus()
{
    if (m_state != Logged) return false;
    unsigned long now;
    time((time_t*)&now);
    PhoneStatusTime = now;
    Buffer b;
    b << (char)3 << (unsigned long)htonl(PhoneStatusTime());
    b << (unsigned short)0x0000 << 0x01000100L;
    b.pack((const char*)PHONEBOOK_SIGN, 0x10);
    b << (char)0x02 << (unsigned short)0x0001 << PhoneState << (char)0 << (unsigned short) 0;
    b << (unsigned long)htonl(PhoneStatusTime());
    b << (unsigned short)0 << 0x00000100L;
    sendUpdate(b, PhoneBookTime(), PhoneStatusTime(), PhoneBookTime());
    needPhoneStatusUpdate = true;
    ICQEvent e(EVENT_INFO_CHANGED, Uin);
    process_event(&e);
    return true;
}

