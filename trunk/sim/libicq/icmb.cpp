/***************************************************************************
                          icmb.cpp  -  description
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
#ifndef WIN32
#include <arpa/inet.h>
#endif

const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002;
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004;
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005;
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006;
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007;
const unsigned short ICQ_SNACxMSG_AUTOREPLY        = 0x000B;
const unsigned short ICQ_SNACxMSG_ACK              = 0x000C;

const unsigned char ICQClient::PHONEBOOK_SIGN[16] =
    { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D, 0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04, 0xAC, 0x96, 0xAA, 0xB2 };

const unsigned char ICQClient::PLUGINS_SIGN[16] =
    { 0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71, 0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E };

void ICQClient::snac_message(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxMSG_RIGHTSxGRANTED:
        log(L_DEBUG, "Message rights granted");
        break;
    case ICQ_SNACxMSG_ACK:
        log(L_DEBUG, "Ack message");
        break;
    case ICQ_SNACxMSG_AUTOREPLY:{
            unsigned long timestamp1, timestamp2;
            readBuffer >> timestamp1 >> timestamp2;
            readBuffer.incReadPos(2);
            unsigned long uin = readBuffer.unpackUin();
            readBuffer.incReadPos(6);
            unsigned long t1, t2;
            readBuffer >> t1 >> t2;
            unsigned short seq;
            readBuffer.incReadPos(0x0F);
            readBuffer >> seq;
            if ((t1 == 0) && (t2 == 0)){
                readBuffer.incReadPos(0x16);
                string answer;
                readBuffer >> answer;
                fromServer(answer);
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
                            cancelMessage(msg, false);
                            break;
                        }
                    }
                    if (it == processQueue.end())
                        log(L_WARN, "Decline file answer: message not found");
                }else{
                    log(L_DEBUG, "[%X] Autoreply from %u %s", seq, uin, answer.c_str());
                    ICQUser *u = getUser(uin);
                    if (u) u->AutoReply = answer;
                    ICQEvent e(EVENT_STATUS_CHANGED, uin);
                    process_event(&e);
                    processResponseRequestQueue(seq);
                }
            }
            if ((t1 == 0xA0E93F37L) && (t2 == 0x4FE9D311L)){
                ICQUser *u = getUser(uin);
                if (u == NULL){
                    log(L_WARN, "Request info no my user %lu", uin);
                    return;
                }
                if (u->inIgnore()){
                    log(L_WARN, "Request info ignore user %lu", uin);
                    return;
                }

                readBuffer.incReadPos(0x1D);
                unsigned long cookie;
                readBuffer >> cookie;
                readBuffer.incReadPos(4);
                readBuffer.unpack(t1);
                if (t1 == 3){
                    u->PhoneBookTime = (unsigned long)htonl(cookie);
                    u->bPhoneChanged = false;
                    log(L_DEBUG, "[%X] Phone book info %u", seq, uin);
                    PhoneBook::iterator it;
                    PhonePtrList myNumbers;
                    for (it = u->Phones.begin(); it != u->Phones.end(); ++it){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        if (!phone->MyInfo()) continue;
                        PhoneInfo *myPhone = new PhoneInfo;
                        *myPhone = *phone;
                        myNumbers.push_back(myPhone);
                    }
                    u->Phones.clear();
                    unsigned long nPhones;
                    readBuffer.unpack(nPhones);
                    for (unsigned i = 0; i < nPhones; i++){
                        PhoneInfo *phone = new PhoneInfo;
                        u->Phones.push_back(phone);
                        readBuffer.unpackStr32(phone->Name);
                        readBuffer.unpackStr32(phone->AreaCode);
                        readBuffer.unpackStr32(phone->Number);
                        readBuffer.unpackStr32(phone->Extension);
                        readBuffer.unpackStr32(phone->Country);
                        unsigned long type;
                        readBuffer.unpack(type);
                        if (type) phone->Active = true;
                        if (readBuffer.readPos() >= readBuffer.size()) break;
                    }
                    for (it = u->Phones.begin(); it != u->Phones.end(); it++){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        string prop;
                        readBuffer.unpackStr32(prop);
                        Buffer b;
                        b.pack(prop.c_str(), prop.length());
                        b.unpack(phone->Type);
                        b.unpackStr32(phone->Provider);
                        if (readBuffer.readPos() >= readBuffer.size()) break;
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
                    process_event(&e);
                    processPhoneRequestQueue(seq);
                }
            }
            break;
        }
    case ICQ_SNACxMSG_SERVERxMESSAGE:{
            unsigned long timestamp1, timestamp2;
            readBuffer >> timestamp1 >> timestamp2;
            unsigned short mFormat;
            readBuffer >> mFormat;
            unsigned long uin = readBuffer.unpackUin();
            log(L_DEBUG, "Message from %u [%04X]", uin, mFormat);
            if (uin == 0xA){
                readBuffer.incReadPos(14);
                string message;
                readBuffer >> message;
                vector<string> l;
                if (!parseFE(message.c_str(), l, 6)){
                    log(L_WARN, "Parse error web panel message");
                    break;
                }
                char SENDER_IP[] = "Sender IP:";
                if ((l[5].size() > strlen(SENDER_IP)) && !memcmp(l[5].c_str(), SENDER_IP, strlen(SENDER_IP))){
                    ICQWebPanel *m = new ICQWebPanel;
                    fromServer(l[0]);
                    fromServer(l[3]);
                    m->Name = l[0];
                    m->Email = l[3];
                    parseMessageText(l[5].c_str(), m->Message);
                    m->Uin.push_back(contacts.findByEmail(m->Name, m->Email));
                    messageReceived(m);
                }else{
                    ICQEmailPager *m = new ICQEmailPager;
                    fromServer(l[0]);
                    fromServer(l[3]);
                    m->Name = l[0];
                    m->Email = l[3];
                    parseMessageText(l[5].c_str(), m->Message);
                    m->Uin.push_back(contacts.findByEmail(m->Name, m->Email));
                    messageReceived(m);
                }
                break;
            }
            if (uin == 1002)
                log(L_DEBUG, "Incoming SMS");
            unsigned short level, nTLV;
            readBuffer >> level >> nTLV;
            switch (mFormat){
            case 0x0001:{
                    TlvList tlv(readBuffer);
                    if (!tlv(2)){
                        log(L_WARN, "No found generic message tlv");
                        break;
                    }
                    Buffer msg(*tlv(2));
                    TlvList tlv_msg(msg);
                    if (!tlv_msg(0x101)){
                        log(L_WARN, "No found generic message tlv 101");
                        break;
                    }
                    ICQMsg *m = new ICQMsg();
                    m->Uin.push_back(uin);
                    m->Received = true;
                    parseMessageText((const char*)(*tlv_msg(0x101)) + 4, m->Message);
                    messageReceived(m);
                    break;
                }
            case 0x0002:{
                    TlvList tlv(readBuffer);
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
                                    cancelMessage(m, false);
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
                    TlvList tlv(readBuffer);
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

void ICQClient::parseAdvancedMessage(unsigned long uin, Buffer &msg, bool needAck, unsigned long timestamp1, unsigned long timestamp2)
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
    if (sign1 != 3){
        log(L_WARN, "Bad sign in TLV 2711");
        return;
    }
    char sign2;
    payload >> sign2;
    if (sign2 && (sign2 != 1) && (sign2 != 2) && (sign2 != 4)){
        log(L_WARN, "Bad sign in TLV 2711 (%u)", sign2);
        return;
    }
    unsigned short cookie1;
    unsigned short cookie2;
    unsigned short cookie3;
    payload >> cookie1;
    payload >> cookie2;
    payload >> cookie3;
    if (cookie1 != cookie3){
        log(L_WARN, "Bad cookie in TLV 2711");
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
    switch (msgType){
    case 0xE8:
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:{
            getAutoResponse(uin, response);
            log(L_DEBUG, "Request autoreply %s", response.c_str());
            break;
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

                            ICQUser *u = getUser(m->getUin());
                            if (u == NULL){
                                log(L_WARN, "User %lu not found", m->getUin());
                                cancelMessage(m, false);
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
                                cancelMessage(msg, false);
                                return;
                            }
                            ICQEvent e(EVENT_ACKED, m->getUin());
                            e.msg = msg;
                            process_event(&e);
                            break;
                        }
                        if (it == processQueue.end())
                            log(L_WARN, "Accept message not found event");
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
                    ICQUser *u = getUser(m->getUin());
                    if (u){
                        bool bChanged = false;
                        if (real_ip && (u->RealIP() != real_ip)){
                            u->RealIP = real_ip;
                            u->RealHostName = "";
                            bChanged = true;
                        }
                        if (ip && (u->IP() != ip)){
                            u->IP = ip;
                            u->HostName = "";
                            bChanged = true;
                        }
                        if (port && (u->Port() != port)){
                            u->Port = port;
                            bChanged = true;
                            bChanged = true;
                        }
                        log(L_DEBUG, "set IP");
                        if (bChanged){
                            ICQEvent e(EVENT_STATUS_CHANGED, u->Uin());
                            process_event(&e);
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
            ICQUser *u = getUser(uin);
            if (u && !u->inIgnore()){
                char b[16];
                payload.unpack(b, sizeof(b));
                if (!memcmp(b, PLUGINS_SIGN, sizeof(b))){
                    msgType = 0;
                    log(L_DEBUG, "Plugins request");
                    response_type = 1;
                    copy << 0x00000100L << 0x07DF463CL;
                    Buffer buf;
                    buf << 0x02000100L << (unsigned long)htonl(1);
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
                    copy << 0x00000100L << (unsigned long)htonl(PhoneBookTime());

                    Buffer b;
                    b << 0x03000000L << (unsigned long)htonl(Phones.size());
                    PhoneBook::iterator it;
                    for (it = Phones.begin(); it != Phones.end(); it++){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        b.packStr32(phone->Name);
                        b.packStr32(phone->AreaCode);
                        b.packStr32(phone->Number);
                        b.packStr32(phone->Extension);
                        b.packStr32(phone->Country);
                        if (phone->Active()){
                            b << 0x01000000L;
                        }else{
                            b << 0x00000000L;
                        }
                    }
                    for (it = Phones.begin(); it != Phones.end(); it++){
                        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
                        Buffer b1;
                        b1 << (unsigned long)htonl(phone->Type());
                        b1.packStr32(phone->Provider);
                        b1 << (unsigned long)htonl(1)
                        << (unsigned long)htonl((phone->Type == SMS) ? 1 : 0)
                        << (unsigned long)htonl(1)
                        << (unsigned long)htonl(2);
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

    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
    writeBuffer << timestamp1 << timestamp2 << 0x0002;
    writeBuffer.packUin(uin);
    writeBuffer << 0x0003 << 0x1B00 << 0x0800;
    writeBuffer.pack(info, sizeof(info));
    writeBuffer
    << 0x03000000L << (char)0
    << cookie1 << cookie2 << cookie1
    << 0x00000000L << 0x00000000L << 0x00000000L
    << (char)msgType << (char)msgFlags << msgState;
    if (response.size()){
        toServer(response);
        writeBuffer << (unsigned short)htons(response.size() + 1);
        writeBuffer << response.c_str();
        writeBuffer << (char)0;
    }else{
        writeBuffer << (char)0x01 << response_type;
        if (response_type != 3){
            if (copy.size()){
                writeBuffer.pack(copy.Data(0), copy.writePos());
            }else{
                writeBuffer << 0x00000000L << 0xFFFFFF00L;
            }
        }
    }
    sendPacket();
}

void ICQClient::declineMessage(ICQMessage *m, const char *reason)
{
    m->state = ICQ_DECLINED;
    if (m->Direct()){
        ICQUser *u = getUser(m->getUin());
        if (u && u->direct)
            u->direct->declineMessage(m, reason);
    }else{
        snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
        writeBuffer << m->timestamp1 << m->timestamp2 << 0x0002;
        writeBuffer.packUin(m->getUin());
        writeBuffer << (unsigned short)0x03;
        packMessage(writeBuffer, m, reason, 1, 0, 0, false, true);
        sendPacket();
    }
    cancelMessage(m, false);
}

void ICQClient::acceptMessage(ICQMessage *m)
{
    switch (m->Type()){
    case ICQ_MSGxFILE:{
            ICQFile *file = static_cast<ICQFile*>(m);
            file->listener = new FileTransferListener(file, this);
            file->listener->listen(MinTCPPort(), MaxTCPPort());
            file->id1 = file->listener->port();
            file->id2 = 0;
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(m);
            chat->listener = new ChatListener(chat, this);
            chat->listener->listen(MinTCPPort(), MaxTCPPort());
            chat->id1 = chat->listener->port();
            chat->id2 = 0;
            break;
        }
    default:
        log(L_WARN, "Unknown message type %u for accept", m->Type());
        cancelMessage(m);
        return;
    }

    if (m->Direct()){
        ICQUser *u = getUser(m->getUin());
        if (u && u->direct){
            u->direct->acceptMessage(m);
        }else{
            cancelMessage(m);
        }
    }else{

        Buffer msgBuf;
        packMessage(msgBuf, m, NULL, 0, 0, 4, false, true);

        char *host;
        unsigned short port;
        unsigned long remote_ip = 0;
        unsigned long local_ip = 0;
        ICQUser *u = getUser(m->getUin());
        if (u){
            remote_ip = u->IP();
            if (remote_ip && ((remote_ip & 0xFFFFFF00) != (IP() & 0xFFFFFF00)))
                remote_ip = u->RealIP();
        }
        if (getLocalAddr(host, port, remote_ip))
            local_ip = inet_addr(host);
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
    process_event(&e);
}

void ICQClient::messageRequest()
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
    sendPacket();
}

void ICQClient::sendICMB()
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SETxICQxMODE);
    writeBuffer
    << 0x00000000L << 0x00031F40L
    << 0x03E703E7L << 0x00000000L;
    sendPacket();
}

void ICQClient::requestAutoResponse(unsigned long uin)
{
    log(L_DEBUG, "Request auto response %lu", uin);
    ICQUser *user = getUser(uin, false);
    if (user == NULL) return;
    unsigned long status = user->uStatus & 0xFF;
    if (status == 0) return;

    advCounter--;
    unsigned char type = 0xEA;
    switch (status & 0xFF){
    case ICQ_STATUS_AWAY:
        type = 0xE8;
        break;
    case ICQ_STATUS_DND:
        type = 0xEB;
        break;
    case ICQ_STATUS_OCCUPIED:
        type = 0xE9;
        break;
    case ICQ_STATUS_FREEFORCHAT:
        type = 0xEC;
        break;
    }

    Buffer msg;
    msg << 0x1B00 << ICQ_TCP_VERSION
    << 0x00000000L << 0x00000000L << 0x00000000L << 0x00000000L
    << 0x00000003L << 0x00000000L
    << advCounter << 0xE000 << advCounter
    << 0x00000000L << 0x00000000L << 0x00000000L
    << type << (char)3
    << (unsigned short)htons(uStatus & 0xFFFF)
    << 0x0100 << 0x0100 << (char)0;

    Buffer buf;
    buf << 0x0000 << 0x00000000L << 0x00000000L
    << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L
    << 0x000A << 0x0002 << 0x0001
    << 0x000F << 0x0000;
    buf.tlv(0x2711, msg);
    sendThroughServer(uin, 2, buf);
}

void ICQClient::sendThroughServer(unsigned long uin, unsigned short type, Buffer &b, msg_id *id, bool addTlv)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER);
    if (id){
        writeBuffer << id->l << id->h;
    }else{
        writeBuffer << 0x00000000L << 0x00000000L;
    }
    writeBuffer << type;
    writeBuffer.packUin(uin);
    writeBuffer.tlv((type == 1) ? 2 : 5, b);
    if (addTlv && ((id == NULL) || (type == 2))) writeBuffer.tlv((type == 2) ? 3 : 6);
    sendPacket();
}

static void packColor(Buffer &mb, unsigned long color)
{
    char r = color & 0xFF;
    char g = (color >> 8) & 0xFF;
    char b = (color >> 16) & 0xFF;
    mb << b << g << r << (char)0;
}

void ICQClient::packMessage(Buffer &mb, ICQMessage *m, const char *msg,
                            unsigned short msgType, unsigned short msgFlags,
                            char oper, bool bShort, bool bConvert)
{
    string message;
    if (msg) message = msg;
    if (bConvert) toServer(message);

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
                if (msg->ForeColor() != msg->BackColor()){
                    packColor(mb, msg->ForeColor());
                    packColor(mb, msg->BackColor());
                }else{
                    packColor(mb, 0x00000000L);
                    packColor(mb, 0x00FFFFFFL);
                }
                ICQUser *u = getUser(msg->getUin());
                if (u && u->GetRTF)
                    mb.packStr32("{97B12751-243C-4334-AD22-D6ABF73F1492}");
                break;
            }
        case ICQ_MSGxURL:
            packColor(mb, 0x00000000L);
            packColor(mb, 0x00FFFFFFL);
            break;
        case ICQ_MSGxFILE:{
                ICQFile *file = static_cast<ICQFile*>(m);
                string fileName = file->shortName();
                toServer(fileName);
                mb
                << (unsigned short)file->id1
                << (unsigned short)0
                << string(fileName)
                << (unsigned long)htonl(file->Size())
                << (unsigned short)htons(file->id1)
                << (unsigned short)0;
                break;
            }
        case ICQ_MSGxCHAT:{
                ICQChat *chat = static_cast<ICQChat*>(m);
                string n = name();
                toServer(n);
                mb << n
                << (unsigned short)chat->id1
                << (unsigned short)0
                << (unsigned short)htons(chat->id1)
                << (unsigned short)0;
                break;
            }
        case ICQ_MSGxSECURExOPEN:
        case ICQ_MSGxSECURExCLOSE:
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
            string fileName = file->shortName();
            toServer(fileDescr);
            toServer(fileName);
            msgBuf.packStr32(fileDescr.c_str());
            msgBuf << file->id1 << file->id2;
            msgBuf << fileName;
            msgBuf << (unsigned long)htonl(file->Size());
            msgBuf << (unsigned short)htons(file->id1);
            msgBuf << (unsigned short)0;
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(m);
            string reason = chat->Reason;
            toServer(reason);
            msgBuf.packStr32(reason.c_str());
            msgBuf << (unsigned short) 0 << chat->id1 << chat->id2;
            msgBuf << (unsigned short)htons(chat->id1) << (unsigned short)0;
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

void ICQClient::processMsgQueueThruServer()
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end();){
        bool bToStart = false;
        ICQEvent *e = *it;
        if (e->message() == NULL){
            msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            process_event(e);
            it = msgQueue.begin();
            continue;
        }
        switch (e->message()->Type()){
        case ICQ_MSGxMSG:{
                ICQMsg *msg = static_cast<ICQMsg*>(e->message());
                Buffer msgBuf;
                string message;
                for (ConfigULongs::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
                    ICQUser *u = getUser(*itUin);
                    if (u && u->GetRTF && (u->uStatus != ICQ_STATUS_OFFLINE)){
                        string msg_text = msg->Message;
                        toServer(msg_text);
                        message = createRTF(msg_text.c_str(), msg->ForeColor);
                        advCounter--;
                        msgBuf
                        << (unsigned short)0x1B00
                        << (char)0x08
                        << 0x00000000L << 0x00000000L << 0x00000000L << 0x00000000L << 0x00000003L << 0x00000000L
                        << advCounter
                        << (unsigned short)0x0E00
                        << advCounter
                        << 0x00000000L << 0x00000000L << 0x00000000L
                        << (char)0x01
                        << (char)0
                        << (unsigned short)0
                        << (unsigned short)0x0100
                        << (unsigned short)htons(message.size() + 1);
                        msgBuf.pack(message.c_str(), message.size() + 1);
                        if (msg->BackColor == msg->ForeColor){
                            msgBuf << 0x00000000L << 0xFFFFFF00L;
                        }else{
                            msgBuf << (msg->ForeColor() << 8) << (msg->BackColor() << 8);
                        }
                        msgBuf.packStr32("{97B12751-243C-4334-AD22-D6ABF73F1492}");
                        Buffer b;
                        msg_id id;
                        id.h = rand();
                        id.l = rand();
                        b << (unsigned short)0;
                        b << id.l << id.h;
                        b << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
                        b.tlv(0x0A, (unsigned short)0x01);
                        b.tlv(0x0F);
                        b.tlv(0x2711, msgBuf);
                        sendThroughServer(*itUin, 2, b, &id);
                    }else{
                        message = clearHTML(msg->Message.c_str());
                        toServer(message);
                        msgBuf << 0x0000L;
                        msgBuf << message.c_str();
                        Buffer b;
                        b.tlv(0x0501, "\x01", 1);
                        b.tlv(0x0101, msgBuf);
                        sendThroughServer(*itUin, 1, b);
                    }
                }
                (*it)->state = ICQEvent::Success;
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
                b.tlv(0x03, (unsigned long)htonl(IP()));
                b.tlv(0x05, (unsigned short)Port());
                b.tlv(0x2711, mb);
                sendThroughServer(file->getUin(), 2, b, &id);
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
                b.tlv(0x03, (unsigned long)htonl(IP()));
                b.tlv(0x05, (unsigned short)Port());
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
                for (ConfigULongs::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
                    Buffer msgBuffer;
                    string message = msg->Message;
                    string url = msg->URL;
                    toServer(message);
                    toServer(url);
                    msgBuffer << message.c_str();
                    msgBuffer << (char)0xFE;
                    msgBuffer << url.c_str();
                    Buffer b;
                    b << (unsigned long)htonl(Uin()) << (char)msg->Type() << (char)0;
                    b << msgBuffer;
                    sendThroughServer(*itUin, 4, b);
                }
                string s;
                quoteText(msg->Message, s);
                msg->Message = s;
                (*it)->state = ICQEvent::Success;
                break;
            }
        case ICQ_MSGxCONTACTxLIST:{
                ICQContacts *msg = static_cast<ICQContacts*>(e->message());
                Buffer msgBuffer;
                unsigned nContacts = msg->Contacts.size();
                char u[13];
                snprintf(u, sizeof(u), "%u", nContacts);
                for (ConfigULongs::iterator itUin = msg->Uin.begin(); itUin != msg->Uin.end(); ++itUin){
                    msgBuffer << u;
                    for (ContactList::iterator it_msg = msg->Contacts.begin(); it_msg != msg->Contacts.end(); it_msg++){
                        Contact *contact = static_cast<Contact*>(*it_msg);
                        msgBuffer << (char)0xFE;
                        snprintf(u, sizeof(u), "%lu", contact->Uin());
                        msgBuffer << u;
                        msgBuffer << (char)0xFE;
                        string alias = contact->Alias;
                        toServer(alias);
                        msgBuffer << alias.c_str();
                    }
                    msgBuffer << (char)0xFE;
                    Buffer b;
                    b << (unsigned long)htonl(Uin()) << (char)msg->Type() << (char)0;
                    b << msgBuffer;
                    sendThroughServer(*itUin, 4, b);
                }
                (*it)->state = ICQEvent::Success;
                break;
            }
        default:
            break;
        }
        if (bToStart) continue;
        if (e->state != ICQEvent::Success){
            it++;
            continue;
        }
        msgQueue.remove(e);
        process_event(e);
        it = msgQueue.begin();
    }
}

void ICQClient::requestPhoneBook(unsigned long uin)
{
    log(L_DEBUG, "Send request phones %lu", uin);
    advCounter--;
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
    msgBuf << (unsigned short)0
    << (unsigned long)htonl(PhoneBookTime());
    Buffer b;
    b << (unsigned short)0;
    b << id.l << id.h;
    b << 0x09461349L << 0x4C7F11D1L << 0x82224445L << 0x53540000L;
    b.tlv(0x0A, (unsigned short)0x01);
    b.tlv(0x0F);
    b.tlv(0x2711, msgBuf);
    sendThroughServer(uin, 2, b, &id);
}

void ICQClient::cancelSendFile(ICQFile *file)
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
