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
#include "html.h"

#include <stdio.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#endif

#include <qtextcodec.h>
#include <qfile.h>
#include <qtimer.h>
#include <qimage.h>
#include <qregexp.h>

bool operator < (const alias_group &s1, const alias_group &s2)
{
    return s1.grp < s2.grp;
}

const unsigned short ICQ_SNACxMSG_ERROR            = 0x0001;
const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002;
const unsigned short ICQ_SNACxMSG_RESETxICQxMODE   = 0x0003;    // not implemented
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004;
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005;
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006;
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007;
const unsigned short ICQ_SNACxMSG_BLAMExUSER       = 0x0008;
const unsigned short ICQ_SNACxMSG_BLAMExSRVxACK    = 0x0009;
const unsigned short ICQ_SNACxMSG_SRV_MISSED_MSG   = 0x000A;
const unsigned short ICQ_SNACxMSG_AUTOREPLY        = 0x000B;
const unsigned short ICQ_SNACxMSG_ACK              = 0x000C;
const unsigned short ICQ_SNACxMSG_MTN			   = 0x0014;

void ICQClient::snac_icmb(unsigned short type, unsigned short seq)
{
    switch (type){
    case ICQ_SNACxMSG_RIGHTSxGRANTED:
        log(L_DEBUG, "Message rights granted");
        break;
    case ICQ_SNACxMSG_MTN:{
            m_socket->readBuffer.incReadPos(10);
            string screen = m_socket->readBuffer.unpackScreen();
            unsigned short type;
            m_socket->readBuffer >> type;
            bool bType = (type > 1);
            Contact *contact;
            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
            if (data == NULL)
                break;
            if (data->bTyping.bValue == bType)
                break;
            data->bTyping.bValue = bType;
            Event e(EventContactStatus, contact);
            e.process();
            break;
        }
    case ICQ_SNACxMSG_ERROR:{
            unsigned short error;
            m_socket->readBuffer >> error;
            const char *err_str = I18N_NOOP("Unknown error");
            if ((error == 0x0009) && ((m_send.msg == NULL) || (m_send.msg->type() != MessageContacts))){
                err_str = I18N_NOOP("Not supported by client");
                Contact *contact;
                ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                if (data){
					list<SendMsg>::iterator it;
                    for (it = sendFgQueue.begin(); it != sendFgQueue.end();){
                        if ((*it).screen != m_send.screen){
                            ++it;
                            continue;
                        }
                        if ((*it).msg){
                            (*it).flags = 0;
                            ++it;
                            continue;
                        }
                        sendFgQueue.erase(it);
                        it = sendFgQueue.begin();
                    }
                    for (it = sendBgQueue.begin(); it != sendBgQueue.end();){
                        if ((*it).screen != m_send.screen){
                            ++it;
                            continue;
                        }
                        if ((*it).msg){
                            (*it).flags = 0;
                            ++it;
                            continue;
                        }
                        sendBgQueue.erase(it);
                        it = sendBgQueue.begin();
                    }
                    data->bBadClient.bValue = true;
                    if (m_send.msg)
                        sendThruServer(m_send.msg, data);
                    m_send.msg    = NULL;
                    m_send.screen = "";
                    m_sendTimer->stop();
                    send(true);
                    break;
                }
            }else{
                err_str = error_message(error);
            }
            if (m_send.msg){
                if ((m_send.msg->type() == MessageCheckInvisible) && (error == 14)) {
                    Contact *contact;
                    ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                    if (data && !data->bInvisible.bValue) {
                        data->bInvisible.bValue = true;
                        Event e(EventContactStatus, contact);
                        e.process();
                    }
                }else{
                    m_send.msg->setError(err_str);
                    Event e(EventMessageSent, m_send.msg);
                    e.process();
                }
                delete m_send.msg;
            }
            m_send.msg    = NULL;
            m_send.screen = "";
            m_sendTimer->stop();
            send(true);
            break;
        }
    case ICQ_SNACxMSG_SRV_MISSED_MSG: {
            unsigned short mFormat; // missed channel
            string screen;			// screen
            unsigned short wrnLevel;// warning level
            unsigned short nTlv;    // number of tlvs
            TlvList  lTlv;          // all tlvs in message
            unsigned short missed;  // number of missed messages
            unsigned short error;   // error reason
            m_socket->readBuffer >> mFormat;
            screen = m_socket->readBuffer.unpackScreen();
            m_socket->readBuffer >> wrnLevel;
            m_socket->readBuffer >> nTlv;
            for(unsigned i = 0; i < nTlv; i++) {
                unsigned short num;
                unsigned short size;
                const char*    data;
                m_socket->readBuffer >> num >> size;
                data = m_socket->readBuffer.data(m_socket->readBuffer.readPos());
                Tlv* tlv = new Tlv(num,size,data);
                lTlv = lTlv + tlv;
            }
            m_socket->readBuffer >> missed >> error;
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
            log(L_DEBUG, "ICMB error %u (%s) - screen(%s)", error, err_str, screen.c_str());
            break;
        }
    case ICQ_SNACxMSG_BLAMExSRVxACK:
        if ((m_send.id.id_l == seq) && m_send.msg){
            unsigned short oldLevel, newLevel;
            m_socket->readBuffer >> oldLevel >> newLevel;
            WarningMessage *msg = static_cast<WarningMessage*>(m_send.msg);
            msg->setOldLevel((unsigned short)(newLevel - oldLevel));
            msg->setNewLevel(newLevel);
            ackMessage(m_send);
        }
        break;
    case ICQ_SNACxMSG_ACK:
        {
            log(L_DEBUG, "Ack message");
            MessageId id;
            m_socket->readBuffer >> id.id_l >> id.id_h;
            m_socket->readBuffer.incReadPos(2);
            string screen = m_socket->readBuffer.unpackScreen();
            bool bAck = false;
            if (m_send.id == id){
                const char *p1 = screen.c_str();
                const char *p2 = m_send.screen.c_str();
                for (; *p1 && *p2; p1++, p2++)
                    if (tolower(*p1) != tolower(*p2))
                        break;
                if ((*p1 == 0) && (*p2 == 0))
                    bAck = true;
            }
            if (bAck){
                log(L_DEBUG, "Ack: %u %u (%s)", m_send.id.id_h, m_send.id.id_l, m_send.screen.c_str());
                if (m_send.msg){
                    if (m_send.msg->type() == MessageCheckInvisible){
                        Contact *contact;
                        ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                        if (data && data->bInvisible.bValue) {
                            data->bInvisible.bValue = false;
                            Event e(EventContactStatus, contact);
                            e.process();
                        }
                        delete m_send.msg;
                    }else{
                        Contact *contact;
                        ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
                        if (((data == NULL) || (data->Status.value == ICQ_STATUS_OFFLINE) || (getAckMode() == 1)) &&
                                (m_send.msg->type() != MessageFile)){
                            ackMessage(m_send);
                        }else{
                            replyQueue.push_back(m_send);
                        }
                    }
                }else{
                    replyQueue.push_back(m_send);
                }
            }
            m_send.msg    = NULL;
            m_send.screen = "";
            m_sendTimer->stop();
            send(true);
            break;
        }
    case ICQ_SNACxMSG_AUTOREPLY:{
            MessageId id;
            m_socket->readBuffer >> id.id_l >> id.id_h;
            m_socket->readBuffer.incReadPos(2);
            string screen = m_socket->readBuffer.unpackScreen();
            m_socket->readBuffer.incReadPos(2);
            unsigned short len;
            m_socket->readBuffer.unpack(len);
            m_socket->readBuffer.incReadPos(2);
            plugin p;
            m_socket->readBuffer.unpack((char*)p, sizeof(p));
            m_socket->readBuffer.incReadPos(len - sizeof(plugin) + 2);
            m_socket->readBuffer.unpack(len);
            m_socket->readBuffer.incReadPos(len + 12);
            unsigned short ackFlags, msgFlags;
            m_socket->readBuffer.unpack(ackFlags);
            m_socket->readBuffer.unpack(msgFlags);

            list<SendMsg>::iterator it;
            for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                SendMsg &s = *it;
                if ((s.id == id) && (s.screen == screen))
                    break;
            }
            if (it == replyQueue.end())
                break;

            unsigned plugin_type = (*it).flags;
            if (plugin_type == PLUGIN_AIM_FT_ACK){
                m_processMsg.push_back((*it).msg);
                replyQueue.erase(it);
                break;
            }
            if ((*it).msg){
                string answer;
                m_socket->readBuffer >> answer;
                if (ackMessage((*it).msg, ackFlags, answer.c_str())){
                    ackMessage(*it);
                }else{
                    Event e(EventMessageSent, (*it).msg);
                    e.process();
                    delete (*it).msg;
                }
                replyQueue.erase(it);
                break;
            }

            replyQueue.erase(it);
            Contact *contact;
            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);

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
                if ((data == NULL) && (plugin_index != PLUGIN_RANDOMxCHAT))
                    break;
                parsePluginPacket(m_socket->readBuffer, plugin_type, data, atol(screen.c_str()), false);
                break;
            }

            if (plugin_type == PLUGIN_AR){
                string answer;
                m_socket->readBuffer >> answer;
                log(L_DEBUG, "Autoreply from %s %s", screen.c_str(), answer.c_str());
                Contact *contact;
                ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
                if (data && set_str(&data->AutoReply.ptr, answer.c_str())){
                    Event e(EventContactChanged, contact);
                    e.process();
                }
            }
            break;
        }
    case ICQ_SNACxMSG_SERVERxMESSAGE:{
            MessageId id;
            m_socket->readBuffer >> id.id_l >> id.id_h;
            unsigned short mFormat;
            m_socket->readBuffer >> mFormat;
            string screen = m_socket->readBuffer.unpackScreen();
            log(L_DEBUG, "Message from %s [%04X]", screen.c_str(), mFormat);
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
                        log(L_WARN, "generic message tlv 0101 not found");
                        break;
                    }
                    if (m_tlv->Size() <= 4){
                        log(L_WARN, "Bad tlv 0101 size");
                        break;
                    }
                    char *m_data = (*m_tlv);
                    unsigned short encoding = (unsigned short)((m_data[0] << 8) + m_data[1]);
                    m_data += 4;
                    if (encoding == 2){
                        QString text;
                        for (int i = 0; i < m_tlv->Size() - 5; i += 2){
                            unsigned char r1 = *(m_data++);
                            unsigned char r2 = *(m_data++);
                            unsigned short c = (unsigned short)((r1 << 8) + r2);
                            text += QChar(c);
                        }
                        Message *msg = new Message(MessageGeneric);
                        if (atol(screen.c_str())){
                            msg->setText(text);
                        }else{
                            unsigned bgColor = clearTags(text);
                            msg->setText(text);
                            msg->setBackground(bgColor);
                            msg->setFlags(MESSAGE_RICHTEXT);
                        }
                        string s;
                        if (!text.isEmpty())
                            s = text.local8Bit();
                        log(L_DEBUG, "Message %s", s.c_str());
                        messageReceived(msg, screen.c_str());
                        break;
                    }
                    log(L_DEBUG, "ServerText: %s", m_data);
                    Message *msg = new Message;
                    if (!m_bAIM && atol(screen.c_str())){
                        msg->setServerText(m_data);
                    }else{
                        QString text = m_data;
                        unsigned bgColor = clearTags(text);
                        msg->setServerText(m_data);
                        msg->setBackground(bgColor);
                        msg->setFlags(MESSAGE_RICHTEXT);
                    }
                    messageReceived(msg, screen.c_str());
                    break;
                }
            case 0x0002:{
                    TlvList tlv(m_socket->readBuffer);
                    if (!tlv(5)){
                        log(L_WARN, "ICMB message tlv5 not found");
                        break;
                    }
                    Buffer msg(*tlv(5));
                    unsigned short type;
                    msg >> type;
                    switch (type){
                    case 0:
                        parseAdvancedMessage(screen.c_str(), msg, tlv(3) != NULL, id);
                        break;
                    case 1:{
                            Contact *contact;
                            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
                            if (data){
                                string name = dataName(data);
                                for (list<Message*>::iterator it = m_acceptMsg.begin(); it != m_acceptMsg.end(); ++it){
                                    Message *msg = *it;
                                    if (msg->client() && (name == msg->client())){
                                        MessageId msg_id;
                                        switch (msg->type()){
                                        case MessageICQFile:
                                            msg_id.id_l = static_cast<ICQFileMessage*>(msg)->getID_L();
                                            msg_id.id_h = static_cast<ICQFileMessage*>(msg)->getID_H();
                                            break;
                                        case MessageFile:
                                            msg_id.id_l = static_cast<AIMFileMessage*>(msg)->getID_L();
                                            msg_id.id_h = static_cast<AIMFileMessage*>(msg)->getID_H();
                                            break;
                                        }
                                        if (msg_id == id){
                                            m_acceptMsg.erase(it);
                                            Event e(EventMessageDeleted, msg);
                                            e.process();
                                            delete msg;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }
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
                        log(L_WARN, "Advanced message tlv5 not found");
                        break;
                    }
                    Buffer msg(*tlv(5));
                    unsigned long msg_uin;
                    msg >> msg_uin;
                    if (msg_uin == 0){
                        parseAdvancedMessage(screen.c_str(), msg, tlv(6) != NULL, id);
                        return;
                    }
                    unsigned char type, flags;
                    msg >> type;
                    msg >> flags;
                    string msg_str;
                    msg >> msg_str;
                    Message *m = parseMessage(type, screen.c_str(), msg_str, msg, id, 0);
                    if (m)
                        messageReceived(m, screen.c_str());
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
    sendPacket(true);
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
    sendPacket(true);
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
        if ((data->Status.value != ICQ_STATUS_OFFLINE) && (getSendFormat() == 0) &&
                hasCap(data, CAP_RTF) && (msg->getFlags() & MESSAGE_RICHTEXT) &&
                !data->bBadClient.bValue){
            s.flags  = SEND_RTF;
            s.msg    = msg;
            s.text   = msg->getRichText();
            s.screen = screen(data);
            sendFgQueue.push_front(s);
            send(false);
            return true;
        }
        if ((data->Status.value != ICQ_STATUS_OFFLINE) &&
                (getSendFormat() <= 1) &&
                hasCap(data, CAP_UTF) &&
                ((msg->getFlags() & MESSAGE_SECURE) == 0) &&
                (data->Version.value >= 8) && !data->bBadClient.bValue){
            s.flags  = SEND_UTF;
            s.msg    = msg;
            s.text   = addCRLF(msg->getPlainText());
            s.screen = screen(data);
            sendFgQueue.push_front(s);
            send(false);
            return true;
        }
        if ((data->Status.value != ICQ_STATUS_OFFLINE) &&
                (data->Version.value >= 8) &&
                !data->bBadClient.bValue &&
                (msg->getPlainText().length() >= MAX_PLAIN_MESSAGE_SIZE)){
            s.flags  = SEND_TYPE2;
            s.msg    = msg;
            s.text   = msg->getPlainText();
            s.screen = screen(data);
            sendFgQueue.push_front(s);
            send(false);
            return true;
        }
        if ((data->Uin.value == 0) || m_bAIM ||
                (hasCap(data, CAP_AIM_BUDDYCON) && !hasCap(data, CAP_AIM_CHAT) &&
                 !hasCap(data, CAP_LICQ) && ((data->InfoUpdateTime.value & 0xFF7F0000L) != 0x7D000000L))){
            s.msg	 = msg;
            if (msg->getFlags() & MESSAGE_RICHTEXT){
                s.flags  = SEND_HTML;
                s.text	 = removeImages(msg->getRichText(), 0);
            }else{
                s.flags  = SEND_HTML_PLAIN;
                s.text	 = msg->getPlainText();
            }
            s.screen = screen(data);
            sendFgQueue.push_front(s);
            send(false);
            return true;
        }
        s.flags	 = SEND_PLAIN;
        s.msg	 = msg;
        s.text	 = addCRLF(msg->getPlainText());
        s.screen = screen(data);
        sendFgQueue.push_front(s);
        send(false);
        return true;
    case MessageUrl:
        if ((data->Uin.value == 0) || m_bAIM ||
                (hasCap(data, CAP_AIM_BUDDYCON) && !hasCap(data, CAP_AIM_CHAT) &&
                 !hasCap(data, CAP_LICQ) && ((data->InfoUpdateTime.value & 0xFF7F0000L) != 0x7D000000L))){
            UrlMessage *m = static_cast<UrlMessage*>(msg);
            QString text = "<a href=\"";
            text += m->getUrl();
            text += "\">";
            text += m->getUrl();
            text += "</a><br>";
            text += removeImages(msg->getRichText(), 0);
            s.flags  = SEND_HTML;
            s.msg	 = msg;
            s.text	 = text;
            s.screen = screen(data);
            sendFgQueue.push_front(s);
            send(false);
            return true;
        }
    case MessageContacts:
    case MessageFile:
    case MessageCheckInvisible:
    case MessageWarning:
        s.flags  = SEND_RAW;
        s.msg    = msg;
        s.screen = screen(data);
		if (msg->type() == MessageCheckInvisible){
			sendBgQueue.push_front(s);
		}else{
			sendFgQueue.push_front(s);
		}
        send(false);
        return true;
    }
    return false;
}

void ICQClient::sendThroughServer(const char *screen, unsigned short channel, Buffer &b, const MessageId &id, bool bOffline, bool bReqAck)
{
    // we need informations about channel 2 tlvs !
    unsigned short tlv_type = 5;
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER);
    m_socket->writeBuffer << id.id_l << id.id_h;
    m_socket->writeBuffer << channel;
    m_socket->writeBuffer.packScreen(screen);
    if (channel == 1)
        tlv_type = 2;
    if (b.size())
        m_socket->writeBuffer.tlv(tlv_type, b);
    if (bReqAck)
        m_socket->writeBuffer.tlv(3);		// req. ack from server
    if (bOffline)
        m_socket->writeBuffer.tlv(6);	// store if user is offline
    sendPacket(true);
}

static char c2h(char c)
{
    c = (char)(c & 0xF);
    if (c < 10)
        return (char)('0' + c);
    return (char)('A' + c - 10);
}

static void b2h(char *&p, char c)
{
    *(p++) = c2h((char)(c >> 4));
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

void ICQClient::ackMessage(SendMsg &s)
{
    if (s.flags == PLUGIN_AIM_FT){
        s.msg->setError(I18N_NOOP("File transfer declined"));
        Event e(EventMessageSent, s.msg);
        e.process();
        delete s.msg;
        s.msg = NULL;
        s.screen = "";
        send(true);
        return;
    }
    if ((s.msg->getFlags() & MESSAGE_NOHISTORY) == 0){
        if ((s.flags & SEND_MASK) == SEND_RAW){
            s.msg->setClient(dataName(m_send.screen.c_str()).c_str());
            Event e(EventSent, s.msg);
            e.process();
        }else if (!s.part.isEmpty()){
            Message m(MessageGeneric);
            m.setContact(s.msg->contact());
            m.setBackground(s.msg->getBackground());
            m.setForeground(s.msg->getForeground());
            unsigned flags = s.msg->getFlags() & (~MESSAGE_RICHTEXT);
            if ((s.flags & SEND_MASK) == SEND_RTF){
                flags |= MESSAGE_RICHTEXT;
                m.setText(removeImages(s.part, 16));
            }else if ((s.flags & SEND_MASK) == SEND_HTML){
                flags |= MESSAGE_RICHTEXT;
                m.setText(removeImages(s.part, 0));
            }else{
                m.setText(s.part);
            }
            m.setFlags(flags);
            m.setClient(dataName(s.screen.c_str()).c_str());
            Event e(EventSent, &m);
            e.process();
        }
    }
    string text;
    if ((s.text.length() == 0) || (s.msg->type() == MessageWarning)){
        Event e(EventMessageSent, s.msg);
        e.process();
        delete s.msg;
        s.msg = NULL;
        s.screen = "";
    }else{
        sendBgQueue.push_front(s);
    }
    send(true);
}

bool ICQClient::ackMessage(Message *msg, unsigned short ackFlags, const char *str)
{
    string msg_str;
    if (str)
        msg_str = str;
    switch (ackFlags){
    case ICQ_TCPxACK_OCCUPIED:
    case ICQ_TCPxACK_DND:
    case ICQ_TCPxACK_REFUSE:
        if (*msg_str.c_str() == 0)
            msg_str = I18N_NOOP("Message declined");
        msg->setError(msg_str.c_str());
        switch (ackFlags){
        case ICQ_TCPxACK_OCCUPIED:
            msg->setRetryCode(static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendOccupied);
            break;
        case ICQ_TCPxACK_DND:
            msg->setRetryCode(static_cast<ICQPlugin*>(protocol()->plugin())->RetrySendDND);
            break;
        }
        return false;
    }
    return true;
}

void ICQClient::sendAdvMessage(const char *screen, Buffer &msgText, unsigned plugin_index, const MessageId &id, bool bOffline, bool bDirect, unsigned short cookie1, unsigned short cookie2, unsigned short type)
{
    if (cookie1 == 0){
        m_advCounter--;
        cookie1 = m_advCounter;
        cookie2 = (plugin_index == PLUGIN_NULL) ? (unsigned short)0x0E : (unsigned short)0x12;
    }
    Buffer msgBuf;
    msgBuf.pack((unsigned short)0x1B);
    msgBuf.pack((unsigned short)0x08);
    msgBuf.pack((char*)plugins[plugin_index], sizeof(plugin));
    msgBuf.pack(0x00000003L);
    msgBuf.pack((char)(type ? 4 : 0));
    msgBuf.pack(cookie1);
    msgBuf.pack(cookie2);
    msgBuf.pack(cookie1);
    msgBuf.pack(0x00000000L);
    msgBuf.pack(0x00000000L);
    msgBuf.pack(0x00000000L);
    msgBuf.pack(msgText.data(0), msgText.size());
    sendType2(screen, msgBuf, id, CAP_SRV_RELAY, bOffline, bDirect ? data.owner.Port.value : 0, NULL, type);
}

static void copyTlv(Buffer &b, TlvList *tlvs, unsigned nTlv)
{
    if (tlvs == NULL)
        return;
    Tlv *tlv = (*tlvs)(nTlv);
    if (tlv == NULL)
        return;
    b.tlv(nTlv, *tlv, tlv->Size());
}

void ICQClient::sendType2(const char *screen, Buffer &msgBuf, const MessageId &id, unsigned cap, bool bOffline, unsigned short port, TlvList *tlvs, unsigned short type)
{
    Buffer b;
    b << (unsigned short)0;
    b << id.id_l << id.id_h;
    b.pack((char*)capabilities[cap], sizeof(capability));
    b.tlv(0x0A, (unsigned short)type);
    if (port){
        b.tlv(0x03, (unsigned long)htonl(get_ip(data.owner.RealIP)));
        b.tlv(0x04, (unsigned long)htonl(get_ip(data.owner.IP)));
        b.tlv(0x05, port);
    }
    b.tlv(0x0F);
    copyTlv(b, tlvs, 0x0E);
    copyTlv(b, tlvs, 0x0D);
    copyTlv(b, tlvs, 0x0C);
    b.tlv(0x2711, msgBuf);
    copyTlv(b, tlvs, 0x2712);
    copyTlv(b, tlvs, 0x03);
    sendThroughServer(screen, 2, b, id, bOffline, true);
}

void ICQClient::clearMsgQueue()
{
	list<SendMsg>::iterator it;
    for (it = sendFgQueue.begin(); it != sendFgQueue.end(); ++it){
        if ((*it).socket){
            // dunno know if this is ok - vladimir please take a look
            (*it).socket->acceptReverse(NULL);
            continue;
        }
        if ((*it).msg) {
            (*it).msg->setError(I18N_NOOP("Client go offline"));
            Event e(EventMessageSent, (*it).msg);
            e.process();
            delete (*it).msg;
        }
    }
    sendFgQueue.clear();
    for (it = sendBgQueue.begin(); it != sendBgQueue.end(); ++it){
        if ((*it).socket){
            // dunno know if this is ok - vladimir please take a look
            (*it).socket->acceptReverse(NULL);
            continue;
        }
        if ((*it).msg) {
            (*it).msg->setError(I18N_NOOP("Client go offline"));
            Event e(EventMessageSent, (*it).msg);
            e.process();
            delete (*it).msg;
        }
    }
    sendBgQueue.clear();
    if (m_send.msg){
        m_send.msg->setError(I18N_NOOP("Client go offline"));
        Event e(EventMessageSent, m_send.msg);
        e.process();
        delete m_send.msg;
    }
    m_send.msg    = NULL;
    m_send.screen = "";
}

void ICQClient::parseAdvancedMessage(const char *screen, Buffer &m, bool needAck, MessageId id)
{
    m.incReadPos(8);
    capability cap;
    m.unpack((char*)cap, sizeof(cap));
    if (!memcmp(cap, capabilities[CAP_DIRECT], sizeof(cap))){
        TlvList tlv(m);
        if (!tlv(0x2711)){
            log(L_DEBUG, "No 2711 tlv in direct message");
            return;
        }
        unsigned long req_uin;
        unsigned long localIP;
        unsigned long localPort;
        unsigned long remotePort;
        unsigned long localPort1;
        char mode;
        Buffer adv(*tlv(0x2711));
        adv.unpack(req_uin);
        adv.unpack(localIP);
        adv.unpack(localPort);
        adv.unpack(mode);
        adv.unpack(remotePort);
        adv.unpack(localPort1);
        if (req_uin != (unsigned)atol(screen)){
            log(L_WARN, "Bad UIN in reverse direct request");
            return;
        }
        Contact *contact;
        ICQUserData *data = findContact(screen, NULL, false, contact);
        if ((data == NULL) || contact->getIgnore()){
            log(L_DEBUG, "Reverse direct request from unknown user");
            return;
        }
        if (get_ip(data->RealIP) == 0)
            set_ip(&data->RealIP, localIP);
        in_addr addr;
        addr.s_addr = localIP;
        for (list<Message*>::iterator it = m_processMsg.begin(); it != m_processMsg.end(); ++it){
            if ((*it)->type() != MessageICQFile)
                continue;
            ICQFileMessage *msg = static_cast<ICQFileMessage*>(*it);
            if (msg->m_transfer == NULL)
                continue;
            ICQFileTransfer *ft = static_cast<ICQFileTransfer*>(msg->m_transfer);
            if (ft->m_localPort == remotePort){
                log(L_DEBUG, "Setup file transfer reverse connect to %s %s:%u", screen, inet_ntoa(addr), localPort);
                ft->reverseConnect(localIP, (unsigned short)localPort);
                return;
            }
        }
        log(L_DEBUG, "Setup reverse connect to %s %s:%u", screen, inet_ntoa(addr), localPort);
        DirectClient *direct = new DirectClient(data, this);
        m_sockets.push_back(direct);
        direct->reverseConnect(localIP, (unsigned short)localPort);
        return;
    }

    TlvList tlv(m);
    unsigned long real_ip = 0;
    unsigned long ip = 0;
    unsigned short port = 0;
    if (tlv(3)) real_ip = htonl((unsigned long)(*tlv(3)));
    if (tlv(4)) ip = htonl((unsigned long)(*tlv(4)));
    if (tlv(5)) port = *tlv(5);
    log(L_DEBUG, "IP: %X %X %u", ip, real_ip, port);
    if (real_ip || ip){
        Contact *contact;
        ICQUserData *data = findContact(screen, NULL, false, contact);
        if (data){
            if (real_ip && (get_ip(data->RealIP) == 0))
                set_ip(&data->RealIP, real_ip);
            if (ip && (get_ip(data->IP) == 0))
                set_ip(&data->IP, ip);
            if (port && (data->Port.value == 0))
                data->Port.value = port;
        }
    }

    if (!memcmp(cap, capabilities[CAP_AIM_IMIMAGE], sizeof(cap))){
        log(L_DEBUG, "AIM set direct connection");
        return;
    }

    if (!memcmp(cap, capabilities[CAP_AIM_SENDFILE], sizeof(cap))){
        log(L_DEBUG, "AIM send file");
        Tlv *desc = tlv(0x0C);
        if (desc == NULL){
            log(L_DEBUG, "Send file ack");
            list<SendMsg>::iterator it;
            for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                if (((*it).id == id) && ((*it).screen == screen)){
                    if ((*it).msg->type() != MessageFile){
                        log(L_WARN, "Bad message type for file ack");
                        return;
                    }
                    FileMessage *m = static_cast<FileMessage*>((*it).msg);
                    replyQueue.erase(it);
                    m_processMsg.push_back(m);
                    Event e(EventMessageAcked, m);
                    e.process();
                    AIMFileTransfer *ft = static_cast<AIMFileTransfer*>(m->m_transfer);
                    ft->connect(port);
                    return;
                }
            }
            log(L_DEBUG, "File message for ack not found");
            return;
        }
        Tlv *info = tlv(0x2711);
        if (info == NULL){
            log(L_WARN, "No info tlv in send file");
            return;
        }
        QString d = convert(desc, tlv, 0x0D);
        Buffer b(*info);
        unsigned short type;
        unsigned short nFiles;
        unsigned long  size;
        b >> type >> nFiles >> size;
        QString name = convert(b.data(8), b.size() - 8, tlv, 0x2712);
        AIMFileMessage *msg = new AIMFileMessage;
        msg->setPort(port);
        msg->setBackground(clearTags(d));
        msg->setText(d);
        msg->setSize(size);
        msg->setID_L(id.id_l);
        msg->setID_H(id.id_h);
        if (type == 2){
            d = i18n("Directory");
            d += " ";
            d += name;
            d += " (";
            d += i18n("%n file", "%n files", nFiles);
            d += ")";
        }else{
            if (nFiles == 1){
                d = name;
            }else{
                d = i18n("%n file", "%n files", nFiles);
            }
        }
        msg->setDescription(d);
        msg->setFlags(MESSAGE_RECEIVED | MESSAGE_RICHTEXT | MESSAGE_TEMP);
        messageReceived(msg, screen);
        return;
    }

    if (!memcmp(cap, capabilities[CAP_AIM_BUDDYLIST], sizeof(cap))){
        log(L_DEBUG, "AIM buddies list");
        if (!tlv(0x2711)){
            log(L_WARN, "No found body in ICMB message");
            return;
        }
        Buffer adv(*tlv(0x2711));
        QString contacts;
        while (adv.readPos() < adv.size()){
            string grp;
            adv.unpackStr(grp);
            unsigned short nBuddies;
            adv >> nBuddies;
            for (unsigned short i = 0; i < nBuddies; i++){
                string s;
                adv.unpackStr(s);
                if (!contacts.isEmpty())
                    contacts += ";";
                if (atol(s.c_str())){
                    contacts += "icq:";
                    contacts += s.c_str();
                    contacts += ",ICQ ";
                    contacts += s.c_str();
                }else{
                    contacts += "aim:";
                    contacts += s.c_str();
                    contacts += ",AIM ";
                    contacts += s.c_str();
                }
            }
        }
        snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
        m_socket->writeBuffer << id.id_l << id.id_h << 0x0002;
        m_socket->writeBuffer.packScreen(screen);
        m_socket->writeBuffer << 0x0003 << 0x0002 << 0x0002;
        sendPacket(false);
        ContactsMessage *msg = new ContactsMessage;
        msg->setContacts(contacts);
        messageReceived(msg, screen);
        return;
    }

    if (memcmp(cap, capabilities[CAP_SRV_RELAY], sizeof(cap))){
        string s;
        for (unsigned i = 0; i < sizeof(cap); i++){
            char b[5];
            sprintf(b, "0x%02X ", cap[i] & 0xFF);
            s += b;
        }
        log(L_DEBUG, "Unknown capability in advansed message\n%s", s.c_str());
        return;
    }

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
        pluginAnswer(plugin_type, atol(screen), info);
        sendAutoReply(screen, id, plugins[plugin_index],
                      cookie1, cookie2, 0, 0, 0x0200, NULL, 1, info);
        return;
    }

    unsigned short msgType;
    unsigned short msgFlags;
    unsigned short msgState;
    adv.unpack(msgType);
    adv.unpack(msgState);
    adv.unpack(msgFlags);
    string msg;
    adv >> msg;

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
            ICQUserData *data = findContact(screen, NULL, false, contact);
            if (data == NULL)
                return;
            if ((getInvisible() && (data->VisibleId.value == 0)) ||
                    (!getInvisible() && data->InvisibleId.value))
                return;
            ar_request req;
            req.screen  = screen;
            req.type    = msgType;
            req.ack		= 0;
            req.id      = id;
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

            if (!msg.empty()){
                set_str(&data->AutoReply.ptr, msg.c_str());
                Event e(EventContactChanged, contact);
                e.process();
            }
            return;
        }
    }
    Buffer copy;
    if (*msg.c_str() || (msgType == ICQ_MSGxEXT)){
        if (adv.readPos() < adv.writePos())
            copy.pack(adv.data(adv.readPos()), adv.writePos() - adv.readPos());
        log(L_DEBUG, "Msg size=%u type=%u", msg.size(), msgType);
        if (msg.size() || (msgType == ICQ_MSGxEXT)){
            Message *m = parseMessage(msgType, screen, msg, adv, id, cookie1 | (cookie2 << 16));
            if (m){
                if ((m_send.id == id) && (m_send.screen == screen)){
                    replyQueue.push_back(m_send);
                    m_send.msg    = NULL;
                    m_send.screen = "";
                    m_sendTimer->stop();
                    send(true);
                }
                list<SendMsg>::iterator it;
                for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                    SendMsg &s = *it;
                    log(L_DEBUG, "%u %u (%s) - %u %u (%s)", s.id.id_h, s.id.id_l, s.screen.c_str(), id.id_h, id.id_l, screen);
                    if ((s.id == id) && (s.screen == screen))
                        break;
                }
                if (it == replyQueue.end()){
                    bool bAccept = true;
                    unsigned short ackFlags = 0;
                    if (m->type() != MessageICQFile){
                        if (m->type() == MessageStatus){
                            bAccept = false;
                        }else{
                            switch (getStatus()){
                            case STATUS_DND:
                                if (getAcceptInDND())
                                    break;
                                ackFlags = ICQ_TCPxACK_DND;
                                bAccept = false;
                                break;
                            case STATUS_OCCUPIED:
                                if (getAcceptInOccupied())
                                    break;
                                ackFlags = ICQ_TCPxACK_OCCUPIED;
                                bAccept = false;
                                break;
                            }
                            if (msgFlags & (ICQ_TCPxMSG_URGENT | ICQ_TCPxMSG_LIST))
                                bAccept = true;
                        }
                        if (!bAccept){
                            Contact *contact;
                            ICQUserData *data = findContact(screen, NULL, false, contact);
                            if (data == NULL)
                                return;

                            ar_request req;
                            req.screen  = screen;
                            req.type    = msgType;
                            req.ack		= ackFlags;
                            req.id      = id;
                            req.id1     = cookie1;
                            req.id2     = cookie2;
                            req.bDirect = false;
                            arRequests.push_back(req);

                            ARRequest ar;
                            ar.contact  = contact;
                            ar.param    = &arRequests.back();
                            ar.receiver = this;
                            ar.status   = getStatus();
                            Event e(EventARRequest, &ar);
                            e.process();
                            return;
                        }
                    }
                    if (msgFlags & ICQ_TCPxMSG_URGENT)
                        m->setFlags(m->getFlags() | MESSAGE_URGENT);
                    if (msgFlags & ICQ_TCPxMSG_LIST)
                        m->setFlags(m->getFlags() | MESSAGE_LIST);
                    needAck = messageReceived(m, screen);
                }else{
                    Message *msg = (*it).msg;
                    replyQueue.erase(it);
                    if (msg->type() == MessageFile){
                        Contact *contact;
                        ICQUserData *data = findContact(screen, NULL, false, contact);
                        if ((m->type() != MessageICQFile) || (data == NULL)){
                            log(L_WARN, "Bad answer type");
                            msg->setError(I18N_NOOP("Send fail"));
                            Event e(EventMessageSent, msg);
                            e.process();
                            delete msg;
                            return;
                        }
                        if (m_state == 1){
                            msg->setError(I18N_NOOP("Message declined"));
                            Event e(EventMessageSent, msg);
                            e.process();
                            delete msg;
                            return;
                        }
                        ICQFileTransfer *ft = new ICQFileTransfer(static_cast<FileMessage*>(msg), data, this);
                        Event e(EventMessageAcked, msg);
                        e.process();
                        m_processMsg.push_back(msg);
                        ft->connect(static_cast<ICQFileMessage*>(m)->getPort());
                    }else{
                        log(L_WARN, "Unknown message type for ACK");
                        delete msg;
                    }
                }
            }
        }
        Buffer copy;
    }
    if (!needAck) return;
    sendAutoReply(screen, id, p, cookie1, cookie2,
                  msgType, 0, 0, NULL, 0, copy);
}

