/***************************************************************************
                          icqbuddy.cpp  -  description
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
#include <stdio.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#endif

const unsigned short ICQ_SNACxBDY_REQUESTxRIGHTS   = 0x0002;
const unsigned short ICQ_SNACxBDY_RIGHTSxGRANTED   = 0x0003;
const unsigned short ICQ_SNACxBDY_ADDxTOxLIST      = 0x0004;
const unsigned short ICQ_SNACxBDY_USERONLINE	   = 0x000B;
const unsigned short ICQ_SNACxBDY_USEROFFLINE	   = 0x000C;

void ICQClient::snac_buddy(unsigned short type, unsigned short)
{
    string screen;
    Contact *contact;
    ICQUserData *data;
    switch (type){
    case ICQ_SNACxBDY_RIGHTSxGRANTED:
        log(L_DEBUG, "Buddy rights granted");
        break;
    case ICQ_SNACxBDY_USEROFFLINE:
        screen = m_socket->readBuffer.unpackScreen();
        data = findContact(screen.c_str(), NULL, false, contact);
        if (data && (data->Status != ICQ_STATUS_OFFLINE)){
            setOffline(data);
            StatusMessage m;
            m.setContact(contact->id());
            m.setClient(dataName(data).c_str());
            m.setStatus(STATUS_OFFLINE);
            m.setFlags(MESSAGE_RECEIVED);
            Event e(EventMessageReceived, &m);
            e.process();
        }
        break;
    case ICQ_SNACxBDY_USERONLINE:
        screen = m_socket->readBuffer.unpackScreen();
        data = findContact(screen.c_str(), NULL, false, contact);
        if (data){
            time_t now;
            time(&now);

            bool bChanged = false;
            unsigned long prevStatus = data->Status;

            unsigned short level, len;
            m_socket->readBuffer >> level >> len;
            data->WarningLevel = level;

            TlvList tlv(m_socket->readBuffer);

            // Status TLV
            Tlv *tlvStatus = tlv(0x0006);
            if (tlvStatus){
                unsigned long status = *tlvStatus;
                if (status != data->Status){
                    data->Status = status;
                    if ((status & 0xFF) == 0)
                        set_str(&data->AutoReply, NULL);
                    data->StatusTime = (unsigned long)now;
                }
            }else if (data->Status == ICQ_STATUS_OFFLINE){
                data->Status = ICQ_STATUS_ONLINE;
                data->StatusTime = (unsigned long)now;
            }

            // Online time TLV
            Tlv *tlvOnlineTime = tlv(0x0003);
            if (tlvOnlineTime){
                unsigned long OnlineTime = *tlvOnlineTime;
                if (OnlineTime != data->OnlineTime){
                    data->OnlineTime = OnlineTime;
                    bChanged = true;
                }
            }
            Tlv *tlvNATime = tlv(0x0004);
            if (tlvNATime){
                unsigned short na_time = *tlvNATime;
                unsigned long StatusTime = (unsigned long)now - na_time * 60;
                if (StatusTime != data->StatusTime){
                    data->StatusTime = StatusTime;
                    bChanged = true;
                }
            }

            // IP TLV
            Tlv *tlvIP = tlv(0x000A);
            if (tlvIP)
                bChanged |= set_ip(&data->IP, htonl((unsigned long)(*tlvIP)));

            Tlv *tlvCapability = tlv(0x000D);
            if (tlvCapability){
                data->Caps = 0;
                Buffer info(*tlvCapability);
                for (; info.readPos() < info.size(); ){
                    capability cap;
                    info.unpack((char*)cap, sizeof(capability));
                    for (unsigned i = 0;; i++){
                        if (*capabilities[i] == 0) break;
                        unsigned size = sizeof(capability);
                        if (i == CAP_SIMOLD) size--;
                        if ((i == CAP_MICQ) || (i == CAP_LICQ) || (i == CAP_SIM)) size -= 4;
                        if (!memcmp(cap, capabilities[i], size)){
                            if (i == CAP_SIMOLD){
                                unsigned char build = cap[sizeof(capability)-1];
                                if (build && ((build == 0x92) || (build < (1 << 6)))) continue;
                                data->Build = build;
                            }
                            if ((i == CAP_MICQ) || (i == CAP_LICQ) || (i == CAP_SIM)){
                                unsigned char *p = (unsigned char*)cap;
                                p += 12;
                                data->Build = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
                            }
                            data->Caps |= (1 << i);
                            break;
                        }
                    }
                }
            }

            unsigned long infoUpdateTime = 0;
            unsigned long pluginInfoTime = 0;
            unsigned long pluginStatusTime = 0;

            // Direct connection info
            Tlv *tlvDirect = tlv(0x000C);
            if (tlvDirect){
                Buffer info(*tlvDirect);
                unsigned long  realIP;
                unsigned short port;
                char mode, version, junk;
                info >> realIP;
                info.incReadPos(2);
                info >> port;
                if (realIP == 0x7F000001)
                    realIP = 0;
                bChanged |= set_ip(&data->RealIP, htonl(realIP));
                data->Port = port;
                unsigned long DCcookie;
                info >> mode >> junk >> version >> DCcookie;
                data->DCcookie = DCcookie;
                info.incReadPos(8);
                info
                >> infoUpdateTime
                >> pluginInfoTime
                >> pluginStatusTime;
                if (mode == MODE_DENIED) mode = MODE_INDIRECT;
                if ((mode != MODE_DIRECT) && (mode != MODE_INDIRECT))
                    mode = MODE_INDIRECT;
                data->Mode    = mode;
                data->Version = version;
            }

            Tlv *tlvPlugin = tlv(0x0011);
            if (tlvPlugin && data->Uin){
                Buffer info(*tlvPlugin);
                char type;
                unsigned long time;
                info >> type;
                info.unpack(time);
                plugin p;
                unsigned plugin_index;
                unsigned long plugin_status;
                switch (type){
                case 1:
                    addFullInfoRequest(data->Uin);
                    break;
                case 2:
                    info.incReadPos(6);
                    info.unpack((char*)p, sizeof(p));
                    data->PluginInfoTime = time;
                    for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++)
                        if (!memcmp(p, plugins[plugin_index], sizeof(p)))
                            break;
                    switch (plugin_index){
                    case PLUGIN_PHONEBOOK:
                        log(L_DEBUG, "Updated phonebook");
                        addPluginInfoRequest(data->Uin, plugin_index);
                        break;
                    case PLUGIN_PICTURE:
                        log(L_DEBUG, "Updated picture");
                        addPluginInfoRequest(data->Uin, plugin_index);
                        break;
                    case PLUGIN_QUERYxINFO:
                        log(L_DEBUG, "Updated info plugin list");
                        addPluginInfoRequest(data->Uin, plugin_index);
                        break;
                    default:
                        if (plugin_index >= PLUGIN_NULL)
                            log(L_WARN, "Unknown plugin sign");
                    }
                    break;
                case 3:
                    info.incReadPos(6);
                    info.unpack((char*)p, sizeof(p));
                    info.incReadPos(1);
                    info.unpack(plugin_status);
                    data->PluginStatusTime = time;
                    for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++)
                        if (!memcmp(p, plugins[plugin_index], sizeof(p)))
                            break;
                    switch (plugin_index){
                    case PLUGIN_FOLLOWME:
                        if (data->FollowMe == plugin_status)
                            break;
                        data->FollowMe = plugin_status;
                        bChanged = true;
                        break;
                    case PLUGIN_FILESERVER:
                        if ((data->SharedFiles != 0) == (plugin_status != 0))
                            break;
                        data->SharedFiles = (plugin_status != 0);
                        bChanged = true;
                        break;
                    case PLUGIN_ICQPHONE:
                        if (data->ICQPhone == plugin_status)
                            break;
                        data->ICQPhone = plugin_status;
                        bChanged = true;
                        break;
                    default:
                        if (plugin_index >= PLUGIN_NULL)
                            log(L_WARN, "Unknown plugin sign");
                    }
                    break;

                }
            }else{
                data->InfoUpdateTime   = infoUpdateTime;
                data->PluginInfoTime   = pluginInfoTime;
                data->PluginStatusTime = pluginStatusTime;
                if (getAutoUpdate()){
                    if (infoUpdateTime == 0)
                        infoUpdateTime = 1;
                    if (infoUpdateTime != data->InfoFetchTime)
                        addFullInfoRequest(data->Uin);
                    if ((data->PluginInfoTime != data->PluginInfoFetchTime)){
                        if (data->PluginInfoTime)
                            addPluginInfoRequest(data->Uin, PLUGIN_QUERYxINFO);
                    }
                    if ((data->PluginInfoTime != data->PluginInfoFetchTime) ||
                            (data->PluginStatusTime != data->PluginStatusFetchTime)){
                        if (data->SharedFiles != 0){
                            data->SharedFiles = 0;
                            bChanged = true;
                        }
                        if (data->FollowMe != 0){
                            data->FollowMe = 0;
                            bChanged = true;
                        }
                        if (data->ICQPhone != 0){
                            data->ICQPhone = 0;
                            bChanged = true;
                        }
                        if (data->PluginStatusTime)
                            addPluginInfoRequest(data->Uin, PLUGIN_QUERYxSTATUS);
                    }
                }
            }
            if (data->bInvisible){
                data->bInvisible = false;
                bChanged = true;
            }
            if (bChanged){
                Event e(EventContactChanged, contact);
                e.process();
            }
            if (data->Status != prevStatus){
                unsigned status = STATUS_OFFLINE;
                if ((data->Status & 0xFFFF) != ICQ_STATUS_OFFLINE){
                    status = STATUS_ONLINE;
                    if (data->Status & ICQ_STATUS_DND){
                        status = STATUS_DND;
                    }else if (data->Status & ICQ_STATUS_OCCUPIED){
                        status = STATUS_OCCUPIED;
                    }else if (data->Status & ICQ_STATUS_NA){
                        status = STATUS_NA;
                    }else if (data->Status & ICQ_STATUS_AWAY){
                        status = STATUS_AWAY;
                    }else if (data->Status & ICQ_STATUS_FFC){
                        status = STATUS_FFC;
                    }
                }
                StatusMessage m;
                m.setContact(contact->id());
                m.setClient(dataName(data).c_str());
                m.setStatus(status);
                m.setFlags(MESSAGE_RECEIVED);
                Event e(EventMessageReceived, &m);
                e.process();
                if (!contact->getIgnore() &&
                        ((data->Status & 0xFF) == ICQ_STATUS_ONLINE) &&
                        ((prevStatus & 0xFF) != ICQ_STATUS_ONLINE) &&
                        (((prevStatus & 0xFFFF) != ICQ_STATUS_OFFLINE) ||
                         (data->OnlineTime > this->data.owner.OnlineTime))){
                    Event e(EventContactOnline, contact);
                    e.process();
                }
                if (getAutoReplyUpdate() && ((data->Status & 0xFF) != ICQ_STATUS_ONLINE)){
                    if ((getInvisible() && data->VisibleId) ||
                            (!getInvisible() && (data->InvisibleId == 0)))
                        addPluginInfoRequest(data->Uin, PLUGIN_AR);
                }
            }
        }
        break;
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
    buddies.clear();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ClientDataIterator it_data(contact->clientData, this);
        ICQUserData *data;
        while ((data = (ICQUserData*)(++it_data)) != NULL){
            if ((data->IgnoreId == 0)  && (data->WaitAuth || (data->GrpId == 0)))
                buddies.push_back(screen(data));
        }
    }
    if (buddies.empty()) return;
    snac(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
    it.reset();
    while ((contact = ++it) != NULL){
        ClientDataIterator it_data(contact->clientData, this);
        ICQUserData *data;
        while ((data = (ICQUserData*)(++it_data)) != NULL){
            if ((data->IgnoreId == 0)  && (data->WaitAuth || (data->GrpId == 0)))
                m_socket->writeBuffer.packScreen(screen(data).c_str());
        }
    }
    sendPacket();
}

void ICQClient::addBuddy(Contact *contact)
{
    if (getState() != Connected)
        return;
    if (contact->id() == 0)
        return;
    ICQUserData *data;
    ClientDataIterator it_data(contact->clientData, this);
    while ((data = (ICQUserData*)(++it_data)) != NULL){
        list<string>::iterator it;
        for (it = buddies.begin(); it != buddies.end(); ++it){
            if (screen(data) == *it)
                break;
        }
        if (it != buddies.end())
            continue;
        if ((data->IgnoreId == 0)  && (data->WaitAuth || (data->GrpId == 0))){
            snac(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST);
            m_socket->writeBuffer.packScreen(screen(data).c_str());
            sendPacket();
            buddies.push_back(screen(data));
        }
    }
}
