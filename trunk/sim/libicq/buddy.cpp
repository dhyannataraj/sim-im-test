/***************************************************************************
                          buddy.cpp  -  description
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
#include <stdio.h>

const unsigned short ICQ_SNACxBDY_REQUESTxRIGHTS   = 0x0002;
const unsigned short ICQ_SNACxBDY_RIGHTSxGRANTED   = 0x0003;
const unsigned short ICQ_SNACxBDY_ADDxTOxLIST      = 0x0004;
const unsigned short ICQ_SNACxBDY_USERONLINE	   = 0x000B;
const unsigned short ICQ_SNACxBDY_USEROFFLINE	   = 0x000C;

void ICQClient::snac_buddy(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxBDY_RIGHTSxGRANTED:
        log(L_DEBUG, "Buddy rights granted");
        break;
    case ICQ_SNACxBDY_USEROFFLINE:{
            unsigned long uin = readBuffer.unpackUin();
            ICQUser *user = getUser(uin);
            if (user && (user->uStatus != ICQ_STATUS_OFFLINE)){
                log(L_DEBUG, "User %lu [%s] offline", uin, user->Alias.c_str());
                user->setOffline();
                ICQEvent e(EVENT_STATUS_CHANGED, uin);
                process_event(&e);
            }
            break;
        }
    case ICQ_SNACxBDY_USERONLINE:{
            unsigned long uin = readBuffer.unpackUin();
            if (uin == Uin()) break;
            ICQUser *user = getUser(uin);
            if (user){
                time_t now;
                time(&now);
                bool changed = false;

                unsigned long cookie1, cookie2, cookie3;
                cookie1 = cookie2 = cookie3 = 0;
                unsigned short level, len;
                readBuffer >> level >> len;
                TlvList tlv(readBuffer);

                // Status TLV
                Tlv *tlvStatus = tlv(0x0006);
                if (tlvStatus){
                    unsigned long status = *tlvStatus;
                    log(L_DEBUG, "User %lu [%s] online [%08lX]", uin, user->Alias.c_str(), status);
                    if (status != user->uStatus){
                        user->prevStatus = user->uStatus;
                        user->uStatus = status;
                        if (status & 0xFF){
                            addResponseRequest(uin);
                        }else{
                            user->AutoReply = "";
                        }
                        user->StatusTime = (unsigned long)now;
                        changed = true;
                    }
                }

                // Online time TLV
                Tlv *tlvOnlineTime = tlv(0x0003);
                if (tlvOnlineTime){
                    user->OnlineTime = (unsigned long)(*tlvOnlineTime);
                    changed = true;
                }
                Tlv *tlvNATime = tlv(0x0004);
                if (tlvNATime){
                    user->StatusTime = (unsigned long)now - (unsigned short)(*tlvNATime) * 60L;
                    changed = true;
                }
                // IP TLV
                Tlv *tlvIP = tlv(0x000A);
                if (tlvIP){
                    unsigned long ip = htonl((unsigned long)(*tlvIP));
                    if (user->IP() != ip) user->HostName = "";
                    user->IP = ip;
                    changed = true;
                }

                // Direct connection info
                Tlv *tlvDirect = tlv(0x000C);
                if (tlvDirect){
                    user->ClientType = 0;
                    Buffer info(*tlvDirect);
                    unsigned long  realIP;
                    unsigned short port;
                    char mode, version, junk;
                    info >> realIP;
                    info.incReadPos(2);
                    info >> port;
                    realIP = htonl(realIP);
                    if (user->RealIP != realIP) user->RealHostName ="";
                    user->RealIP = realIP;
                    user->Port = port;
                    info >> mode >> junk >> version >> user->DCcookie;
                    info.incReadPos(8);
                    info
                    >> cookie1
                    >> cookie2
                    >> cookie3;
                    if (cookie3 != user->PhoneBookTime()){
                        user->bPhoneChanged = true;
                        user->PhoneBookTime = cookie3;
                    }
                    user->PhoneStatusTime = cookie2;
                    user->TimeStamp       = cookie1;
                    if (mode == MODE_DENIED) mode = MODE_INDIRECT;
                    if ((mode != MODE_DIRECT) && (mode != MODE_INDIRECT))
                        mode = MODE_INDIRECT;
                    user->Mode = (unsigned short)mode;
                    user->Version = (unsigned short)version;
                    changed = true;
                    if (user->DCcookie == 0)
                        user->ClientType = 4;
                    if ((user->DCcookie == cookie1) && (cookie1 == cookie2))
                        user->ClientType = 5;
                    if (cookie1 == 0x279c6996)
                        user->ClientType = 3;
                }

                Tlv *tlvCapability = tlv(0x000D);
                if (tlvCapability){
                    user->GetRTF = false;
                    Buffer info(*tlvCapability);
                    for (; info.readPos() < info.size(); ){
                        capability cap;
                        info.unpack((char*)cap, sizeof(capability));
                        if (!memcmp(cap, capabilities[1], sizeof(capability))){
                            user->GetRTF = true;
                            if (!user->CanPlugin){
                                if (user->bPhoneChanged) addPhoneRequest(uin);
                                user->CanPlugin = true;
                            }
                        }
                        if (!memcmp(cap, capabilities[3], sizeof(capability)))
                            user->CanResponse = true;
                        if (!memcmp(cap, capabilities[5], sizeof(capability)))
                            user->ClientType = 2;
                        if (!memcmp(cap, capabilities[4], sizeof(capability)))
                            user->ClientType = 1;
                    }
                }

                Tlv *tlvPlugin = tlv(0x0011);
                if (tlvPlugin){
                    Buffer info(*tlvPlugin);
                    char reqType;
                    info >> reqType;
                    info.incReadPos(10);
                    capability cap;
                    info.unpack((char*)cap, sizeof(capability));
                    if (user->CanPlugin && !memcmp(cap, PHONEBOOK_SIGN, sizeof(capability))){
                        if (reqType == 3){
                            info.incReadPos(3);
                            info >> user->PhoneState;
                            if (user->bPhoneChanged) addPhoneRequest(uin);
                        }else if (reqType == 2)
                            if (user->bPhoneChanged) addPhoneRequest(uin);
                    }
                    addInfoRequest(uin);
                    changed = true;
                }

                if (changed){
                    ICQEvent e(EVENT_STATUS_CHANGED, uin);
                    process_event(&e);
                    user->prevStatus = user->uStatus;
                }
            }
            break;
        }
    default:
        log(L_WARN, "Unknown buddy family type %04X", type);
    }
}

void ICQClient::buddyRequest()
{
    snac(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REQUESTxRIGHTS);
    sendPacket();
}

void ICQClient::sendContactList()
{
    int nBuddies = 0;
    list<ICQUser*>::iterator it;
    for (it = contacts.users.begin(); it != contacts.users.end(); it++){
        if (((*it)->Uin() < UIN_SPECIAL) && !(*it)->inIgnore() &&
                ((*it)->WaitAuth() || ((*it)->GrpId() == 0))) nBuddies++;
    }
    if (nBuddies == 0) return;
    snac(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    for (it = contacts.users.begin(); it != contacts.users.end(); it++){
        if (((*it)->Uin() < UIN_SPECIAL) && !(*it)->inIgnore() &&
                ((*it)->WaitAuth() || ((*it)->GrpId() == 0)))
            writeBuffer.packUin((*it)->Uin());
    }
    sendPacket();
}

void ICQClient::addToContacts(unsigned long uin)
{
    if (m_state != Logged) return;
    if (uin >= UIN_SPECIAL) return;
    snac(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    writeBuffer.packUin(uin);
    sendPacket();
}