void ICQClient::sendAutoReply(const char *screen, MessageId id,
                              const plugin p, unsigned short cookie1, unsigned short cookie2,
                              unsigned short msgType, char msgFlags, unsigned short msgState,
                              const char *response, unsigned short response_type, Buffer &copy)
{
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
    m_socket->writeBuffer << id.id_l << id.id_h << 0x0002;
    m_socket->writeBuffer.packScreen(screen);
    m_socket->writeBuffer << 0x0003 << 0x1B00 << 0x0800;
    m_socket->writeBuffer.pack((char*)p, sizeof(plugin));
    m_socket->writeBuffer << 0x03000000L << (char)0;
    m_socket->writeBuffer.pack(cookie1);
    m_socket->writeBuffer.pack(cookie2);
    m_socket->writeBuffer.pack(cookie1);
    m_socket->writeBuffer << 0x00000000L << 0x00000000L << 0x00000000L;
    m_socket->writeBuffer.pack(msgType);
    m_socket->writeBuffer << msgFlags << msgState << (char)0;
    if (response){
        Contact *contact = NULL;
        findContact(screen, NULL, false, contact);
        string r;
        r = getContacts()->fromUnicode(contact, QString::fromUtf8(response));
        unsigned short size = (unsigned short)(r.length() + 1);
        m_socket->writeBuffer.pack(size);
        m_socket->writeBuffer.pack(r.c_str(), size);
    }else{
        m_socket->writeBuffer << (char)0x01 << response_type;
    }
    if (response_type != 3){
        if (copy.size()){
            m_socket->writeBuffer.pack(copy.data(0), copy.writePos());
        }else{
            m_socket->writeBuffer << 0x00000000L << 0xFFFFFF00L;
        }
    }
    sendPacket(false);
}

