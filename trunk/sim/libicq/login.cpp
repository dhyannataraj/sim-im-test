/***************************************************************************
                          login.cpp  -  description
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

#include <stdio.h>
#include <time.h>

#define RATE_LIMIT_TIMEOUT 600

const unsigned short ICQ_SNACxLOGIN_ERROR	 = 0x0001;
const unsigned short ICQ_SNACxLOGIN_REGISTERxREQ = 0x0004;
const unsigned short ICQ_SNACxLOGIN_REGISTER     = 0x0005;

void ICQClient::snac_login(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxLOGIN_ERROR:
        m_reconnectTime = (time_t)(-1);
        log(L_WARN, "Login error");
        sock->error_state(ErrorProtocol);
        break;
    case ICQ_SNACxLOGIN_REGISTER:{
            if (m_state != Register){
                log(L_WARN, "Registered in no register state");
                break;
            }
            sock->readBuffer.incReadPos(0x2E);
            unsigned long newUin;
            sock->readBuffer.unpack(newUin);
            log(L_DEBUG, "Register %u %08lX", newUin, newUin);
            owner->Uin = newUin;
            ICQEvent e(EVENT_INFO_CHANGED);
            process_event(&e);
            m_state = Connect;
            sock->setProxy(getProxy());
            sock->connect(ServerHost.c_str(), ServerPort);
            break;
        }
    default:
        log(L_WARN, "Unknown login family type %04X", type);
    }
}

void ICQClient::chn_login()
{
    switch (m_state){
    case Connect:{
            log(L_DEBUG, "Login %lu [%s]", owner->Uin, EncryptedPassword.c_str());
            char uin[20];
            sprintf(uin, "%lu", owner->Uin);

            flap(ICQ_CHNxNEW);
            int n = 0;
            char pass[16];
            for (const char *p = EncryptedPassword.c_str(); *p && n < 16; p++){
                if (*p != '\\'){
                    pass[n++] = *p;
                    continue;
                }
                char c = *(++p);
                if (c == '0') c = 0;
                pass[n++] = c;
            }

            sock->writeBuffer << 0x00000001L;
            sock->writeBuffer.tlv(0x0001, uin);
            sock->writeBuffer.tlv(0x0002, pass, n);
            sock->writeBuffer.tlv(0x0003, "ICQ Inc. - Product of ICQ (TM).2001b.5.17.1.3642.85");
            sock->writeBuffer.tlv(0x0016, 0x010A);
            sock->writeBuffer.tlv(0x0017, 0x0005);
            sock->writeBuffer.tlv(0x0018, 0x0011);
            sock->writeBuffer.tlv(0x0019, 0x0001);
            sock->writeBuffer.tlv(0x001A, 0x0E3A);
            sock->writeBuffer.tlv(0x0014, 0x00000055L);
            sock->writeBuffer.tlv(0x000e, "us");
            sock->writeBuffer.tlv(0x000f, "en");
            sendPacket();
            break;
        }
    case Register:
        {
            flap(ICQ_CHNxNEW);
            sock->writeBuffer << 0x00000001L;
            sendPacket();
            snac(ICQ_SNACxFAM_LOGIN, ICQ_SNACxLOGIN_REGISTERxREQ);
            Buffer msg;
            msg
            << 0x00000000L << 0x28000300L << 0x00000000L
            << 0x00000000L << 0x94680000L << 0x94680000L
            << 0x00000000L << 0x00000000L << 0x00000000L
            << 0x00000000L;
            unsigned short len = DecryptedPassword.size() + 1;
            msg.pack(len);
            msg.pack(DecryptedPassword.c_str(), len);
            msg << 0x94680000L << 0x00000602L;
            sock->writeBuffer.tlv(0x0001, msg);
            sendPacket();
            break;
        }
    default:
        break;
    }
}

void ICQClient::chn_close()
{
    TlvList tlv(sock->readBuffer);
    Tlv *tlv_error = tlv(8);
    if (tlv_error){
        unsigned short err = *tlv_error;
        if (err)
            if (sock) sock->error_state(ErrorProtocol);
        switch (err){
        case 0x18:{
                log(L_WARN, "Rate limit");
                time(&m_reconnectTime);
                m_reconnectTime += RATE_LIMIT_TIMEOUT;
                ICQEvent e(EVENT_RATE_LIMIT);
                process_event(&e);
                break;
            }
        case 0x05:{
                ICQEvent e(EVENT_BAD_PASSWORD);
                process_event(&e);
                log(L_WARN, "Invalid UIN and password combination");
                m_reconnectTime = (time_t)(-1);
                break;
            }
        case 0:
            break;
        default:
            ICQEvent e(EVENT_LOGIN_ERROR);
            process_event(&e);
            log(L_WARN, "Unknown error %04X", err);
        }
        if (err) return;
    }
    tlv_error = tlv(9);
    if (tlv_error){
        unsigned short err = *tlv_error;
        if (err)
            if (sock) sock->error_state(ErrorProtocol);
        switch (err){
        case 0x1:{
                log(L_WARN, "Your ICQ number is used from another location");
                m_reconnectTime = (time_t)(-1);
                ICQEvent e(EVENT_ANOTHER_LOCATION);
                process_event(&e);
                break;
            }
        case 0:
            break;
        default:
            log(L_WARN, "Unknown run-time error %04X", err);
        }
        if (err) return;
    }

    Tlv *tlv_host = tlv(5);
    Tlv *tlv_cookie = tlv(6);
    if ((tlv_host == NULL) || (tlv_cookie == NULL)){
        log(L_ERROR, "Close packet");
        sock->error_state(ErrorProtocol);
        return;
    }
    char *host = *tlv_host;
    char *port = strchr(host, ':');
    if (port == NULL){
        log(L_ERROR, "Bad host address %s", host);
        sock->error_state(ErrorProtocol);
        return;
    }

    *port = 0;
    port++;
    m_state = Login;
    sock->close();
    Socket *s = sock->socket();
    sock->setSocket(createSocket());
    sock->setProxy(getProxy());
    sock->connect(host, atol(port));
    delete s;
    cookie.init(0);
    cookie.pack(*tlv_cookie, tlv_cookie->Size());
}

