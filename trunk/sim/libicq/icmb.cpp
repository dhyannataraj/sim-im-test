/***************************************************************************
                          icmb.cpp  -  description
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
#include "icqprivate.h"
#include "log.h"

#include <stdio.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <time.h>

const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002;
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004;
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005;
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006;
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007;
const unsigned short ICQ_SNACxMSG_AUTOREPLY        = 0x000B;
const unsigned short ICQ_SNACxMSG_ACK              = 0x000C;

const unsigned char ICQClientPrivate::PHONEBOOK_SIGN[16] =
    {
        0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D, 0xD3, 0x11,
        0xAD, 0xEB, 0x00, 0x04, 0xAC, 0x96, 0xAA, 0xB2
    };

const unsigned char ICQClientPrivate::PLUGINS_SIGN[16] =
    {
        0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71, 0xD3, 0x11,
        0x8D, 0xD2, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E
    };

const unsigned char ICQClientPrivate::SHARED_FILES_SIGN[16] =
    {
        0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91, 0xD3, 0x11,
        0x8D, 0xD7, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E
    };


void ICQClientPrivate::snac_message(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxMSG_RIGHTSxGRANTED:
        log(L_DEBUG, "Message rights granted");
        break;
    case ICQ_SNACxMSG_ACK:{
            log(L_DEBUG, "Ack message");
            unsigned long timestamp1, timestamp2;
            sock->readBuffer >> timestamp1 >> timestamp2;
            for (list<ICQEvent*>::iterator it = msgQueue.begin(); it != msgQueue.end();){
                ICQEvent *e = *it;
                if ((e->message() == NULL) || (e->state != ICQEvent::Send)) continue;
                ICQMessage *m = e->message();
                if ((m->timestamp1 != timestamp1) != (m->timestamp2 != timestamp2)) continue;
                if (m->Type() != ICQ_MSGxCHATxINFO){
                    time_t now;
                    time(&now);
                    m->Time = now;
                    e->state = ICQEvent::Success;
                    msgQueue.remove(e);
                    client->process_event(e);
                }
                break;
            }
            break;
        }
    case ICQ_SNACxMSG_AUTOREPLY:{
            unsigned long timestamp1, timestamp2;
            sock->readBuffer >> timestamp1 >> timestamp2;
            sock->readBuffer.incReadPos(2);
            unsigned long uin = sock->readBuffer.unpackUin();
            sock->readBuffer.incReadPos(6);
            unsigned long t1, t2;
            sock->readBuffer >> t1 >> t2;
            if ((t1 == 0x60F1A83DL) && (t2 == 0x9149D311L)){
                for (list<ICQEvent*>::iterator it = msgQueue.begin(); it != msgQueue.end();){
                    ICQEvent *e = *it;
                    if ((e->message() == NULL) || (e->state != ICQEvent::Send)) continue;
                    ICQMessage *m = e->message();
                    if ((m->timestamp1 != timestamp1) != (m->timestamp2 != timestamp2)) continue;
                    if (m->Type() == ICQ_MSGxCHATxINFO){
                        ICQChatInfo *chatInfo = static_cast<ICQChatInfo*>(m);
                        e->state = ICQEvent::Success;
                        sock->readBuffer.incReadPos(0x32);
                        sock->readBuffer.unpack(chatInfo->name);
                        client->fromServer(chatInfo->name, client->owner);
                        sock->readBuffer.unpack(chatInfo->topic);
                        client->fromServer(chatInfo->topic, client->owner);
                        sock->readBuffer.unpack(chatInfo->age);
                        sock->readBuffer.unpack(chatInfo->gender);
                        sock->readBuffer.unpack(chatInfo->language);
                        sock->readBuffer.unpack(chatInfo->country);
                        sock->readBuffer.unpack(chatInfo->homepage);
                        client->fromServer(chatInfo->homepage, client->owner);
                        msgQueue.remove(e);
                        m->bDelete = true;
                        client->process_event(e);
                        if (m->bDelete) delete m;
                        delete e;
                    }
                    break;
                }
                return;
            }
            unsigned short seq;
            sock->readBuffer.incReadPos(0x0F);
            sock->readBuffer >> seq;
            ICQUser *u = client->getUser(uin);
            if ((t1 == 0) && (t2 == 0)){
                sock->readBuffer.incReadPos(0x16);
                string answer;
                sock->readBuffer >> answer;
                client->fromServer(answer, u);
                if (timestamp1 || timestamp2){
                    log(L_DEBUG, "Message declined %s", answer.c_str());
                    list<ICQEvent*>::iterator it;
                    for (it = processQueue.begin(); it != processQueue.end(); ++it){
                        ICQEvent *e = *it;
                        if (e->type() != EVENT_MESSAGE_SEND) continue;
                        ICQMessage *msg = e->message();
                        if (msg == NULL) continue;
                        if ((msg->getUin() == uin) && (msg->timestamp1 == timestamp1) && (msg->timestamp2 == timestamp2)){
                            msg->DeclineReason = answer;
                            client->cancelMessage(msg, false);
                            break;
                        }
                    }
                    if (it == processQueue.end())
                        log(L_WARN, "Decline file answer: message not found");
                }else{
                    log(L_DEBUG, "[%X] Autoreply from %u %s", seq, uin, answer.c_str());
                    ICQUser *u = client->getUser(uin);
                    if (u) u->AutoReply = answer;
                    ICQEvent e(EVENT_INFO_CHANGED, uin, EVENT_SUBTYPE_AUTOREPLY);
                    client->process_event(&e);
                    processResponseRequestQueue(seq);
                }
            }
            if ((t1 == 0xA0E93F37L) && (t2 == 0x4FE9D311L)){
                ICQUser *u = client->getUser(uin);
                if (u == NULL){
                    log(L_WARN, "Request info no my user %lu", uin);
                    return;
                }
                if (u->IgnoreId){
                    log(L_WARN, "Request info ignore user %lu", uin);
                    return;
                }
                if (client->owner->InvisibleId && (u->VisibleId == 0)){
                    log(L_WARN, "Request info in invisible from user %lu", uin);
                    return;
                }
                if ((client->owner->InvisibleId == 0) && u->InvisibleId){
                    log(L_WARN, "Request info from invisible user %lu", uin);
                    return;
                }

                sock->readBuffer.incReadPos(0x1D);
                unsigned long cookie;
                sock->readBuffer.unpack(cookie);
                sock->readBuffer.incReadPos(4);
                sock->readBuffer.unpack(t1);
                if (t1 == 3){
                    u->PhoneBookTime = cookie;
                    u->bPhoneChanged = false;
                    log(L_DEBUG, "[%X] Phone book info %u", seq, uin);
                    PhoneBook::iterator it;
                    PhoneBook myNumbers;
                    for (it = u->Phones.begin(); it != u->Phones.end(); ++it){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        if (!phone->MyInfo) continue;
                        PhoneInfo *myPhone = new PhoneInfo;
                        *myPhone = *phone;
                        myNumbers.push_back(myPhone);
                    }
                    u->Phones.clear();
                    unsigned long nPhones;
                    sock->readBuffer.unpack(nPhones);
                    for (unsigned i = 0; i < nPhones; i++){
                        PhoneInfo *phone = new PhoneInfo;
                        u->Phones.push_back(phone);
                        sock->readBuffer.unpackStr32(phone->Name);
                        sock->readBuffer.unpackStr32(phone->AreaCode);
                        sock->readBuffer.unpackStr32(phone->Number);
                        sock->readBuffer.unpackStr32(phone->Extension);
                        sock->readBuffer.unpackStr32(phone->Country);
                        unsigned long type;
                        sock->readBuffer.unpack(type);
                        if (type) phone->Active = true;
                        if (sock->readBuffer.readPos() >= sock->readBuffer.size()) break;
                    }
                    for (it = u->Phones.begin(); it != u->Phones.end(); it++){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        string prop;
                        sock->readBuffer.unpackStr32(prop);
                        Buffer b;
                        b.pack(prop.c_str(), prop.length());
                        b.unpack(phone->Type);
                        b.unpackStr32(phone->Provider);
                        if (sock->readBuffer.readPos() >= sock->readBuffer.size()) break;
                    }
                    for (;;){
                        for (it = u->Phones.begin(); it != u->Phones.end(); it++){
                            PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                            bool bOK = (phone->getNumber().length() > 0);
                            if (bOK && !*phone->Number.c_str()) bOK = false;
                            if (bOK)
                                for (const char *p = phone->Number.c_str(); *p; p++){
                                    if ((*p >= '0') && (*p <= '9')) continue;
                                    if ((*p == ' ') || (*p == '-')) continue;
                                    bOK = false;
                                    break;
                                }
                            if (bOK) continue;
                            u->Phones.remove(phone);
                            break;
                        }
                        if (it == u->Phones.end()) break;
                    }
                    u->adjustPhones();
                    u->Phones.add(myNumbers);
                    ICQEvent e(EVENT_INFO_CHANGED, uin);
                    client->process_event(&e);
                    processPhoneRequestQueue(seq);
                }
            }
            break;
        }
    case ICQ_SNACxMSG_SERVERxMESSAGE:{
            unsigned long timestamp1, timestamp2;
            sock->readBuffer >> timestamp1 >> timestamp2;
            unsigned short mFormat;
            sock->readBuffer >> mFormat;
            unsigned long uin = sock->readBuffer.unpackUin();
            log(L_DEBUG, "Message from %u [%04X]", uin, mFormat);
            if (uin == 0xA){
                sock->readBuffer.incReadPos(14);
                string message;
                sock->readBuffer >> message;
                vector<string> l;
                if (!parseFE(message.c_str(), l, 6)){
                    log(L_WARN, "Parse error web panel message");
                    break;
                }
                char SENDER_IP[] = "Sender IP:";
                if ((l[5].size() > strlen(SENDER_IP)) && !memcmp(l[5].c_str(), SENDER_IP, strlen(SENDER_IP))){
                    ICQWebPanel *m = new ICQWebPanel;
                    client->fromServer(l[0], client->owner);
                    client->fromServer(l[3], client->owner);
                    m->Name = l[0];
                    m->Email = l[3];
                    parseMessageText(l[5], m->Message, client->owner);
                    m->Uin.push_back(client->contacts.findByEmail(m->Name, m->Email));
                    messageReceived(m);
                }else{
                    ICQEmailPager *m = new ICQEmailPager;
                    client->fromServer(l[0], client->owner);
                    client->fromServer(l[3], client->owner);
                    m->Name = l[0];
                    m->Email = l[3];
                    parseMessageText(l[5], m->Message, client->owner);
                    m->Uin.push_back(client->contacts.findByEmail(m->Name, m->Email));
                    messageReceived(m);
                }
                break;
            }
            if (uin == 1002)
                log(L_DEBUG, "Incoming SMS");
            unsigned short level, nTLV;
            sock->readBuffer >> level >> nTLV;
            switch (mFormat){
            case 0x0001:{
                    TlvList tlv(sock->readBuffer);
                    if (!tlv(2)){
                        log(L_WARN, "No found generic message tlv");
                        break;
                    }
                    Buffer msg(*tlv(2));
                    TlvList tlv_msg(msg);
                    Tlv *m_tlv = tlv_msg(0x101);
                    if (m_tlv == NULL){
                        log(L_WARN, "No found generic message tlv 101");
                        break;
                    }
                    if (m_tlv->Size() < 4){
                        log(L_WARN, "Bad tlv 101 size");
                        break;
                    }
                    ICQMsg *m = new ICQMsg();
                    ICQUser *u = client->getUser(uin);
                    m->Uin.push_back(uin);
                    m->Received = true;
                    char *m_data = (*m_tlv);
                    string packet;
                    bool isUTF = false;
                    if (m_tlv->Size() > 4){
                        unsigned short encoding = (m_data[0] << 8) + m_data[1];
                        m_data += 4;
                        if (encoding == 2){
                            for (unsigned i = 0; i < m_tlv->Size() - 5; i += 2){
                                unsigned char r1 = *(m_data++);
                                unsigned char r2 = *(m_data++);
                                unsigned short c = (r1 << 8) + r2;
                                utf16to8(c, packet);
                            }
                            isUTF = true;
                            m->Charset = "utf-8";
                        }else{
                            packet.append(m_tlv->Size() - 4, '\x00');
                            memcpy((char*)(packet.c_str()), m_data, m_tlv->Size() - 4);
                        }
                    }
                    parseMessageText(packet, m->Message, u, false, isUTF);
                    messageReceived(m);
                    break;
                }
            case 0x0002:{
                    TlvList tlv(sock->readBuffer);
                    if (!tlv(5)){
                        log(L_WARN, "No found ICMB message tlv");
                        break;
                    }
                    Buffer msg(*tlv(5));
                    unsigned short type;
                    msg >> type;
                    switch (type){
                    case 0:
                        parseAdvancedMessage(uin, msg, tlv(3) != NULL, timestamp1, timestamp2);
                        break;
                    case 1:{
                            log(L_DEBUG, "Abort request");
                            list<ICQEvent*>::iterator it;
                            for (it = processQueue.begin(); it != processQueue.end(); ++it){
                                ICQEvent *e = *it;
                                if ((e->type() != EVENT_MESSAGE_RECEIVED) || (e->message() == NULL))
                                    continue;
                                ICQMessage *m = e->message();
                                if ((m->getUin() == uin) && (m->timestamp1 == timestamp1) && (m->timestamp2 == timestamp2)){
                                    client->cancelMessage(m, false);
                                    break;
                                }
                            }
                            if (it == processQueue.end())
                                log(L_WARN, "Event for cancel not found");
                            break;
                        }
                    case 2:{
                            log(L_DEBUG, "File ack");
                            break;
                        }
                    default:
                        log(L_WARN, "Unknown type: %u", type);
                    }
                    break;
                }
            case 0x0004:{
                    TlvList tlv(sock->readBuffer);
                    if (!tlv(5)){
                        log(L_WARN, "No found advanced message tlv");
                        break;
                    }
                    Buffer msg(*tlv(5));
                    unsigned long msg_uin;
                    msg >> msg_uin;
                    if (msg_uin == 0){
                        parseAdvancedMessage(uin, msg, tlv(6) != NULL, timestamp1, timestamp2);
                        return;
                    }
                    char type, flags;
                    msg >> type;
                    msg >> flags;
                    string msg_str;
                    msg >> msg_str;
                    ICQMessage *m = parseMessage(type, uin, msg_str, msg, 0, 0, timestamp1, timestamp2);
                    if (m){
                        m->Received = true;
                        messageReceived(m);
                    }
                    break;
                }
            default:
                log(L_WARN, "Unknown message format %04X", mFormat);
            }
            break;
        }
    default:
        log(L_WARN, "Unknown message family type %04X", type);
    }
}

void ICQClientPrivate::parseAdvancedMessage(unsigned long uin, Buffer &msg, bool needAck, unsigned long timestamp1, unsigned long timestamp2)
{
    msg.incReadPos(24);
    TlvList tlv(msg);
    if (!tlv(0x2711)){
        log(L_WARN, "No found body in ICMB message");
        return;
    }
    Buffer payload(*tlv(0x2711));
    unsigned short sign;
    payload.unpack(sign);
    if (sign != 0x1B){
        log(L_WARN, "Bad start TLV 2711");
        return;
    }
    payload.unpack(sign);
    char info[18];
    payload.unpack(info, sizeof(info));
    unsigned long sign1;
    payload.unpack(sign1);
    if ((sign1 != 3) && (sign1 != 0) && (sign1 != 1)){
        log(L_WARN, "Bad sign1 in TLV 2711 (%u)", sign1);
        return;
    }
    char sign2;
    payload >> sign2;
    if (sign2 && (sign2 != 1) && (sign2 != 2) && (sign2 != 4)){
        log(L_WARN, "Bad sign2 in TLV 2711 (%u)", sign2);
        return;
    }
    unsigned short cookie1;
    unsigned short cookie2;
    unsigned short cookie3;
    payload.unpack(cookie1);
    payload.unpack(cookie2);
    payload.unpack(cookie3);
    if ((cookie1 != cookie3) && (cookie1 + 1 != cookie3)){
        log(L_WARN, "Bad cookie in TLV 2711 (%X %X %X)", cookie1, cookie2, cookie3);
        return;
    }
    payload.incReadPos(12);
    unsigned char msgType, msgFlags;
    payload >> msgType >> msgFlags;
    unsigned long msgState;
    payload >> msgState;
    string response;
    unsigned short response_type = 0;
    Buffer copy;
    ICQUser *u = client->getUser(uin);
    switch (msgType){
    case 0xE8:
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:{
            ICQMessage *msg = new ICQAutoResponse;
            msg->setType(ICQ_READxAWAYxMSG);
            msg->Uin.push_back(uin);
            switch (msgType){
            case 0xE9:
                msg->setType(ICQ_READxOCCUPIEDxMSG);
                break;
            case 0xEA:
                msg->setType(ICQ_READxNAxMSG);
                break;
            case 0xEB:
                msg->setType(ICQ_READxDNDxMSG);
                break;
            case 0xEC:
                msg->setType(ICQ_READxFFCxMSG);
                break;
            }
            msg->timestamp1 = timestamp1;
            msg->timestamp2 = timestamp2;
            msg->id1 = cookie1;
            msg->id2 = cookie2;
            messageReceived(msg);
            return;
        }
    default:
        string msg;
        unsigned long real_ip = 0;
        unsigned long ip = 0;
        unsigned short port = 0;
        if (tlv(3)) real_ip = htonl((unsigned long)(*tlv(3)));
        if (tlv(4)) ip = htonl((unsigned long)(*tlv(4)));
        log(L_DEBUG, "IP: %X %X", ip, real_ip);
        payload >> msg;
        if (*msg.c_str() || (msgType == ICQ_MSGxEXT)){
            if (payload.readPos() < payload.writePos())
                copy.pack(payload.Data(payload.readPos()), payload.writePos() - payload.readPos());
            log(L_DEBUG, "Msg size=%u type=%u", msg.size(), msgType);
            if (msg.size() || (msgType == ICQ_MSGxEXT)){
                ICQMessage *m = parseMessage(msgType, uin, msg, payload, cookie1, cookie2, timestamp1, timestamp2);
                if (m && (msgState == 0)){
                    port = m->id1;
                    if (port && (ip || real_ip)){
                        struct in_addr a;
                        a.s_addr = real_ip;
                        log(L_DEBUG, "Accept message on %s:%u", inet_ntoa(a), port);
                        list<ICQEvent*>::iterator it;
                        for (it = processQueue.begin(); it != processQueue.end(); ++it){
                            ICQMessage *msg = (*it)->message();
                            if (((*it)->type() != EVENT_MESSAGE_SEND) ||
                                    (msg == NULL) ||
                                    (msg->timestamp1 != timestamp1) ||
                                    (msg->timestamp2 != timestamp2)) {
                                log(L_DEBUG, "Accept message found");
                                continue;
                            }

                            Buffer b;
                            msg_id id;
                            id.l = m->timestamp1;
                            id.h = m->timestamp2;
                            b << (unsigned short)2;
                            b << m->timestamp1 << m->timestamp2
                            << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
                            sendThroughServer(m->getUin(), 2, b, &id, false);

                            if (u == NULL){
                                log(L_WARN, "User %lu not found", m->getUin());
                                client->cancelMessage(m, false);
                                return;
                            }

                            switch (msg->Type()){
                            case ICQ_MSGxFILE:{
                                    ICQFile *file = static_cast<ICQFile*>(msg);
                                    file->ft = new FileTransfer(ip, real_ip, port, u, this, file);
                                    file->ft->connect();
                                    break;
                                }
                            case ICQ_MSGxCHAT:{
                                    log(L_DEBUG, "Chat port %u", port);
                                    ICQChat *chat = static_cast<ICQChat*>(msg);
                                    chat->chat = new ChatSocket(ip, real_ip, port, u, this, chat);
                                    chat->chat->connect();
                                    break;
                                }
                            default:
                                log(L_WARN, "Unknown accept message type");
                                client->cancelMessage(msg, false);
                                return;
                            }
                            ICQEvent e(EVENT_ACKED, m->getUin());
                            e.msg = msg;
                            client->process_event(&e);
                            break;
                        }
                        if (it == processQueue.end())
                            log(L_WARN, "Accept message not found event");
                        if (u){
                            bool bChanged = false;
                            if (real_ip && (u->RealIP != real_ip)){
                                u->RealIP = real_ip;
                                u->RealHostName = "";
                                bChanged = true;
                            }
                            if (ip && (u->IP != ip)){
                                u->IP = ip;
                                u->HostName = "";
                                bChanged = true;
                            }
                            if (port && (u->Port != port)){
                                u->Port = port;
                                bChanged = true;
                                bChanged = true;
                            }
                            log(L_DEBUG, "set IP");
                            if (bChanged){
                                ICQEvent e(EVENT_STATUS_CHANGED, u->Uin);
                                client->process_event(&e);
                            }
                        }
                        delete m;
                        m = NULL;
                        needAck = false;
                    }
                }
                if (m){
                    switch (m->Type()){
                    case ICQ_MSGxEXT:
                    case ICQ_MSGxFILE:
                    case ICQ_MSGxCHAT:
                        needAck = false;
                    }
                    m->Received = true;
                    if (u){
                        bool bChanged = false;
                        if (real_ip && (u->RealIP != real_ip)){
                            u->RealIP = real_ip;
                            u->RealHostName = "";
                            bChanged = true;
                        }
                        if (ip && (u->IP != ip)){
                            u->IP = ip;
                            u->HostName = "";
                            bChanged = true;
                        }
                        if (port && (u->Port != port)){
                            u->Port = port;
                            bChanged = true;
                            bChanged = true;
                        }
                        log(L_DEBUG, "set IP");
                        if (bChanged){
                            ICQEvent e(EVENT_STATUS_CHANGED, u->Uin);
                            client->process_event(&e);
                        }
                    }
                    messageReceived(m);
                }else{
                    needAck = false;
                }
            }else{
                needAck = false;
            }
        }else{
            if (u && (u->IgnoreId == 0)){
                char b[16];
                payload.unpack(b, sizeof(b));
                if (!memcmp(b, PLUGINS_SIGN, sizeof(b))){
                    msgType = 0;
                    log(L_DEBUG, "Plugins request");
                    response_type = 1;
                    copy << 0x00000100L << 0x07DF463CL;
                    Buffer buf;
                    buf << 0x02000100L << 0x01000000L;
                    buf.pack((char*)PHONEBOOK_SIGN, sizeof(PHONEBOOK_SIGN));
                    buf << 0x000000L << (unsigned short)0x0100;
                    buf.packStr32("Phone Book");
                    buf.packStr32("Phone Book / Phone \"Follow Me\"");
                    buf << 0x00000000L;
                    buf << 0x00000000L;
                    copy.pack32(buf);
                }else if (!memcmp(b, PHONEBOOK_SIGN, sizeof(b))){
                    msgType = 0;
                    msgState = 0x0200;
                    log(L_DEBUG, "Phonebook request");
                    response_type = 1;
                    copy << 0x00000100L;
                    copy.pack(client->owner->PhoneBookTime);

                    Buffer b;
                    b << 0x03000000L;
                    b.pack((unsigned long)(client->owner->Phones.size()));
                    PhoneBook::iterator it;
                    for (it = client->owner->Phones.begin(); it != client->owner->Phones.end(); it++){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        b.packStr32(phone->Name.c_str());
                        b.packStr32(phone->AreaCode.c_str());
                        b.packStr32(phone->Number.c_str());
                        b.packStr32(phone->Extension.c_str());
                        b.packStr32(phone->Country.c_str());
                        if (phone->Active){
                            b << 0x01000000L;
                        }else{
                            b << 0x00000000L;
                        }
                    }
                    for (it = client->owner->Phones.begin(); it != client->owner->Phones.end(); it++){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        Buffer b1;
                        b1.pack((unsigned long)(phone->Type));
                        b1.packStr32(phone->Provider.c_str());
                        b1.pack(0x00000001L);
                        b1.pack((unsigned long)((phone->Type == SMS) ? 1 : 0));
                        b1.pack(0x00000001L);
                        b1.pack(0x00000002L);
                        b.pack32(b1);
                    }
                    copy.pack32(b);
                }else{
                    log(L_DEBUG, "Unknown info request %lu", uin);
                    needAck = false;
                }
            }else{
                log(L_DEBUG, "User %u not found or ignore", uin);
                needAck = false;
            }
        }
    }
    if (!needAck) return;
    sendAutoReply(uin, timestamp1, timestamp2, info, cookie1, cookie2,
                  msgType, 0, 0, response, 0, copy);
}

void ICQClientPrivate::sendAutoReply(unsigned long uin, unsigned long timestamp1, unsigned long timestamp2,
                                     char info[18], unsigned short cookie1, unsigned short cookie2,
                                     unsigned char msgType, unsigned char msgFlags, unsigned long msgState,
                                     string response, unsigned short response_type, Buffer &copy)
{
    ICQUser *u = client->getUser(uin);
    if (u == NULL) return;

    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
    sock->writeBuffer << timestamp1 << timestamp2 << 0x0002;
    sock->writeBuffer.packUin(uin);
    sock->writeBuffer << 0x0003 << 0x1B00 << 0x0800;
    sock->writeBuffer.pack(info, 18);
    sock->writeBuffer
    << 0x03000000L << (char)0;
    sock->writeBuffer.pack(cookie1);
    sock->writeBuffer.pack(cookie2);
    sock->writeBuffer.pack(cookie1);
    sock->writeBuffer
    << 0x00000000L << 0x00000000L << 0x00000000L
    << (char)msgType << (char)msgFlags << msgState;
    if (response.size()){
        client->toServer(response, u);
        sock->writeBuffer.pack((unsigned short)(response.size() + 1));
        sock->writeBuffer << response.c_str();
        sock->writeBuffer << (char)0;
    }else{
        sock->writeBuffer << (char)0x01 << response_type;
        if (response_type != 3){
            if (copy.size()){
                sock->writeBuffer.pack(copy.Data(0), copy.writePos());
            }else{
                sock->writeBuffer << 0x00000000L << 0xFFFFFF00L;
            }
        }
    }
    sendPacket();
}

void ICQClient::declineMessage(ICQMessage *m, const char *reason)
{
    switch (m->Type()){
    case ICQ_READxAWAYxMSG:
    case ICQ_READxOCCUPIEDxMSG:
    case ICQ_READxNAxMSG:
    case ICQ_READxDNDxMSG:
    case ICQ_READxFFCxMSG:{
            unsigned char msgType = 0xE8;
            switch (m->Type()){
            case ICQ_READxOCCUPIEDxMSG:
                msgType = 0xE9;
                break;
            case ICQ_READxNAxMSG:
                msgType = 0xEA;
                break;
            case ICQ_READxDNDxMSG:
                msgType = 0xEB;
                break;
            case ICQ_READxFFCxMSG:
                msgType = 0xEC;
                break;
            }
            char info[18];
            memset(info, 0, sizeof(info));
            Buffer copy;
            p->sendAutoReply(m->getUin(), m->timestamp1, m->timestamp2, info,
                             m->id1, m->id2, msgType, 3, 256, reason, 0, copy);
        }
    default:
        p->declineMessage(m, reason);
    }
}

void ICQClientPrivate::declineMessage(ICQMessage *m, const char *reason)
{
    m->state = ICQ_DECLINED;
    if (m->Direct){
        ICQUser *u = client->getUser(m->getUin());
        if (u && u->direct && u->direct->isLogged())
            u->direct->declineMessage(m, reason);
    }else{
        switch (m->Type()){
        case ICQ_READxAWAYxMSG:
        case ICQ_READxOCCUPIEDxMSG:
        case ICQ_READxNAxMSG:
        case ICQ_READxDNDxMSG:
        case ICQ_READxFFCxMSG:{
                unsigned char msgType = 0xE8;
                switch (m->Type()){
                case ICQ_READxOCCUPIEDxMSG:
                    msgType = 0xE9;
                    break;
                case ICQ_READxNAxMSG:
                    msgType = 0xEA;
                    break;
                case ICQ_READxDNDxMSG:
                    msgType = 0xEB;
                    break;
                case ICQ_READxFFCxMSG:
                    msgType = 0xEC;
                    break;
                }
                char info[18];
                memset(info, 0, sizeof(info));
                Buffer copy;
                sendAutoReply(m->getUin(), m->timestamp1, m->timestamp2, info,
                              m->id1, m->id2, msgType, 3, 256, reason, 0, copy);
            }
        default:
            snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
            sock->writeBuffer << m->timestamp1 << m->timestamp2 << 0x0002;
            sock->writeBuffer.packUin(m->getUin());
            sock->writeBuffer << (unsigned short)0x03;
            packMessage(sock->writeBuffer, m, reason, 1, 0, 0, false, true);
            sendPacket();
        }
    }
    client->cancelMessage(m, false);
}

void ICQClient::acceptMessage(ICQMessage *m)
{
    p->acceptMessage(m);
}

void ICQClientPrivate::acceptMessage(ICQMessage *m)
{
    switch (m->Type()){
    case ICQ_MSGxFILE:{
            ICQFile *file = static_cast<ICQFile*>(m);
            file->listener = new FileTransferListener(file, this);
            file->id1 = file->listener->port();
            file->id2 = 0;
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(m);
            chat->listener = new ChatListener(chat, this);
            chat->id1 = chat->listener->port();
            chat->id2 = 0;
            break;
        }
    default:
        log(L_WARN, "Unknown message type %u for accept", m->Type());
        client->cancelMessage(m);
        return;
    }

    if (m->Direct){
        ICQUser *u = client->getUser(m->getUin());
        if (u && u->direct){
            u->direct->acceptMessage(m);
        }else{
            client->cancelMessage(m);
        }
    }else{

        Buffer msgBuf;
        packMessage(msgBuf, m, NULL, 0, 0, 4, false, true);

        unsigned long local_ip = client->owner->RealIP;
        ICQUser *u = client->getUser(m->getUin());
        if (u){
            if ((u->IP & 0xFFFFFF) != (client->owner->IP & 0xFFFFFF))
                local_ip = client->owner->IP;
        }
        local_ip = client->owner->RealIP;
        msg_id id;
        id.l = m->timestamp1;
        id.h = m->timestamp2;

        Buffer b;
        b << (unsigned short)0;
        b << m->timestamp1 << m->timestamp2
        << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
        b.tlv(0x0A, (unsigned short)0x02);
        b.tlv(0x0F);
        b.tlv(0x03, (unsigned long)htonl(local_ip));
        b.tlv(0x05, (unsigned short)listener->port());
        b.tlv(0x2711, msgBuf);
        sendThroughServer(m->getUin(), 2, b, &id);
    }
    ICQEvent e(EVENT_ACKED, m->getUin());
    e.msg = m;
    client->process_event(&e);
}

void ICQClientPrivate::messageRequest()
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
    sendPacket();
}

void ICQClientPrivate::sendICMB()
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SETxICQxMODE);
    sock->writeBuffer
    << 0x00000000L << 0x00031F40L
    << 0x03E703E7L << 0x00000000L;
    sendPacket();
}

bool ICQClientPrivate::requestAutoResponse(unsigned long uin, bool bAuto)
{
    log(L_DEBUG, "Request auto response %lu", uin);
    ICQUser *user = client->getUser(uin, false);
    if (user == NULL) return false;
    unsigned long status = user->uStatus & 0xFF;
    if (status == 0) return false;
    if (bAuto){
        if (client->owner->InvisibleId && (user->VisibleId == 0)) return false;
        if ((client->owner->InvisibleId == 0) && user->InvisibleId) return false;
    }

    responseRequestSeq = --advCounter;
    unsigned char type = 0xE8;
    if (status & ICQ_STATUS_DND){
        type = 0xEB;
    }else if (status & ICQ_STATUS_OCCUPIED){
        type = 0xE9;
    }else if (status & ICQ_STATUS_NA){
        type = 0xEA;
    }else if (status & ICQ_STATUS_FREEFORCHAT){
        type = 0xEC;
    }

    Buffer msg;
    msg << 0x1B00 << ICQ_TCP_VERSION
    << 0x00000000L << 0x00000000L << 0x00000000L << 0x00000000L
    << 0x00000003L << 0x00000000L
    << advCounter << 0xE000 << advCounter
    << 0x00000000L << 0x00000000L << 0x00000000L
    << type << (char)3;
    msg.pack((unsigned short)(client->owner->uStatus & 0xFFFF));
    msg << 0x0100 << 0x0100 << (char)0;

    Buffer buf;
    buf << 0x0000 << 0x00000000L << 0x00000000L
    << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L
    << 0x000A << 0x0002 << 0x0001
    << 0x000F << 0x0000;
    buf.tlv(0x2711, msg);
    sendThroughServer(uin, 2, buf);

    return true;
}

void ICQClientPrivate::sendThroughServer(unsigned long uin, unsigned short type, Buffer &b, msg_id *id, bool addTlv)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER);
    if (id){
        sock->writeBuffer << id->l << id->h;
    }else{
        sock->writeBuffer << 0x00000000L << 0x00000000L;
    }
    sock->writeBuffer << type;
    sock->writeBuffer.packUin(uin);
    sock->writeBuffer.tlv((type == 1) ? 2 : 5, b);
    if (addTlv && ((id == NULL) || (type == 2))) sock->writeBuffer.tlv((type == 2) ? 3 : 6);
    sendPacket();
}

static void packColor(Buffer &mb, unsigned long color)
{
    char r = color & 0xFF;
    char g = (color >> 8) & 0xFF;
    char b = (color >> 16) & 0xFF;
    mb << b << g << r << (char)0;
}

static char c2h(char c)
{
    c = c & 0xF;
    if (c < 10)
        return '0' + c;
    return 'A' + c - 10;
}

static void b2h(char *&p, char c)
{
    *(p++) = c2h(c >> 4);
    *(p++) = c2h(c);
}

static void packCap(Buffer &b, const capability &c)
{
    char pack_cap[0x27];
    char *p = pack_cap;
    *(p++) = '{';
    b2h(p, c[0]); b2h(p, c[1]); b2h(p, c[2]); b2h(p, c[3]);
    *(p++) = '-';
    b2h(p, c[4]); b2h(p, c[5]);
    *(p++) = '-';
    b2h(p, c[6]); b2h(p, c[7]);
    *(p++) = '-';
    b2h(p, c[8]); b2h(p, c[9]);
    *(p++) = '-';
    b2h(p, c[10]); b2h(p, c[11]);
    b2h(p, c[12]); b2h(p, c[13]); b2h(p, c[14]); b2h(p, c[15]);
    *(p++) = '}';
    *p = 0;
    b << pack_cap;
}

void ICQClientPrivate::packMessage(Buffer &mb, ICQMessage *m, const char *msg,
                                   unsigned short msgType, unsigned short msgFlags,
                                   char oper, bool bShort, bool bConvert)
{
    string message;
    if (msg) message = msg;
    ICQUser *u = client->getUser(m->getUin());
    if (bConvert)
        client->toServer(message, u);

    if (!bShort){
        mb  << (unsigned short)0x1B00 << 0x08000000L
        << 0x0L << 0x0L << 0x0L << 0x0L
        << 0x03000000L << (char)oper
        << m->cookie1 << m->cookie2 << m->cookie1
        << 0x00000000L << 0x00000000L << 0x00000000L;
    }

    mb.pack(m->isExt ? (unsigned short)0x1A : m->Type());		// type
    mb.pack(msgType);
    mb.pack(msgFlags);

    mb << message;

    if (!m->isExt){
        switch (m->Type()){
        case ICQ_MSGxMSG:{
                ICQMsg *msg = static_cast<ICQMsg*>(m);
                if (msg->ForeColor != msg->BackColor){
                    packColor(mb, msg->ForeColor);
                    packColor(mb, msg->BackColor);
                }else{
                    packColor(mb, 0x00000000L);
                    packColor(mb, 0x00FFFFFFL);
                }
                ICQUser *u = client->getUser(msg->getUin());
                if (u && (u->canRTF() || u->canUTF())){
                    mb << 0x26000000L;
                    packCap(mb, capabilities[u->canRTF() ? CAP_RTF : CAP_UTF]);
                }
                break;
            }
        case ICQ_MSGxURL:
            packColor(mb, 0x00000000L);
            packColor(mb, 0x00FFFFFFL);
            break;
        case ICQ_MSGxFILE:{
                ICQFile *file = static_cast<ICQFile*>(m);
                string fileName = file->Name;
                client->toServer(fileName, u);
                mb
                << (unsigned short)file->id1
                << (unsigned short)0
                << string(fileName)
                << (unsigned long)htonl(file->Size);
                mb.pack(file->id1);
                mb
                << (unsigned short)0;
                break;
            }
        case ICQ_MSGxCHAT:{
                ICQChat *chat = static_cast<ICQChat*>(m);
                string n = client->owner->name();
                client->toServer(n, u);
                mb << n
                << (unsigned short)chat->id1
                << (unsigned short)0;
                mb.pack(chat->id1);
                mb << (unsigned short)0;
                break;
            }
        case ICQ_MSGxSECURExOPEN:
        case ICQ_MSGxSECURExCLOSE:
            break;
        case ICQ_READxAWAYxMSG:
        case ICQ_READxOCCUPIEDxMSG:
        case ICQ_READxNAxMSG:
        case ICQ_READxDNDxMSG:
        case ICQ_READxFFCxMSG:
            mb << 0xFFFFFFFFL << 0xFFFFFFFFL;
            break;
        default:
            log(L_WARN, "Unknow type %u in pack message", m->Type());
        }
        return;
    }

    Buffer msgBuf;
    switch (m->Type()){
    case ICQ_MSGxFILE:{
            ICQFile *file = static_cast<ICQFile*>(m);
            string fileDescr = file->Description;
            string fileName = file->Name;
            client->toServer(fileDescr, u);
            client->toServer(fileName, u);
            msgBuf.packStr32(fileDescr.c_str());
            msgBuf << file->id1 << file->id2;
            msgBuf << fileName;
            msgBuf.pack(file->Size);
            msgBuf.pack(file->id1);
            msgBuf << (unsigned short)0;
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(m);
            string reason = chat->Reason;
            client->toServer(reason, u);
            msgBuf.packStr32(reason.c_str());
            msgBuf << (unsigned short) 0 << chat->id1 << chat->id2;
            msgBuf.pack(chat->id1);
            msgBuf.pack((unsigned short)0);
            break;
        }
    default:
        log(L_WARN, "packMessage unknown type");
        return;
    }

    switch (m->Type()){
    case ICQ_MSGxFILE:
        mb << 0x2900F02DL << 0x12D93091L << 0xD3118DD7L
        << 0x00104B06L << 0x462E0000L;
        mb.packStr32("File");
        break;
    case ICQ_MSGxCHAT:
        mb << 0x3A00BFF7L << 0x20B2378EL << 0xD411BD28L
        << 0x0004AC96L << 0xD9050000L;
        mb.packStr32("Send / Start ICQ Chat");
        break;
    }
    mb << 0x00000100L << 0x00010000L << 0x00000000L << (unsigned short)0 << (char)0;
    mb.pack32(msgBuf);

}

void ICQClientPrivate::processMsgQueueThruServer()
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end();){
        if ((sock == NULL) || (sock->isError())) return;
        bool bToStart = false;
        ICQEvent *e = *it;
        if (e->message() == NULL){
            msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            client->process_event(e);
            it = msgQueue.begin();
            continue;
        }
        switch (e->message()->Type()){
        case ICQ_MSGxMSG:{
                ICQMsg *msg = static_cast<ICQMsg*>(e->message());
                Buffer msgBuf;
                string message;
                e->state = ICQEvent::Send;
                for (list<unsigned long>::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
                    ICQUser *u = client->getUser(*itUin);
                    if (u && u->canUTF())
                        msg->Charset = "utf-8";
                    message = makeMessageText(msg, u);
                    if (u && (u->canRTF() || u->canUTF()) &&
                            ((client->owner->InvisibleId && u->VisibleId) ||
                             ((client->owner->InvisibleId == 0) && (u->InvisibleId == 0)))){
                        advCounter--;
                        msgBuf
                        << (unsigned short)0x1B00
                        << (char)0x08
                        << 0x00000000L << 0x00000000L << 0x00000000L << 0x00000000L << 0x00000003L << 0x00000000L;
                        msgBuf.pack(advCounter);
                        msgBuf.pack((unsigned short)0x0E);
                        msgBuf.pack(advCounter);
                        msgBuf
                        << 0x00000000L << 0x00000000L << 0x00000000L
                        << (char)0x01
                        << (char)0;
                        msgBuf.pack((unsigned short)(client->owner->uStatus & 0xFFFF));
                        msgBuf
                        << (unsigned short)0x2100;
                        unsigned short size = strlen(message.c_str()) + 1;
                        msgBuf.pack(size);
                        msgBuf.pack(message.c_str(), size);
                        if (msg->BackColor == msg->ForeColor){
                            msgBuf << 0x00000000L << 0xFFFFFF00L;
                        }else{
                            msgBuf << (msg->ForeColor << 8) << (msg->BackColor << 8);
                        }
                        msgBuf << 0x26000000L;
                        packCap(msgBuf, capabilities[u->canRTF() ? CAP_RTF : CAP_UTF]);
                        Buffer b;
                        msg_id id;
                        id.h = rand();
                        id.l = rand();
                        msg->timestamp1 = id.h;
                        msg->timestamp2 = id.l;
                        b << (unsigned short)0;
                        b << id.l << id.h;
                        b << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
                        b.tlv(0x0A, (unsigned short)0x01);
                        b.tlv(0x0F);
                        b.tlv(0x2711, msgBuf);
                        sendThroughServer(*itUin, 2, b, &id);
                    }else{
                        msgBuf << 0x0000L;
                        msgBuf << message.c_str();
                        Buffer b;
                        b.tlv(0x0501, "\x01", 1);
                        b.tlv(0x0101, msgBuf);
                        sendThroughServer(*itUin, 1, b);
                        e->state = ICQEvent::Success;
                    }
                }
                break;
            }
        case ICQ_MSGxFILE:{
                ICQFile *file = static_cast<ICQFile*>(e->message());
                Buffer mb;
                advCounter--;
                file->cookie1 = advCounter;
                file->cookie2 = 0x0E00;
                file->isExt = true;
                packMessage(mb, file, NULL, 0, 1, 0, false, true);
                Buffer b;
                msg_id id;
                id.h = rand();
                id.l = rand();
                file->timestamp1 = id.l;
                file->timestamp2 = id.h;
                b << (unsigned short)0;
                b << id.l << id.h;
                b << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
                b.tlv(0x0A, (unsigned short)0x01);
                b.tlv(0x0F);
                b.tlv(0x03, (unsigned long)htonl(client->owner->IP));
                b.tlv(0x05, (unsigned short)client->owner->Port);
                b.tlv(0x2711, mb);
                sendThroughServer(file->getUin(), 2, b, &id);
                file->ftState = ICQFile::ThruServerSend;
                ICQEvent eft(EVENT_FILETRANSFER);
                eft.setMessage(file);
                client->process_event(&eft);
                msgQueue.remove(e);
                processQueue.push_back(e);
                it = msgQueue.begin();
                bToStart = true;
                break;
            }
        case ICQ_MSGxCHAT:{
                ICQChat *chat = static_cast<ICQChat*>(e->message());
                Buffer mb;
                advCounter--;
                chat->cookie1 = advCounter;
                chat->cookie2 = 0x0E00;
                chat->isExt = true;

                packMessage(mb, chat, NULL, 0, 1, 0, false, true);
                Buffer b;
                msg_id id;
                id.h = rand();
                id.l = rand();
                chat->timestamp1 = id.l;
                chat->timestamp2 = id.h;
                b << (unsigned short)0;
                b << id.l << id.h;
                b << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
                b.tlv(0x0A, (unsigned short)0x01);
                b.tlv(0x0F);
                b.tlv(0x03, (unsigned long)htonl(client->owner->IP));
                b.tlv(0x05, (unsigned short)client->owner->Port);
                b.tlv(0x2711, mb);
                sendThroughServer(chat->getUin(), 2, b, &id);
                msgQueue.remove(e);
                processQueue.push_back(e);
                it = msgQueue.begin();
                bToStart = true;
                break;
            }
        case ICQ_MSGxURL:{
                ICQUrl *msg = static_cast<ICQUrl*>(e->message());
                for (list<unsigned long>::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
                    ICQUser *u = client->getUser(*itUin);
                    Buffer msgBuffer;
                    string message = msg->Message;
                    string url = msg->URL;
                    client->toServer(message, u);
                    client->toServer(url, u);
                    msgBuffer << message.c_str();
                    msgBuffer << (char)0xFE;
                    msgBuffer << url.c_str();
                    Buffer b;
                    b.pack(client->owner->Uin);
                    b << (char)msg->Type() << (char)0;
                    b << msgBuffer;
                    sendThroughServer(*itUin, 4, b);
                }
                msg->Message = ICQClient::quoteText(msg->Message.c_str());
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxCONTACTxLIST:{
                ICQContacts *msg = static_cast<ICQContacts*>(e->message());
                Buffer msgBuffer;
                unsigned nContacts = msg->Contacts.size();
                char uBuf[13];
                snprintf(uBuf, sizeof(uBuf), "%u", nContacts);
                for (list<unsigned long>::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
                    msgBuffer << uBuf;
                    ICQUser *u = client->getUser(*itUin);
                    for (ContactList::iterator it_msg = msg->Contacts.begin(); it_msg != msg->Contacts.end(); it_msg++){
                        Contact *contact = static_cast<Contact*>(*it_msg);
                        msgBuffer << (char)0xFE;
                        snprintf(uBuf, sizeof(uBuf), "%lu", contact->Uin);
                        msgBuffer << uBuf;
                        msgBuffer << (char)0xFE;
                        string alias = contact->Alias;
                        client->toServer(alias, u);
                        msgBuffer << alias.c_str();
                    }
                    msgBuffer << (char)0xFE;
                    Buffer b;
                    b.pack(client->owner->Uin);
                    b << (char)msg->Type() << (char)0;
                    b << msgBuffer;
                    sendThroughServer(*itUin, 4, b);
                }
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxCHATxINFO:{
                ICQChatInfo *chatInfo = static_cast<ICQChatInfo*>(e->message());
                e->state = ICQEvent::Send;
                msg_id id;
                id.h = rand();
                id.l = rand();
                chatInfo->timestamp1 = id.l;
                chatInfo->timestamp2 = id.h;
                Buffer msg;
                msg << 0x1B00 << ICQ_TCP_VERSION << (char)0
                << 0x60F1A83DL << 0x9149D311L
                << 0x8DBE0010L << 0x4B06462EL
                << (unsigned short)0 << (char)3
                << (unsigned long) 0
                << advCounter << 0x1200 << advCounter
                << 0x00000000L << 0x00000000L << 0x00000000L
                << (char)0x01 << (char)0;
                msg.pack((unsigned short)(client->owner->uStatus & 0xFFFF));
                msg << 0x0100 << 0x0000 << (char)0;
                Buffer buf;
                buf << 0x0000 << 0x00000000L << 0x00000000L
                << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L
                << 0x000A << 0x0002 << 0x0001
                << 0x000F << 0x0000;
                buf.tlv(0x2711, msg);
                sendThroughServer(chatInfo->getUin(), 2, buf, &id);
                break;
            }
        default:
            break;
        }
        if (bToStart) continue;
        it++;
    }
}

bool ICQClientPrivate::requestPhoneBook(unsigned long uin, bool)
{
    log(L_DEBUG, "Send request phones %lu", uin);
    ICQUser *user = client->getUser(uin, false);
    if (user == NULL) return false;
    if (client->owner->InvisibleId && (user->VisibleId == 0)) return false;
    if ((client->owner->InvisibleId == 0) && user->InvisibleId) return false;

    phoneRequestSeq = --advCounter;
    Buffer msgBuf;
    msg_id id;
    id.h = rand();
    id.l = rand();
    msgBuf
    << (unsigned short)0x1B00
    << (char)0x07
    << 0x00A0E93FL << 0x374FE9D3L << 0x11BCD200L << (unsigned short)0x04AC
    << (char)0x96
    << (char)0xDD
    << (char)0x96
    << (unsigned short)0
    << 0x03000000L
    << (char)0
    << advCounter
    << (unsigned short)0x1200
    << advCounter
    << 0x00000000L << 0x00000000L << 0x00000000L
    << (char)0x01
    << (char)0
    << (unsigned short)0
    << (unsigned short)0x0200
    << (char)0x01 << (unsigned short)0;
    msgBuf.pack((char*)PHONEBOOK_SIGN, 16);
    msgBuf << (unsigned short)0;
    msgBuf.pack(client->owner->PhoneBookTime);
    Buffer b;
    b << (unsigned short)0;
    b << id.l << id.h;
    b << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
    b.tlv(0x0A, (unsigned short)0x01);
    b.tlv(0x0F);
    b.tlv(0x2711, msgBuf);
    sendThroughServer(uin, 2, b, &id);
    return true;
}

void ICQClientPrivate::cancelSendFile(ICQFile *file)
{
    file->DeclineReason = "";
    msg_id id;
    id.l = file->timestamp1;
    id.h = file->timestamp2;
    Buffer b;
    b << (unsigned short)1
    << id.l << id.h
    << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L
    << 0x000B0002L << (unsigned short)0x0001;
    sendThroughServer(file->getUin(), 2, b, &id);
}