void ICQClient::sendMTN(const char *screen, unsigned short type)
{
    if (!getTypingNotification())
        return;
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_MTN);
    m_socket->writeBuffer << 0x00000000L << 0x00000000L << (unsigned short)0x0001;
    m_socket->writeBuffer.packScreen(screen);
    m_socket->writeBuffer << type;
    sendPacket(true);
}

class AIMParser : public HTMLParser
{
public:
    AIMParser() {}
    QString parse(const QString &str);
protected:
    bool bPara;
    QString res;
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
};

QString AIMParser::parse(const QString &str)
{
    bPara = false;
    HTMLParser::parse(str);
    return res;
}

void AIMParser::text(const QString &text)
{
    if (text.isEmpty())
        return;
    bPara = true;
    res += text;
}

void AIMParser::tag_start(const QString &tag, const list<QString> &options)
{
    QString otag;
    QString add;
    if (tag == "br")
        otag = "BR";
    if (tag == "p"){
        if (!bPara)
            return;
        otag = "BR";
    }
    if ((tag == "font") || (tag == "b") || (tag == "u") || (tag == "i"))
        otag = tag.upper();
    if (tag == "span")
        otag = "FONT";
    if (otag.isEmpty())
        return;
    res += "<";
    res += otag;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString key = *it;
        ++it;
        QString value = *it;
        if (key.lower() == "style"){
            list<QString> styles = parseStyle(value);
            for (list<QString>::const_iterator its = styles.begin(); its != styles.end(); ++its){
                QString key = *its;
                ++its;
                QString value = *its;
                if (key == "font-family")
                {
                    res += " FACE=\"";
                    res += value;
                    res += "\"";
                }
                else if (key == "font-size")
                {
                    if (value == "smaller")
                    {
                        res += " SIZE=2";
                    }
                    else if (value == "larger")
                    {
                        res += " SIZE=4";
                    }
                }
                else if (key == "font-style")
                {
                    if (value.lower() == "italic")
                        add = "<i>";
                }
                else if (key == "font-weight")
                {
                    if (value.toInt() >= 600)
                        add = "<b>";
                }
                else if (key == "text-decoration")
                {
                    if (value.lower() == "underline")
                        add = "u";
                }
                else if (key == "color")
                {
                    res += " COLOR=\"";
                    res += value;
                    res += "\"";
                }
            }
            continue;
        }
        res += " ";
        res += key.upper();
        res += "=\"";
        res += value;
        res += "\"";
    }
    res += ">";
}

