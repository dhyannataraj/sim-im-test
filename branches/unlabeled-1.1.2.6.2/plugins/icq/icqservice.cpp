/***************************************************************************
                          icqservice.cpp  -  description
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

#include <time.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <qtimer.h>

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
const unsigned short ICQ_SNACxSRV_IMxICQ 		= 0x0017;
const unsigned short ICQ_SNACxSRV_ACKxIMxICQ    = 0x0018;
const unsigned short ICQ_SNACxSRV_SETxSTATUS    = 0x001E;
const unsigned short ICQ_SNACxSRV_SETxIDLE		= 0x001F;

void ICQClient::snac_service(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxSRV_RATExCHANGE:
        log(L_DEBUG, "Rate change");
        if (m_nSendTimeout < 200){
            m_nSendTimeout = m_nSendTimeout + 2;
            if (m_sendTimer->isActive()){
                m_sendTimer->stop();
                m_sendTimer->start(m_nSendTimeout * 500);
            }
        }
        break;
    case ICQ_SNACxSRV_RATExINFO:
        log(L_DEBUG, "Rate info");
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_RATExACK);
        m_socket->writeBuffer << 0x00010002L << 0x00030004L << 0x0005;
        sendPacket();
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_GETxUSERxINFO);
        sendPacket();
        listsRequest();
        locationRequest();
        buddyRequest();
        icmbRequest();
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
            unsigned long uin = m_socket->readBuffer.unpackUin();
            if (uin == 0){
                char n;
                m_socket->readBuffer >> n;
                m_socket->readBuffer.incReadPos(n);
                uin = m_socket->readBuffer.unpackUin();
            }
            if (uin != data.owner.Uin){
                log(L_WARN, "No my name info (%lu)", uin);
                break;
            }
            m_socket->readBuffer.incReadPos(4);
            TlvList tlv(m_socket->readBuffer);
            Tlv *tlvIP = tlv(0x000A);
            if (tlvIP)
                set_ip(&data.owner.IP, htonl((unsigned long)(*tlvIP)));
            log(L_DEBUG, "Name info");
            break;
        }
    case ICQ_SNACxSRV_READYxSERVER:
        log(L_DEBUG, "Server ready");
        snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_IMxICQ);
        m_socket->writeBuffer
        << 0x00010003L
        << 0x00130002L
        << 0x00020001L
        << 0x00030001L
        << 0x00150001L
        << 0x00040001L
        << 0x00060001L
        << 0x00090001L
        << 0x000a0001L
        << 0x000b0001L;
        sendPacket();
        break;
    default:
        log(L_WARN, "Unknown service family type %04X", type);
    }
}

void ICQClient::sendClientReady()
{
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_READYxCLIENT);
    m_socket->writeBuffer
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

void ICQClient::sendLogonStatus()
{
    log(L_DEBUG, "Logon status %u", m_logonStatus);
    if (getInvisible())
        sendVisibleList();
    sendContactList();

    unsigned long now;
    time((time_t*)&now);
    if (data.owner.PluginInfoTime == 0)
        data.owner.PluginInfoTime = now;
    if (data.owner.PluginStatusTime == 0)
        data.owner.PluginStatusTime = now;
    if (data.owner.InfoUpdateTime == 0)
        data.owner.InfoUpdateTime = now;
    data.owner.OnlineTime = now;
    if (getContacts()->owner()->getPhones() != QString::fromUtf8(data.owner.PhoneBook)){
        set_str(&data.owner.PhoneBook, getContacts()->owner()->getPhones().utf8());
        data.owner.PluginInfoTime = now;
    }
    if (getPicture() != QString::fromUtf8(data.owner.Picture)){
        set_str(&data.owner.Picture, getPicture().utf8());
        data.owner.PluginInfoTime = now;
    }
    if (getContacts()->owner()->getPhoneStatus() != data.owner.FollowMe){
        data.owner.FollowMe = getContacts()->owner()->getPhoneStatus();
        data.owner.PluginStatusTime = now;
    }

    Buffer directInfo(25);
    fillDirectInfo(directInfo);

    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    m_socket->writeBuffer.tlv(0x0006, fullStatus(m_logonStatus));
    m_socket->writeBuffer.tlv(0x0008, 0);
    m_socket->writeBuffer.tlv(0x000C, directInfo);

    sendPacket();
    if (!getInvisible())
        sendInvisibleList();
    sendIdleTime();
    m_status = m_logonStatus;
    QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
    setState(Connected);
    Event e(EventClientChanged, this);
    e.process();
}

void ICQClient::setInvisible()
{
    if (getInvisible()) sendVisibleList();
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    m_socket->writeBuffer.tlv(0x0006, fullStatus(m_status));
    sendPacket();
    if (!getInvisible()) sendInvisibleList();
}

void ICQClient::sendStatus()
{
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    m_socket->writeBuffer.tlv(0x0006, fullStatus(m_status));
    sendPacket();
    sendIdleTime();
}

void ICQClient::sendPluginInfoUpdate(unsigned plugin_id)
{
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    m_socket->writeBuffer.tlv(0x0006, fullStatus(m_status));
    Buffer directInfo(25);
    fillDirectInfo(directInfo);
    m_socket->writeBuffer.tlv(0x000C, directInfo);
    Buffer b;
    b << (char)2;
    b.pack(data.owner.PluginInfoTime);
    b.pack((unsigned short)2);
    b.pack((unsigned short)1);
    b.pack((unsigned short)2);
    b.pack((char*)plugins[plugin_id], sizeof(plugin));
    b.pack(data.owner.PluginInfoTime);
    b << (char)0;
    m_socket->writeBuffer.tlv(0x0011, b);
    m_socket->writeBuffer.tlv(0x0012, (unsigned short)0);
    sendPacket();
}

void ICQClient::sendPluginStatusUpdate(unsigned plugin_id, unsigned long status)
{
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    m_socket->writeBuffer.tlv(0x0006, fullStatus(m_logonStatus));
    Buffer directInfo(25);
    fillDirectInfo(directInfo);
    m_socket->writeBuffer.tlv(0x000C, directInfo);
    Buffer b;
    b << (char)3;
    b.pack(data.owner.PluginStatusTime);
    b.pack((unsigned short)0);
    b.pack((unsigned short)1);
    b.pack((unsigned short)1);
    b.pack((char*)plugins[plugin_id], sizeof(plugin));
    b << (char)1;
    b.pack(status);
    b.pack(data.owner.PluginStatusTime);
    b.pack((unsigned short)0);
    b.pack((unsigned short)0);
    b.pack((unsigned short)1);
    m_socket->writeBuffer.tlv(0x0011, b);
    m_socket->writeBuffer.tlv(0x0012, (unsigned short)0);
    sendPacket();
}

void ICQClient::sendUpdate()
{
    if (m_nUpdates == 0)
        return;
    if (--m_nUpdates)
        return;
    time_t now;
    time(&now);
    data.owner.InfoUpdateTime = now;
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS);
    m_socket->writeBuffer.tlv(0x0006, fullStatus(m_status));
    Buffer directInfo(25);
    fillDirectInfo(directInfo);
    m_socket->writeBuffer.tlv(0x000C, directInfo);
    sendPacket();
}

void ICQClient::fillDirectInfo(Buffer &directInfo)
{
	data.owner.Port = m_listener ? m_listener->port() : 0;
    set_ip(&data.owner.RealIP, m_socket->localHost());
    if (getHideIP()){
        directInfo
        << (unsigned long)0
        << (unsigned long)0;
    }else{
        directInfo
        << (unsigned long)htonl(get_ip(data.owner.RealIP))
        << (unsigned short)0
        << (unsigned short)data.owner.Port;
    }

    char mode = DIRECT_MODE_DIRECT;
    unsigned long ip1 = get_ip(data.owner.IP);
    unsigned long ip2 = get_ip(data.owner.RealIP);
    if (ip1 && ip2 && (ip1 != ip2))
        mode = DIRECT_MODE_INDIRECT;
    switch (m_socket->socket()->mode()){
    case Socket::Indirect:
        mode = DIRECT_MODE_INDIRECT;
        break;
    case Socket::Web:
        mode = DIRECT_MODE_DENIED;
        break;
    default:
	break;
    }
    directInfo
    << mode
    << (char)0x00
    << (char)ICQ_TCP_VERSION;

    directInfo
    << data.owner.DCcookie
    << 0x00000050L
    << 0x00000003L
    << data.owner.InfoUpdateTime
    << data.owner.PluginInfoTime
    << data.owner.PluginStatusTime
    << (unsigned short) 0x0000;
}

void ICQClient::sendIdleTime()
{
    if (getIdleTime() == 0){
        m_bIdleTime = false;
        return;
    }
    time_t now;
    time(&now);
    unsigned long idle = now - getIdleTime();
    if (idle <= 0)
        idle = 1;
    snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxIDLE);
    m_socket->writeBuffer << idle;
    sendPacket();
    m_bIdleTime = true;
}
