/***************************************************************************
                          icqicmb.cpp  -  description
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
#include "icqmessage.h"
#include "icq.h"

#include "core.h"

#include <stdio.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif
#include <time.h>

#include <qtextcodec.h>
#include <qfile.h>
#include <qtimer.h>
#include <qimage.h>

const unsigned short ICQ_SNACxMSG_ERROR			   = 0x0001;
const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002;
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004;
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005;
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006;
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007;
const unsigned short ICQ_SNACxMSG_SRV_MISSED_MSG   = 0x000A;
const unsigned short ICQ_SNACxMSG_AUTOREPLY        = 0x000B;
const unsigned short ICQ_SNACxMSG_ACK              = 0x000C;
const unsigned short ICQ_SNACxMSG_MTN			   = 0x0014;

void ICQClient::snac_icmb(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxMSG_RIGHTSxGRANTED:
        log(L_DEBUG, "Message rights granted");
        break;
    case ICQ_SNACxMSG_MTN:{
            m_socket->readBuffer.incReadPos(10);
            unsigned long uin = m_socket->readBuffer.unpackUin();
            unsigned short type;
            m_socket->readBuffer >> type;
            bool bType = (type > 1);
            Contact *contact;
            ICQUserData *data = findContact(uin, NULL, false, contact);
            if (data == NULL)
                break;
            if ((bool)(data->bTyping) == bType)
                break;
            data->bTyping = bType;
            Event e(EventContactStatus, contact);
            e.process();
            break;
        }
    case ICQ_SNACxMSG_ERROR:{
            unsigned short error;
            m_socket->readBuffer >> error;
            const char *err_str = I18N_NOOP("Unknown error");
            if (error == 0x0009){
                Contact *contact;
                ICQUserData *data = findContact(m_send.uin, NULL, false, contact);
                if (data){
                    for (list<SendMsg>::iterator it = sendQueue.begin(); it != sendQueue.end();){
                        if ((*it).uin != m_send.uin){
                            ++it;
                            continue;
                        }
                        if ((*it).msg){
                            (*it).flags = 0;
                            ++it;
                            continue;
                        }
                        sendQueue.erase(it);
                        it = sendQueue.begin();
                    }
                    data->bBadClient = true;
                    if (m_send.msg)
                        sendThruServer(m_send.msg, data);
                    m_send.msg = NULL;
                    m_send.uin = 0;
                    m_sendTimer->stop();
                    send(true);
                    break;
                }
            }
            switch (error){
            case 0x0004:
                err_str = I18N_NOOP("User is offline");
                break;
            case 0x000E:
                err_str = I18N_NOOP("Packet was malformed");
                break;
            case 0x0015:
                err_str = I18N_NOOP("List overflow");
                break;
            }
            log(L_DEBUG, "ICMB error %u (%s)", error, err_str);
            if (m_send.msg){
                if (m_send.msg->type() == MessageCheckInvisible){
                    bool bInvisible = (error == 0x000E);
                    Contact *contact;
                    ICQUserData *data = findContact(m_send.uin, NULL, false, contact);
                    if (data && ((bool)(data->bInvisible) != bInvisible)){
                        data->bInvisible = bInvisible;
                        Event e(EventContactStatus, contact);
                        e.process();
                    }
                }
                m_send.msg->setError(err_str);
                Event e(EventMessageSent, m_send.msg);
                e.process();
                delete m_send.msg;
            }
            m_send.msg = NULL;
            m_send.uin = 0;
            m_sendTimer->stop();
            send(true);
            break;
        }
    case ICQ_SNACxMSG_SRV_MISSED_MSG: {
            unsigned short error;
            m_socket->readBuffer >> error;
            unsigned short uin = m_socket->readBuffer.unpackUin();
            const char *err_str = NULL;
            switch (error) {
            case 0x00:
                err_str = I18N_NOOP("Invalid message");
                break;
            case 0x01:
                err_str = I18N_NOOP("Message was too large");
                break;
            case 0x02:
                err_str = I18N_NOOP("Message rate exceeded");
                break;
            case 0x03:
                err_str = I18N_NOOP("Sender too evil");
                break;
            case 0x04:
                err_str = I18N_NOOP("We are to evil :(");
                break;
            default:
                err_str = I18N_NOOP("Unknown error");
            }
            log(L_DEBUG, "ICMB error %u (%s) - uin(%u)", error, err_str, uin);

        }
    case ICQ_SNACxMSG_ACK:
        {
            log(L_DEBUG, "Ack message");
            MessageId id;
            m_socket->readBuffer >> id.id_l >> id.id_h;
            m_socket->readBuffer.incReadPos(2);
            unsigned long uin = m_socket->readBuffer.unpackUin();
            if ((m_send.uin != uin) || !(m_send.id == id))
                log(L_WARN, "Bad ack sequence");
            if (m_send.msg){
                ackMessage();
            }else{
                replyQueue.push_back(m_send);
            }
            m_send.msg = NULL;
            m_send.uin = 0;
            m_sendTimer->stop();
            send(true);
            break;
        }
    case ICQ_SNACxMSG_AUTOREPLY:{
            MessageId id;
            m_socket->readBuffer >> id.id_l >> id.id_h;
            m_socket->readBuffer.incReadPos(2);
            unsigned long uin = m_socket->readBuffer.unpackUin();
            m_socket->readBuffer.incReadPos(2);
            unsigned short len;
            m_socket->readBuffer.unpack(len);
            m_socket->readBuffer.incReadPos(2);
            plugin p;
            m_socket->readBuffer.unpack((char*)p, sizeof(p));
            m_socket->readBuffer.incReadPos(len - sizeof(plugin) + 2);
            m_socket->readBuffer.unpack(len);
            m_socket->readBuffer.incReadPos(len + 0x10);

            if (memcmp(p, plugins[PLUGIN_NULL], sizeof(plugin))){
                unsigned plugin_index;
                for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++){
                    if (memcmp(p, plugins[plugin_index], sizeof(plugin)) == 0)
                        break;
                }
                if (plugin_index == PLUGIN_NULL){
                    string plugin_str;
                    unsigned i;
                    for (i = 0; i < sizeof(plugin); i++){
                        char b[4];
                        sprintf(b, "%02X ", p[i]);
                        plugin_str += b;
                    }
                    log(L_DEBUG, "Unknown plugin sign in reply %s", plugin_str.c_str());
                    break;
                }
                Contact *contact;
                ICQUserData *data = findContact(uin, NULL, false, contact);
                if ((data == NULL) && (plugin_index != PLUGIN_RANDOMxCHAT))
                    break;

                list<SendMsg>::iterator it;
                for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                    SendMsg &s = *it;
                    if ((s.id == id) && (s.uin == uin))
                        break;
                }
                if (it == replyQueue.end()){
                    log(L_DEBUG, "No found message for plugin answer (%u)", uin);
                    break;
                }
                unsigned plugin_type = (*it).flags;
                replyQueue.erase(it);
                parsePluginPacket(m_socket->readBuffer, plugin_type, data, uin, false);
                break;
            }

            string answer;
            m_socket->readBuffer >> answer;
            log(L_DEBUG, "Autoreply from %u %s", uin, answer.c_str());
            Contact *contact;
            ICQUserData *data = findContact(uin, NULL, false, contact);
            if (data && set_str(&data->AutoReply, answer.c_str())){
                Event e(EventContactChanged, contact);
                e.process();
            }
            break;
        }
    case ICQ_SNACxMSG_SERVERxMESSAGE:{
            unsigned long timestamp1, timestamp2;
            m_socket->readBuffer >> timestamp1 >> timestamp2;
            unsigned short mFormat;
            m_socket->readBuffer >> mFormat;
            unsigned long uin = m_socket->readBuffer.unpackUin();
            log(L_DEBUG, "Message from %u [%04X]", uin, mFormat);
            unsigned short level, nTLV;
            m_socket->readBuffer >> level >> nTLV;
            switch (mFormat){
            case 0x0001:{
                    TlvList tlv(m_socket->readBuffer);
                    if (!tlv(2)){
                        log(L_WARN, "No found generic message tlv");
                        break;
                    }
                    Buffer m(*tlv(2));
                    TlvList tlv_msg(m);
                    Tlv *m_tlv = tlv_msg(0x101);
                    if (m_tlv == NULL){
                        log(L_WARN, "No found generic message tlv 101");
                        break;
                    }
                    if (m_tlv->Size() <= 4)
                        break;
                    char *m_data = (*m_tlv);
                    unsigned short encoding = (m_data[0] << 8) + m_data[1];
                    m_data += 4;
                    if (encoding == 2){
                        QString text;
                        for (int i = 0; i < m_tlv->Size() - 5; i += 2){
                            unsigned char r1 = *(m_data++);
                            unsigned char r2 = *(m_data++);
                            unsigned short c = (r1 << 8) + r2;
                            text += QChar(c);
                        }
                        Message *msg = new Message(MessageGeneric);
                        msg->setText(text);
                        messageReceived(msg, uin);
                        break;
                    }
                    ICQMessage *msg = new ICQMessage;
                    msg->setServerText(m_data);
                    messageReceived(msg, uin);
                    break;
                }
            case 0x0002:{
                    TlvList tlv(m_socket->readBuffer);
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
                    case 1:
                        log(L_DEBUG, "Cancel");
                        break;
                    case 2:
                        log(L_DEBUG, "File ack");
                        break;
                    default:
                        log(L_WARN, "Unknown type: %u", type);
                    }
                    break;
                }
            case 0x0004:{
                    TlvList tlv(m_socket->readBuffer);
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
                    Message *m = parseMessage(type, uin, msg_str, msg, 0, 0, timestamp1, timestamp2);
                    if (m)
                        messageReceived(m, uin);
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

void ICQClient::icmbRequest()
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_REQUESTxRIGHTS);
    sendPacket();
}

void ICQClient::sendICMB(unsigned short channel, unsigned long flags)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SETxICQxMODE);
    m_socket->writeBuffer
    << channel << flags
    << (unsigned short)8000		// max message size
    << (unsigned short)999		// max sender warning level
    << (unsigned short)999		// max receiver warning level
    << (unsigned short)0		// min message interval
    << (unsigned short)0;		// unknown
    sendPacket();
}

bool ICQClient::sendThruServer(Message *msg, void *_data)
{
    ICQUserData *data = (ICQUserData*)_data;
    Contact *contact = getContacts()->contact(msg->contact());
    if ((contact == NULL) || (data == NULL))
        return false;
    SendMsg s;
    switch (msg->type()){
    case MessageGeneric:
        if ((data->Status != ICQ_STATUS_OFFLINE) &&
                hasCap(data, CAP_RTF) && (msg->getFlags() & MESSAGE_RICHTEXT) &&
                !data->bBadClient){
            s.flags  = SEND_RTF;
            s.msg    = msg;
            s.text   = msg->getRichText();
            s.uin    = data->Uin;
            sendQueue.push_front(s);
            send(false);
            return true;
        }
        if ((data->Status != ICQ_STATUS_OFFLINE) &&
                hasCap(data, CAP_UTF) &&
                !data->bBadClient){
            s.flags  = SEND_UTF;
            s.msg    = msg;
            s.text   = msg->getPlainText();
            s.uin    = data->Uin;
            sendQueue.push_front(s);
            send(false);
            return true;
        }
        s.flags	= SEND_PLAIN;
        s.msg	= msg;
        s.text	= msg->getPlainText();
        s.uin	= data->Uin;
        sendQueue.push_front(s);
        send(false);
        return true;
    case MessageURL:
    case MessageContact:
    case MessageCheckInvisible:
        s.flags = SEND_RAW;
        s.msg   = msg;
        s.uin	= data->Uin;
        sendQueue.push_front(s);
        send(false);
        return true;
    }
    return false;
}

void ICQClient::sendThroughServer(unsigned long uin, unsigned short type, Buffer &b, unsigned long id_l, unsigned long id_h, bool addTlv)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER);
    m_socket->writeBuffer << id_l << id_h;
    m_socket->writeBuffer << type;
    m_socket->writeBuffer.packUin(uin);
    m_socket->writeBuffer.tlv((type == 1) ? 2 : 5, b);
    if (addTlv && (((id_l == 0) && (id_h == 0)) || (type == 2)))
        m_socket->writeBuffer.tlv((type == 2) ? 3 : 6);
    sendPacket();
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

void packCap(Buffer &b, const capability &c)
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

void ICQClient::ackMessage()
{
    if ((m_send.msg->getFlags() & MESSAGE_NOHISTORY) == 0){
        if ((m_send.flags & SEND_MASK) == SEND_RAW){
            m_send.msg->setClient(dataName(m_send.uin).c_str());
            Event e(EventSent, m_send.msg);
            e.process();
        }else if (!m_send.part.isEmpty()){
            Message m(MessageGeneric);
            m.setContact(m_send.msg->contact());
            m.setText(m_send.part);
            m.setBackground(m_send.msg->getBackground());
            m.setForeground(m_send.msg->getForeground());
            if ((m_send.flags & SEND_MASK) == SEND_RTF)
                m.setFlags(MESSAGE_RICHTEXT);
            m.setClient(dataName(m_send.uin).c_str());
            Event e(EventSent, &m);
            e.process();
        }
    }
    string text;
    if (m_send.text.length() == 0){
        Event e(EventMessageSent, m_send.msg);
        e.process();
        delete m_send.msg;
    }else{
        sendQueue.push_front(m_send);
    }
    m_send.msg = NULL;
    m_send.uin = 0;
    send(true);
}

void ICQClient::sendAdvMessage(unsigned long uin, Buffer &msgText, unsigned plugin_index, const MessageId &id, bool bPeek)
{
    Buffer msgBuf;
    m_advCounter--;
    msgBuf.pack((unsigned short)0x1B);
    msgBuf.pack((unsigned short)0x08);
    msgBuf.pack((char*)plugins[plugin_index], sizeof(plugin));
    msgBuf.pack(0x00000003L);
    msgBuf.pack((char)0);
    msgBuf.pack(m_advCounter);
    msgBuf.pack((plugin_index == PLUGIN_NULL) ? (unsigned short)0x0E : (unsigned short)0x12);
    msgBuf.pack(m_advCounter);
    msgBuf.pack(0x00000000L);
    msgBuf.pack(0x00000000L);
    msgBuf.pack(0x00000000L);
    msgBuf.pack(msgText.data(0), msgText.size());
    Buffer b;
    b << (unsigned short)0;
    b << id.id_l << id.id_h;
    b.pack((char*)capabilities[CAP_SRV_RELAY], sizeof(capability));
    b.tlv(0x0A, (unsigned short)0x01);
    b.tlv(0x0F);
    b.tlv(0x2711, msgBuf);
    if (bPeek)
        b.tlv(0x03);
    sendThroughServer(uin, 2, b, id.id_l, id.id_h);
}

void ICQClient::clearMsgQueue()
{
    for (list<SendMsg>::iterator it = sendQueue.begin(); it != sendQueue.end(); ++it){
        if ((*it).msg == NULL)
            continue;
        (*it).msg->setError(I18N_NOOP("Client go offline"));
        Event e(EventMessageSent, (*it).msg);
        e.process();
        delete (*it).msg;
    }
    sendQueue.clear();
    if (m_send.msg){
        m_send.msg->setError(I18N_NOOP("Client go offline"));
        Event e(EventMessageSent, m_send.msg);
        e.process();
        delete m_send.msg;
    }
    m_send.msg = NULL;
    m_send.uin = 0;
}

void ICQClient::parseAdvancedMessage(unsigned long uin, Buffer &msg, bool needAck, unsigned long timestamp1, unsigned long timestamp2)
{
    msg.incReadPos(8);
    capability cap;
    msg.unpack((char*)cap, sizeof(cap));
    if (memcmp(cap, capabilities[CAP_SRV_RELAY], sizeof(cap))){
        string cap_str;
        for (unsigned i = 0; i < sizeof(cap); i++){
            char b[4];
            sprintf(b, "%02X ", cap[i]);
            cap_str += b;
        }
        log(L_DEBUG, "Unknown capability in adavansed message (%s)", cap_str.c_str());
        return;
    }

    TlvList tlv(msg);
    if (!tlv(0x2711)){
        log(L_WARN, "No found body in ICMB message");
        return;
    }

    Buffer adv(*tlv(0x2711));
    unsigned short len;
    unsigned short tcp_version;
    plugin p;

    adv.unpack(len);
    adv.unpack(tcp_version);
    adv.unpack((char*)p, sizeof(p));
    adv.incReadPos(len - sizeof(p) - 4);

    unsigned short cookie1;
    unsigned short cookie2;
    unsigned short cookie3;
    adv.unpack(cookie1);
    adv.unpack(cookie2);
    adv.unpack(cookie3);
    if ((cookie1 != cookie3) && (cookie1 + 1 != cookie3)){
        log(L_WARN, "Bad cookie in TLV 2711 (%X %X %X)", cookie1, cookie2, cookie3);
        return;
    }
    adv.unpack(len);
    adv.incReadPos(len + 10);

    if (memcmp(p, plugins[PLUGIN_NULL], sizeof(p))){
        unsigned plugin_index;
        for (plugin_index = 0; plugin_index < PLUGIN_NULL; plugin_index++)
            if (memcmp(p, plugins[plugin_index], sizeof(p)) == 0)
                break;
        if (plugin_index >= PLUGIN_NULL){
            log(L_WARN, "Unknown plugin sign");
            return;
        }
        switch (plugin_index){
        case PLUGIN_INFOxMANAGER:
        case PLUGIN_STATUSxMANAGER:
            break;
        default:
            log(L_WARN, "Bad plugin index request %u", plugin_index);
            return;
        }
        char type;
        adv.unpack(type);
        if (type != 1){
            log(L_WARN, "Unknown type plugin request %u", type);
            return;
        }
        adv.incReadPos(8);
        plugin p;
        adv.unpack((char*)p, sizeof(p));
        unsigned plugin_type;
        for (plugin_type = 0; plugin_type < PLUGIN_NULL; plugin_type++){
            if (memcmp(p, plugins[plugin_type], sizeof(p)) == 0)
                break;
        }
        if (plugin_type >= PLUGIN_NULL){
            log(L_WARN, "Unknown plugin request");
            return;
        }
        Buffer info;
        pluginAnswer(plugin_type, uin, info);
        sendAutoReply(uin, timestamp1, timestamp2, plugins[plugin_index],
                      cookie1, cookie2, 0, 0, 0x0200, NULL, 1, info);
        return;
    }

    unsigned short msgType;
    adv.unpack(msgType);
    unsigned long msgState;
    adv >> msgState;
    Buffer copy;
    switch (msgType){
    case ICQ_MSGxAR_AWAY:
    case ICQ_MSGxAR_OCCUPIED:
    case ICQ_MSGxAR_NA:
    case ICQ_MSGxAR_DND:
    case ICQ_MSGxAR_FFC:{
            unsigned req_status = STATUS_AWAY;
            switch (msgType){
            case ICQ_MSGxAR_OCCUPIED:
                req_status = STATUS_OCCUPIED;
                break;
            case ICQ_MSGxAR_NA:
                req_status = STATUS_NA;
                break;
            case ICQ_MSGxAR_DND:
                req_status = STATUS_DND;
                break;
            case ICQ_MSGxAR_FFC:
                req_status = STATUS_FFC;
                break;
            }
            Contact *contact;
            ICQUserData *data = findContact(uin, NULL, false, contact);
            if (data == NULL)
                return;
            if ((getInvisible() && (data->VisibleId == 0)) ||
                    (!getInvisible() && data->InvisibleId))
                return;
            ar_request req;
            req.uin  = uin;
            req.type = msgType;
            req.timestamp1 = timestamp1;
            req.timestamp2 = timestamp2;
            req.id1     = cookie1;
            req.id2     = cookie2;
            req.bDirect = false;
            arRequests.push_back(req);

            ARRequest ar;
            ar.contact  = contact;
            ar.param    = &arRequests.back();
            ar.receiver = this;
            ar.status   = req_status;
            Event e(EventARRequest, &ar);
            e.process();

            string msg;
            adv >> msg;

            if (!msg.empty()){
                set_str(&data->AutoReply, msg.c_str());
                Event e(EventContactChanged, contact);
                e.process();
            }
            return;
        }
    default:
        string msg;
        unsigned long real_ip = 0;
        unsigned long ip = 0;
        if (tlv(3)) real_ip = htonl((unsigned long)(*tlv(3)));
        if (tlv(4)) ip = htonl((unsigned long)(*tlv(4)));
        log(L_DEBUG, "IP: %X %X", ip, real_ip);
        adv >> msg;
        if (*msg.c_str() || (msgType == ICQ_MSGxEXT)){
            if (adv.readPos() < adv.writePos())
                copy.pack(adv.data(adv.readPos()), adv.writePos() - adv.readPos());
            log(L_DEBUG, "Msg size=%u type=%u", msg.size(), msgType);
            if (msg.size() || (msgType == ICQ_MSGxEXT)){
                Message *m = parseMessage(msgType, uin, msg, adv, cookie1, cookie2, timestamp1, timestamp2);
                if (m)
                    messageReceived(m, uin);
            }
        }
    }
    if (!needAck) return;
    sendAutoReply(uin, timestamp1, timestamp2, p, cookie1, cookie2,
                  msgType, 0, 0, NULL, 0, copy);
}

void ICQClient::sendAutoReply(unsigned long uin, unsigned long timestamp1, unsigned long timestamp2,
                              const plugin p, unsigned short cookie1, unsigned short cookie2,
                              unsigned char msgType, unsigned char msgFlags, unsigned long msgState,
                              const char *response, unsigned short response_type, Buffer &copy)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
    m_socket->writeBuffer << timestamp1 << timestamp2 << 0x0002;
    m_socket->writeBuffer.packUin(uin);
    m_socket->writeBuffer << 0x0003 << 0x1B00 << 0x0800;
    m_socket->writeBuffer.pack((char*)p, sizeof(plugin));
    m_socket->writeBuffer << 0x03000000L << (char)0;
    m_socket->writeBuffer.pack(cookie1);
    m_socket->writeBuffer.pack(cookie2);
    m_socket->writeBuffer.pack(cookie1);
    m_socket->writeBuffer
    << 0x00000000L << 0x00000000L << 0x00000000L
    << (char)msgType << (char)msgFlags << msgState;
    if (response && *response){
        Contact *contact;
        ICQUserData *data = findContact(uin, NULL, false, contact);
        string r = fromUnicode(QString::fromUtf8(response), data);
        m_socket->writeBuffer.pack((unsigned short)(r.size() + 1));
        m_socket->writeBuffer << r.c_str();
        m_socket->writeBuffer << (char)0;
    }else{
        m_socket->writeBuffer << (char)0x01 << response_type;
        if (response_type != 3){
            if (copy.size()){
                m_socket->writeBuffer.pack(copy.data(0), copy.writePos());
            }else{
                m_socket->writeBuffer << 0x00000000L << 0xFFFFFF00L;
            }
        }
    }
    sendPacket();
}

void ICQClient::sendMTN(unsigned long uin, unsigned short type)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_MTN);
    m_socket->writeBuffer << 0x00000000L << 0x00000000L << (unsigned short)0x0001;
    m_socket->writeBuffer.packUin(uin);
    m_socket->writeBuffer << type;
    sendPacket();
}

void ICQClient::processSendQueue()
{
    m_sendTimer->stop();
    if (m_send.uin){
        log(L_WARN, "Send timeout");
        if (m_send.msg){
            m_send.msg->setError(I18N_NOOP("Send timeout"));
            Event e(EventMessageSent, m_send.msg);
            e.process();
            delete m_send.msg;
        }
        m_send.msg = NULL;
        m_send.uin = 0;
        send(true);
        return;
    }
    m_sendTimer->start(30000);
    for (;;){
        if ((getState() != Connected) || sendQueue.empty()){
            m_sendTimer->stop();
            return;
        }
        m_send = sendQueue.front();
        sendQueue.pop_front();

        Contact *contact;
        ICQUserData *data = findContact(m_send.uin, NULL, false, contact);
        if ((data == NULL) && (m_send.flags != PLUGIN_RANDOMxCHAT)){
            m_send.msg->setError(I18N_NOOP("No contact"));
            Event e(EventMessageSent, m_send.msg);
            e.process();
            delete m_send.msg;
            m_send.msg = NULL;
            m_send.uin = 0;
            continue;
        }

        if (m_send.msg){
            switch (m_send.msg->type()){
            case MessageURL:
            case MessageContact:{
                    unsigned short type;
                    string message = packMessage(m_send.msg, data, type);
                    Buffer b;
                    b.pack(this->data.owner.Uin);
                    b << (char)type << (char)0;
                    b << message;
                    sendThroughServer(data->Uin, 4, b);
                    if (data->Status != ICQ_STATUS_OFFLINE)
                        ackMessage();
                    return;
                }
            case MessageCheckInvisible:{
                    Buffer msg;
                    msg.pack(ICQ_MSGxAR_AWAY);
                    msg.pack((unsigned short)(fullStatus(m_status) & 0xFFFF));
                    msg << 0x0100 << 0x0100 << (char)0;

                    m_send.id.id_l = rand();
                    m_send.id.id_h = rand();
                    sendAdvMessage(data->Uin, msg, PLUGIN_NULL, m_send.id, true);
                    return;
                }
            }
            string text;
            string encoding;
            if (data->Encoding)
                encoding = data->Encoding;
            switch (m_send.flags & SEND_MASK){
            case SEND_RTF:
                m_send.part = getRichTextPart(m_send.text, MAX_MESSAGE_SIZE);
                text = createRTF(m_send.part.utf8(), m_send.msg->getForeground(), encoding.c_str());
                break;
            case SEND_UTF:
                m_send.part = getPart(m_send.text, MAX_MESSAGE_SIZE);
                text = m_send.part.utf8();
                break;
            default:
                m_send.part = getPart(m_send.text, MAX_MESSAGE_SIZE);
                QTextCodec *codec = getCodec(encoding.c_str());
                string msg_text;
                msg_text = codec->fromUnicode(m_send.part);
                Buffer msgBuf;
                msgBuf << 0x0000L;
                msgBuf << msg_text.c_str();
                Buffer b;
                b.tlv(0x0501, "\x01", 1);
                b.tlv(0x0101, msgBuf);
                sendThroughServer(m_send.uin, 1, b);
                if (data->Status != ICQ_STATUS_OFFLINE)
                    ackMessage();
                return;
            }

            Buffer msgBuf;
            unsigned short size = text.length() + 1;
            msgBuf.pack((unsigned short)1);
            msgBuf.pack((unsigned short)(fullStatus(m_status) & 0xFFFF));
            msgBuf.pack((unsigned short)0x21);
            msgBuf.pack(size);
            msgBuf.pack(text.c_str(), size);
            if (m_send.msg->getBackground() == m_send.msg->getForeground()){
                msgBuf << 0x00000000L << 0xFFFFFF00L;
            }else{
                msgBuf << (m_send.msg->getForeground() << 8) << (m_send.msg->getBackground() << 8);
            }
            msgBuf << 0x26000000L;
            packCap(msgBuf, capabilities[((m_send.flags & SEND_MASK) == SEND_RTF) ? CAP_RTF : CAP_UTF]);
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            sendAdvMessage(m_send.uin, msgBuf, PLUGIN_NULL, m_send.id);
            return;
        }
        if (m_send.flags == PLUGIN_AR){
            log(L_DEBUG, "Request auto response %lu", m_send.uin);

            unsigned long status = data->Status;
            if ((status == ICQ_STATUS_ONLINE) || (status == ICQ_STATUS_OFFLINE))
                continue;

            unsigned short type = ICQ_MSGxAR_AWAY;
            if (status & ICQ_STATUS_DND){
                type = ICQ_MSGxAR_DND;
            }else if (status & ICQ_STATUS_OCCUPIED){
                type = ICQ_MSGxAR_OCCUPIED;
            }else if (status & ICQ_STATUS_NA){
                type = ICQ_MSGxAR_NA;
            }else if (status & ICQ_STATUS_FFC){
                type = ICQ_MSGxAR_FFC;
            }

            Buffer msg;
            msg.pack(type);
            msg.pack((unsigned short)(fullStatus(m_status) & 0xFFFF));
            msg << 0x0100 << 0x0100 << (char)0;

            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            sendAdvMessage(data->Uin, msg, PLUGIN_NULL, m_send.id);
            return;
        }else if (m_send.flags == PLUGIN_RANDOMxCHAT){
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            Buffer b;
            b << (char)1 << 0x00000000L << 0x00010000L;
            sendAdvMessage(m_send.uin, b, PLUGIN_RANDOMxCHAT, m_send.id);
        }else{
            unsigned plugin_index = m_send.flags;
            log(L_DEBUG, "Plugin info request %lu (%u)", m_send.uin, plugin_index);

            Buffer b;
            unsigned short type = 0;
            switch (plugin_index){
            case PLUGIN_QUERYxINFO:
            case PLUGIN_PHONEBOOK:
            case PLUGIN_PICTURE:
                type = 2;
                break;
            }
            b.pack((unsigned short)1);
            b.pack((unsigned short)0);
            b.pack((unsigned short)2);
            b.pack((unsigned short)1);
            b.pack((char)0);
            b.pack((char*)plugins[plugin_index], sizeof(plugin));
            b.pack((unsigned long)0);

            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            sendAdvMessage(m_send.uin, b, type ? PLUGIN_INFOxMANAGER : PLUGIN_STATUSxMANAGER, m_send.id);
            return;
        }
    }
}

string ICQClient::packMessage(Message *msg, ICQUserData *data, unsigned short &type)
{
    string res;
    switch (msg->type()){
    case MessageURL:
        res = fromUnicode(msg->getPlainText(), data);
        res += (char)0xFE;
        res += fromUnicode(static_cast<URLMessage*>(msg)->getUrl(), data);
        type = ICQ_MSGxURL;
        break;
    case MessageContact:{
            unsigned nContacts = 0;
            QString contacts = static_cast<ContactMessage*>(msg)->getContacts();
            while (!contacts.isEmpty()){
                QString contact = getToken(contacts, ';');
                nContacts++;
            }
            res = number(nContacts);
            contacts = static_cast<ContactMessage*>(msg)->getContacts();
            while (!contacts.isEmpty()){
                QString contact = getToken(contacts, ';');
                QString uin = getToken(contact, ',');
                res += (char)0xFE;
                res += uin.latin1();
                res += (char)0xFE;
                res += fromUnicode(contact, data);
            }
            res += (char)0xFE;
            type = ICQ_MSGxCONTACTxLIST;
            break;
        }
    case MessageOpenSecure:
        type = ICQ_MSGxSECURExOPEN;
        break;
    case MessageCloseSecure:
        type = ICQ_MSGxSECURExCLOSE;
        break;
    }
    return res;
}

void ICQClient::send(bool bTimer)
{
    if (sendQueue.size() == 0){
        if (m_sendTimer->isActive() && (m_send.uin == 0))
            m_sendTimer->stop();
        return;
    }
    if (m_send.uin)
        return;
    if (!bTimer){
        processSendQueue();
        return;
    }
    if (!m_sendTimer->isActive())
        m_sendTimer->start(m_nSendTimeout * 500);
}

static const plugin arrPlugins[] =
    {
        // PLUGIN_PHONExBOOK
        { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D,
          0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04,
          0xAC, 0x96, 0xAA, 0xB2, 0x00, 0x00 },
        // PLUGIN_PICTURE
        { 0x80, 0x66, 0x28, 0x83, 0x80, 0x28,
          0xD3, 0x11, 0x8D, 0xBB, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_FILExSERVER
        { 0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91,
          0xD3, 0x11, 0x8D, 0xD7, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x04, 0x00 },
        // PLUGIN_FOLLOWxME
        { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D,
          0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04,
          0xAC, 0x96, 0xAA, 0xB2, 0x02, 0x00 },
        // PLUGIN_ICQxPHONE
        { 0x3F, 0xB6, 0x5E, 0x38, 0xA0, 0x30,
          0xD4, 0x11, 0xBD, 0x0F, 0x00, 0x06,
          0x29, 0xEE, 0x4D, 0xA1, 0x00, 0x00 },
        // PLUGIN_QUERYxINFO
        { 0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71,
          0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_QUERYxSTATUS
        { 0x10, 0x18, 0x06, 0x70, 0x54, 0x71,
          0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_INFOxMANAGER
        { 0xA0, 0xE9, 0x3F, 0x37, 0x4F, 0xE9,
          0xD3, 0x11, 0xBC, 0xD2, 0x00, 0x04,
          0xAC, 0x96, 0xDD, 0x96, 0x00, 0x00 },
        // PLUGIN_STATUSxMANAGER
        { 0x10, 0xCF, 0x40, 0xD1, 0x4F, 0xE9,
          0xD3, 0x11, 0xBC, 0xD2, 0x00, 0x04,
          0xAC, 0x96, 0xDD, 0x96, 0x00, 0x00 },
        // PLUGIN_RANDOM_CHAT
        { 0x60, 0xF1, 0xA8, 0x3D, 0x91, 0x49,
          0xD3, 0x11, 0x8D, 0xBE, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_NULL
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
    };

plugin const *ICQClient::plugins = arrPlugins;

bool operator == (const MessageId &m1, const MessageId &m2)
{
    return ((m1.id_l == m2.id_l) && (m1.id_h == m2.id_h));
}