void AIMParser::tag_end(const QString &tag)
{
    QString otag;
    if ((tag == "font") || (tag == "b") || (tag == "u") || (tag == "i"))
        otag = tag.upper();
    if (tag == "span")
        otag = "FONT";
    if (otag.isEmpty())
        return;
    res += "</";
    res += otag;
    res += ">";
}

void ICQClient::processSendQueue()
{
    m_sendTimer->stop();
    if (m_send.screen.length()){
        log(L_WARN, "Send timeout");
        if (m_send.msg){
            if (m_send.msg->type() == MessageCheckInvisible){
                Contact *contact;
                ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                if (data && (data->bInvisible.bValue == 0)) {
                    data->bInvisible.bValue = true;
                    Event e(EventContactStatus, contact);
                    e.process();
                }
            }
            m_send.msg->setError(I18N_NOOP("Send timeout"));
            Event e(EventMessageSent, m_send.msg);
            e.process();
            delete m_send.msg;
        }
        m_send.msg = NULL;
        m_send.screen = "";
        send(true);
        return;
    }
    m_sendTimer->start(50000);
    for (;;){
        if ((getState() != Connected) || (sendFgQueue.empty() && sendBgQueue.empty())){
            m_sendTimer->stop();
            return;
        }
        m_send = sendQueue.front();
        sendQueue.pop_front();

        Contact *contact;
        ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
        if ((data == NULL) && (m_send.flags != PLUGIN_RANDOMxCHAT)){
            if (m_send.msg != NULL)
            {
                m_send.msg->setError(I18N_NOOP("No contact"));
                Event e(EventMessageSent, m_send.msg);
                e.process();
                delete m_send.msg;
                m_send.msg = NULL;
            }
            m_send.screen = "";
            continue;
        }
        unsigned type = 0;
        if (m_send.msg)
            type = m_send.msg->type(),
                   log(L_DEBUG, "Send: %s %u %X", m_send.screen.c_str(), type, m_send.flags);
        if (m_send.msg && (m_send.socket == NULL)){
            unsigned short type;
            Buffer b;
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            switch (m_send.msg->type()){
            case MessageContacts:
                if (data->Uin.value == 0){
                    CONTACTS_MAP c;
                    QString nc = packContacts(static_cast<ContactsMessage*>(m_send.msg), data, c);
                    if (c.empty()){
                        m_send.msg->setError(I18N_NOOP("No contacts for send"));
                        Event e(EventMessageSent, m_send.msg);
                        e.process();
                        delete m_send.msg;
                        m_send.msg = NULL;
                        m_send.screen = "";
                        continue;
                    }
                    static_cast<ContactsMessage*>(m_send.msg)->setContacts(nc);
                    Buffer msgBuf;
                    vector<alias_group> cc;
                    for (CONTACTS_MAP::iterator it = c.begin(); it != c.end(); ++it){
                        alias_group c;
                        c.alias = (*it).first.c_str();
                        c.grp   = (*it).second.grp;
                        cc.push_back(c);
                    }
                    sort(cc.begin(), cc.end());

                    unsigned grp   = (unsigned)(-1);
                    unsigned start = 0;
                    unsigned short size = 0;
                    unsigned i;
                    for (i = 0; i < cc.size(); i++){
                        if (cc[i].grp != grp){
                            if (grp != (unsigned)(-1)){
                                string s = "Not in list";
                                if (grp){
                                    Group *group = getContacts()->group(grp);
                                    if (group)
                                        s = group->getName().utf8();
                                }
                                msgBuf.pack(s);
                                msgBuf << size;
                                for (unsigned j = start; j < i; j++)
                                    msgBuf.pack(cc[j].alias);
                            }
                            size  = 0;
                            start = i;
                            grp   = cc[i].grp;
                        }
                        size++;
                    }
                    string s = "Not in list";
                    if (grp){
                        Group *group = getContacts()->group(grp);
                        if (group)
                            s = group->getName().utf8();
                    }
                    msgBuf.pack(s);
                    msgBuf << size;
                    for (unsigned j = start; j < i; j++)
                        msgBuf.pack(cc[j].alias);
                    m_send.id.id_l = rand();
                    m_send.id.id_h = rand();
                    sendType2(m_send.screen.c_str(), msgBuf, m_send.id, CAP_AIM_BUDDYLIST, false, 0);
                    return;
                }
            case MessageUrl:{
                    if (data->Uin.value == 0)
                        break;
                    packMessage(b, m_send.msg, data, type);
                    const char *err = m_send.msg->getError();
                    if (err && *err){
                        Event e(EventMessageSent, m_send.msg);
                        e.process();
                        delete m_send.msg;
                        m_send.msg = NULL;
                        m_send.screen = "";
                        continue;
                    }
                    sendThroughServer(screen(data).c_str(), 4, b, m_send.id, true, true);
                    if ((data->Status.value != ICQ_STATUS_OFFLINE) || (getAckMode() == 0))
                        ackMessage(m_send);
                    return;
                }
            case MessageFile:
                packMessage(b, m_send.msg, data, type);
                sendAdvMessage(screen(data).c_str(), b, PLUGIN_NULL, m_send.id, false, true);
                return;
            case MessageCheckInvisible:{
                    Buffer b;
                    sendThroughServer(m_send.screen.c_str(), 2, b, m_send.id, true, false);
                    return;
                }
            case MessageWarning:{
                    WarningMessage *msg = static_cast<WarningMessage*>(m_send.msg);
                    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_BLAMExUSER, true);
                    m_send.id.id_l = m_nMsgSequence;
                    unsigned short flag = 0;
                    if (msg->getAnonymous())
                        flag = 1;
                    m_socket->writeBuffer << flag;
                    m_socket->writeBuffer.packScreen(screen(data).c_str());
                    sendPacket();
                    return;
                }
            }
            string text;
            switch (m_send.flags & SEND_MASK){
            case SEND_RTF:
                m_send.part = getRichTextPart(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
                text = createRTF(m_send.part, m_send.msg->getForeground(), contact);
                break;
            case SEND_UTF:
                m_send.part = getPart(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
                text = m_send.part.utf8();
                break;
            case SEND_TYPE2:{
                    m_send.part = getPart(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
                    text = getContacts()->fromUnicode(contact, m_send.part);
                    messageSend ms;
                    ms.msg  = m_send.msg;
                    ms.text = &text;
                    Event e(EventSend, &ms);
                    e.process();
                    break;
                }
            case SEND_HTML:
            case SEND_HTML_PLAIN:{
                    QString t;
                    if ((m_send.flags & SEND_MASK) == SEND_HTML){
                        m_send.part = getRichTextPart(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
                    }else{
                        m_send.part = getPart(m_send.text, MAX_TYPE2_MESSAGE_SIZE);
                    }
                    char b[15];
                    sprintf(b, "%06X", (unsigned)(m_send.msg->getBackground() & 0xFFFFFF));
                    t += "<HTML><BODY BGCOLOR=\"#";
                    t += b;
                    t += "\">";
                    if ((m_send.flags & SEND_MASK) == SEND_HTML){
                        AIMParser p;
                        t += p.parse(m_send.part);
                    }else{
                        string s;
                        s = m_send.part.utf8();
                        messageSend ms;
                        ms.msg  = m_send.msg;
                        ms.text = &s;
                        Event e(EventSend, &ms);
                        e.process();
                        t += quoteString(QString::fromUtf8(s.c_str()));
                    }
                    t += "</BODY></HTML>";
                    bool bWide = false;
                    for (int i = 0; i < (int)(t.length()); i++){
                        if (t[i].unicode() > 0x7F){
                            bWide = true;
                            break;
                        }
                    }
                    sendType1(t, bWide, data);
                    return;
                }
            default:
                m_send.part = getPart(m_send.text, MAX_PLAIN_MESSAGE_SIZE);
                sendType1(m_send.part, false, data);
                return;
            }

            Buffer msgBuf;
            unsigned short size  = (unsigned short)(text.length() + 1);
            unsigned short flags = ICQ_TCPxMSG_NORMAL;
            if (m_send.msg->getFlags() & MESSAGE_URGENT)
                flags = ICQ_TCPxMSG_URGENT;
            if (m_send.msg->getFlags() & MESSAGE_LIST)
                flags = ICQ_TCPxMSG_LIST;
            msgBuf.pack((unsigned short)1);
            msgBuf.pack(msgStatus());
            msgBuf.pack(flags);
            msgBuf.pack(size);
            msgBuf.pack(text.c_str(), size);
            if (m_send.msg->getBackground() == m_send.msg->getForeground()){
                msgBuf << 0x00000000L << 0xFFFFFF00L;
            }else{
                msgBuf << (m_send.msg->getForeground() << 8) << (m_send.msg->getBackground() << 8);
            }
            if ((m_send.flags & SEND_MASK) != SEND_TYPE2){
                msgBuf << 0x26000000L;
                packCap(msgBuf, capabilities[((m_send.flags & SEND_MASK) == SEND_RTF) ? CAP_RTF : CAP_UTF]);
            }
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            sendAdvMessage(m_send.screen.c_str(), msgBuf, PLUGIN_NULL, m_send.id, true, false);
            return;
        }
        if (m_send.socket){
            Buffer msgBuf;
            if (m_send.flags == PLUGIN_AIM_FT_ACK){
                AIMFileMessage *msg = static_cast<AIMFileMessage*>(m_send.msg);
                m_send.id.id_l = msg->getID_L();
                m_send.id.id_h = msg->getID_H();
                Buffer b;
                b << (unsigned short)0;
                b << m_send.id.id_l << m_send.id.id_h;
                b.pack((char*)capabilities[CAP_AIM_SENDFILE], sizeof(capability));
                b.tlv(0x0A, (unsigned short)2);
                b.tlv(0x03, (unsigned long)htonl(get_ip(this->data.owner.RealIP)));
                b.tlv(0x05, static_cast<AIMFileTransfer*>(msg->m_transfer)->remotePort());
                sendThroughServer(m_send.screen.c_str(), 2, b, m_send.id, false, false);
                replyQueue.push_back(m_send);
                m_send.msg    = NULL;
                m_send.screen = "";
                continue;
            }
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            if (m_send.flags == PLUGIN_AIM_FT){
                TlvList tlvs;
                tlvs + new Tlv(0x0E, 2, "en");
                char b[15];
                sprintf(b, "%06X", (unsigned)(m_send.msg->getBackground() & 0xFFFFFF));
                QString text = QString("<HTML><BODY BGCOLOR=\"#%1\">%2</BODY></HTML>")
                               .arg(b)
                               .arg(removeImages(m_send.msg->getRichText(), 0));
                bool bWide = false;
                int i;
                for (i = 0; i < (int)(text.length()); i++){
                    if (text[i].unicode() > 0x7F){
                        bWide = true;
                        break;
                    }
                }
                string charset = bWide ? "unicode-2-0" : "us-ascii";
                tlvs + new Tlv(0x0D, charset.length(), charset.c_str());
                string st;
                if (bWide){
                    for (i = 0; i < (int)(text.length()); i++){
                        unsigned short s = text[i].unicode();
                        st += (char)((s >> 8) & 0xFF);
                        st += (char)(s & 0xFF);
                    }
                }else{
                    st = text.utf8();
                }
                tlvs + new Tlv(0x0C, st.length(), st.c_str());
                FileMessage *msg = static_cast<FileMessage*>(m_send.msg);
                FileMessage::Iterator it(*msg);
                msgBuf
                << (unsigned short)0x0001
                << (unsigned short)(it.count())
                << (unsigned long)(it.size());
                QString fname;
                if (it.count() == 1){
                    fname = *(it[0]);
                    fname = fname.replace(QRegExp("\\\\"), "/");
                    int n = fname.findRev("/");
                    if (n >= 0)
                        fname = fname.mid(n + 1);
                }else{
                    fname = QString::number(it.count());
                    fname += " files";
                }
                bWide = false;
                for (i = 0; i < (int)(fname.length()); i++){
                    if (fname[i].unicode() > 0x7F){
                        bWide = true;
                        break;
                    }
                }
                charset = bWide ? "utf8" : "us-ascii";
                tlvs + new Tlv(0x2712, charset.length(), charset.c_str());
                msgBuf << (const char*)(fname.utf8()) << (char)0;
                sendType2(m_send.screen.c_str(), msgBuf, m_send.id, CAP_AIM_SENDFILE, false, m_send.socket->localPort(), &tlvs);
                return;
            }
            msgBuf.pack(this->data.owner.Uin.value);
            unsigned long ip = get_ip(this->data.owner.IP);
            if (ip == get_ip(m_send.socket->m_data->IP))
                ip = get_ip(this->data.owner.RealIP);
            msgBuf.pack(ip);
            msgBuf.pack((unsigned long)(m_send.socket->localPort()));
            msgBuf.pack((char)MODE_DIRECT);
            msgBuf.pack((unsigned long)(m_send.socket->remotePort()));
            msgBuf.pack(this->data.owner.Port.value);
            msgBuf.pack((unsigned short)8);
            msgBuf.pack((unsigned long)m_nMsgSequence);
            sendType2(m_send.screen.c_str(), msgBuf, m_send.id, CAP_DIRECT, false, 0);
            return;
        }
        if (m_send.flags == PLUGIN_AR){
            log(L_DEBUG, "Request auto response %s", m_send.screen.c_str());

            unsigned long status = data->Status.value;
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
            sendAdvMessage(screen(data).c_str(), msg, PLUGIN_NULL, m_send.id, false, false);
            return;
        }else if (m_send.flags == PLUGIN_RANDOMxCHAT){
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            Buffer b;
            b << (char)1 << 0x00000000L << 0x00010000L;
            sendAdvMessage(m_send.screen.c_str(), b, PLUGIN_RANDOMxCHAT, m_send.id, false, false);
        }else{
            unsigned plugin_index = m_send.flags;
            log(L_DEBUG, "Plugin info request %s (%u)", m_send.screen.c_str(), plugin_index);

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
            sendAdvMessage(m_send.screen.c_str(), b, type ? PLUGIN_INFOxMANAGER : PLUGIN_STATUSxMANAGER, m_send.id, false, false);
            return;
        }
    }
}

void ICQClient::sendType1(const QString &text, bool bWide, ICQUserData *data)
{
    Buffer msgBuf;
    if (bWide){
        string msg_text;
        for (int i = 0; i < (int)text.length(); i++){
            unsigned short c = text[i].unicode();
            char c1 = (char)((c >> 8) & 0xFF);
            char c2 = (char)(c & 0xFF);
            msg_text += c1;
            msg_text += c2;
        }
        msgBuf << 0x00020000L;
        msgBuf.pack(msg_text.c_str(), msg_text.length());
    }else{
        string msg_text;
        msg_text = getContacts()->fromUnicode(getContact(data), text);
        messageSend ms;
        ms.msg  = m_send.msg;
        ms.text = &msg_text;
        Event e(EventSend, &ms);
        e.process();
        msgBuf << 0x0000L;
        msgBuf << msg_text.c_str();
    }
    Buffer b;
    b.tlv(0x0501, "\x01", 1);
    b.tlv(0x0101, msgBuf);
    sendThroughServer(m_send.screen.c_str(), 1, b, m_send.id, true, true);
    if ((data->Status.value != ICQ_STATUS_OFFLINE) || (getAckMode() == 0))
        ackMessage(m_send);
}

void ICQClient::accept(Message *msg, ICQUserData *data)
{
    MessageId id;
    if (msg->getFlags() & MESSAGE_DIRECT){
        Contact *contact = getContacts()->contact(msg->contact());
        ICQUserData *data = NULL;
        if (contact){
            ClientDataIterator it(contact->clientData, this);
            while ((data = ((ICQUserData*)(++it))) != NULL){
                if (msg->client() && (dataName(data) == msg->client()))
                    break;
                data = NULL;
            }
        }
        if (data == NULL){
            log(L_WARN, "Data for request not found");
            return;
        }
        if (data->Direct.ptr == NULL){
            log(L_WARN, "No direct connection");
            return;
        }
        ((DirectClient*)(data->Direct.ptr))->acceptMessage(msg);
    }else{
        id.id_l = static_cast<ICQFileMessage*>(msg)->getID_L();
        id.id_h = static_cast<ICQFileMessage*>(msg)->getID_H();
        Buffer b;
        unsigned short type = ICQ_MSGxEXT;
        packMessage(b, msg, data, type, 0);
        unsigned cookie  = static_cast<ICQFileMessage*>(msg)->getCookie();
        sendAdvMessage(screen(data).c_str(), b, PLUGIN_NULL, id, false, true, (unsigned short)(cookie & 0xFFFF), (unsigned short)((cookie >> 16) & 0xFFFF), 2);
    }
}

void ICQClient::accept(Message *msg, const char *dir, OverwriteMode overwrite)
{
    ICQUserData *data = NULL;
    bool bDelete = true;
    if (msg->client()){
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact){
            ClientDataIterator it(contact->clientData, this);
            while ((data = ((ICQUserData*)(++it))) != NULL){
                if (dataName(data) == msg->client())
                    break;
                data = NULL;
            }
        }
    }
    if (data){
        switch (msg->type()){
        case MessageICQFile:{
                ICQFileTransfer *ft = new ICQFileTransfer(static_cast<FileMessage*>(msg), data, this);
                ft->setDir(QFile::encodeName(dir));
                ft->setOverwrite(overwrite);
                Event e(EventMessageAcked, msg);
                e.process();
                m_processMsg.push_back(msg);
                bDelete = false;
                ft->listen();
                break;
            }
        case MessageFile:{
                AIMFileTransfer *ft = new AIMFileTransfer(static_cast<FileMessage*>(msg), data, this);
                ft->setDir(QFile::encodeName(dir));
                ft->setOverwrite(overwrite);
                Event e(EventMessageAcked, msg);
                e.process();
                m_processMsg.push_back(msg);
                bDelete = false;
                ft->accept();
                break;
            }
        default:
            log(L_DEBUG, "Bad message type %u for accept", msg->type());
        }
    }
    Event e(EventMessageDeleted, msg);
    e.process();
    if (bDelete)
        delete msg;
}

void ICQClient::decline(Message *msg, const char *reason)
{
    if (msg->getFlags() & MESSAGE_DIRECT){
        Contact *contact = getContacts()->contact(msg->contact());
        ICQUserData *data = NULL;
        if (contact){
            ClientDataIterator it(contact->clientData, this);
            while ((data = ((ICQUserData*)(++it))) != NULL){
                if (msg->client() && (dataName(data) == msg->client()))
                    break;
                data = NULL;
            }
        }
        if (data == NULL){
            log(L_WARN, "Data for request not found");
            return;
        }
        if (data->Direct.ptr == NULL){
            log(L_WARN, "No direct connection");
            return;
        }
        ((DirectClient*)(data->Direct.ptr))->declineMessage(msg, reason);
    }else{
        MessageId id;
        unsigned cookie = 0;
        switch (msg->type()){
        case MessageICQFile:
            id.id_l = static_cast<ICQFileMessage*>(msg)->getID_L();
            id.id_h = static_cast<ICQFileMessage*>(msg)->getID_H();
            cookie  = static_cast<ICQFileMessage*>(msg)->getCookie();
            break;
        case MessageFile:
            id.id_l = static_cast<AIMFileMessage*>(msg)->getID_L();
            id.id_h = static_cast<AIMFileMessage*>(msg)->getID_H();
            break;
        default:
            log(L_WARN, "Bad type %u for decline");
        }
        ICQUserData *data = NULL;
        Contact *contact = NULL;
        if (msg->client()){
            contact = getContacts()->contact(msg->contact());
            if (contact){
                ClientDataIterator it(contact->clientData, this);
                while ((data = ((ICQUserData*)(++it))) != NULL){
                    if (dataName(data) == msg->client())
                        break;
                    data = NULL;
                }
            }
        }
        if (data && (id.id_l || id.id_h)){
            if (msg->type() == MessageICQFile){
                Buffer buf, msgBuf;
                Buffer b;
                packExtendedMessage(msg, buf, msgBuf, data);
                b.pack((unsigned short)buf.size());
                b.pack(buf.data(0), buf.size());
                b.pack32(msgBuf);
                unsigned short type = ICQ_MSGxEXT;
                sendAutoReply(screen(data).c_str(), id, plugins[PLUGIN_NULL], (unsigned short)(cookie & 0xFFFF), (unsigned short)((cookie >> 16) & 0xFFFF), type, 1, 0, reason, 2, b);
            }else{
                snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_AUTOREPLY);
                m_socket->writeBuffer << id.id_l << id.id_h << 0x0002;
                m_socket->writeBuffer.packScreen(screen(data).c_str());
                m_socket->writeBuffer << 0x0003 << 0x0002 << 0x0001;
                sendPacket();
                if (reason && *reason){
                    Message *msg = new Message(MessageGeneric);
                    msg->setText(QString::fromUtf8(reason));
                    msg->setFlags(MESSAGE_NOHISTORY);
                    msg->setContact(contact->id());
                    if (!send(msg, data))
                        delete msg;
                }
            }
        }
    }
    Event e(EventMessageDeleted, msg);
    e.process();
    delete msg;
}

void ICQClient::requestReverseConnection(const char *screen, DirectSocket *socket)
{
    SendMsg s;
    s.flags  = PLUGIN_REVERSE;
    s.socket = socket;
    s.screen = screen;
    sendQueue.push_front(s);
    send(false);
}

void ICQClient::send(bool bTimer)
{
    if (!m_send.screen.empty()){
        m_sendTimer->start(m_nSendTimeout * 500);
        return;
    }
    if (sendQueue.size() == 0){
        if (m_sendTimer->isActive() && m_send.screen.empty()){
            log(L_DEBUG, "Stop send timer");
            m_sendTimer->stop();
        }
        return;
    }
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
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        // PLUGIN_FILE
        { 0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91,
          0xD3, 0x11, 0x8D, 0xD7, 0x00, 0x10,
          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 },
        // PLUGIN_CHAT
        { 0xBF, 0xF7, 0x20, 0xB2, 0x37, 0x8E,
          0xD4, 0x11, 0xBD, 0x28, 0x00, 0x04,
          0xAC, 0x96, 0xD9, 0x05, 0x00, 0x00 }
    };

plugin const *ICQClient::plugins = arrPlugins;

bool operator == (const MessageId &m1, const MessageId &m2)
{
    return ((m1.id_l == m2.id_l) && (m1.id_h == m2.id_h));
}

