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

const unsigned short ICQ_SNACxMSG_ERROR            = 0x0001;
const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002;
const unsigned short ICQ_SNACxMSG_RESETxICQxMODE   = 0x0003;    // not implemented
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004;
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005;
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006;
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007;
const unsigned short ICQ_SNACxMSG_BLAMExUSER       = 0x0008;    // not implemented
const unsigned short ICQ_SNACxMSG_BLAMExSRVxACK    = 0x0009;    // not implemented
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
            string screen = m_socket->readBuffer.unpackScreen();
            unsigned short type;
            m_socket->readBuffer >> type;
            bool bType = (type > 1);
            Contact *contact;
            ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
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
                err_str = I18N_NOOP("Not supported by client");
                Contact *contact;
                ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                if (data){
                    for (list<SendMsg>::iterator it = sendQueue.begin(); it != sendQueue.end();){
                        if ((*it).screen != m_send.screen){
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
                if ((m_send.msg->type() == MessageCheckInvisible) && (error == 0x0004)) {
                    Contact *contact;
                    ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                    if (data && (bool)(data->bInvisible)) {
                        data->bInvisible = false;
                        Event e(EventContactStatus, contact);
                        e.process();
                    }
                } else {
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
            for(unsigned short i = 0; i < nTlv; i++) {
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
            // need to do more ...
        }
    case ICQ_SNACxMSG_ACK:
        {
            log(L_DEBUG, "Ack message");
            MessageId id;
            m_socket->readBuffer >> id.id_l >> id.id_h;
            m_socket->readBuffer.incReadPos(2);
            string screen = m_socket->readBuffer.unpackScreen();
            if ((m_send.screen != screen) || !(m_send.id == id)){
                log(L_WARN, "Bad ack sequence");
		if (m_send.msg){
			m_send.msg->setError(I18N_NOOP("Bad ack sequence"));
			Event e(EventMessageSent, m_send.msg);
			e.process();
			delete m_send.msg;
		}
            }else{
                if (m_send.msg){
                    if (m_send.msg->type() == MessageCheckInvisible){
                        Contact *contact;
                        ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
                        if (data && !(bool)(data->bInvisible)) {
                            data->bInvisible = true;
                            Event e(EventContactStatus, contact);
                            e.process();
                        }
                        delete m_send.msg;
                    }else{
                        Contact *contact;
                        ICQUserData *data = findContact(screen.c_str(), NULL, false, contact);
                        if ((data == NULL) || (data->Status == ICQ_STATUS_OFFLINE)){
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
                if (data && set_str(&data->AutoReply, answer.c_str())){
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
                        messageReceived(msg, screen.c_str());
                        break;
                    }
                    ICQMessage *msg = new ICQMessage;
                    msg->setServerText(m_data);
                    messageReceived(msg, screen.c_str());
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
                        parseAdvancedMessage(screen.c_str(), msg, tlv(3) != NULL, id);
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
                        parseAdvancedMessage(screen.c_str(), msg, tlv(6) != NULL, id);
                        return;
                    }
                    char type, flags;
                    msg >> type;
                    msg >> flags;
                    string msg_str;
                    msg >> msg_str;
                    Message *m = parseMessage(type, screen.c_str(), msg_str, msg);
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
        if ((data->Status != ICQ_STATUS_OFFLINE) && (getSendFormat() == 0) &&
                hasCap(data, CAP_RTF) && (msg->getFlags() & MESSAGE_RICHTEXT) &&
                !data->bBadClient){
            s.flags  = SEND_RTF;
            s.msg    = msg;
            s.text   = msg->getRichText();
            s.screen = screen(data);
            sendQueue.push_front(s);
            send(false);
            return true;
        }
        if ((data->Status != ICQ_STATUS_OFFLINE) &&
                (getSendFormat() <= 1) &&
                hasCap(data, CAP_UTF) &&
                (data->Version >= 8) && !data->bBadClient){
            s.flags  = SEND_UTF;
            s.msg    = msg;
            s.text   = msg->getPlainText();
            s.screen = screen(data);
            sendQueue.push_front(s);
            send(false);
            return true;
        }
        s.flags	 = SEND_PLAIN;
        s.msg	 = msg;
        s.text	 = msg->getPlainText();
        s.screen = screen(data);
        sendQueue.push_front(s);
        send(false);
        return true;
    case MessageURL:
    case MessageContact:
    case MessageFile:
    case MessageCheckInvisible:
        s.flags  = SEND_RAW;
        s.msg    = msg;
        s.screen = screen(data);
        sendQueue.push_front(s);
        send(false);
        return true;
    }
    return false;
}

void ICQClient::sendThroughServer(const char *screen, unsigned short channel, Buffer &b, unsigned long id_l, unsigned long id_h, bool bOffline)
{
    // we need informations about channel 2 tlvs !
    int tlv_type = 5;
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER);
    m_socket->writeBuffer << id_l << id_h;
    m_socket->writeBuffer << channel;
    m_socket->writeBuffer.packScreen(screen);
    if (channel == 1)
        tlv_type = 2;
    m_socket->writeBuffer.tlv(tlv_type,b);
    m_socket->writeBuffer.tlv(3);		// req. ack from server
    if (bOffline)
        m_socket->writeBuffer.tlv(6);	// store if user is offline
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

void ICQClient::ackMessage(SendMsg &s)
{
    if ((s.msg->getFlags() & MESSAGE_NOHISTORY) == 0){
        if ((s.flags & SEND_MASK) == SEND_RAW){
            s.msg->setClient(dataName(m_send.screen.c_str()).c_str());
            Event e(EventSent, s.msg);
            e.process();
        }else if (!s.part.isEmpty()){
            Message m(MessageGeneric);
            m.setContact(s.msg->contact());
            m.setText(s.part);
            m.setBackground(s.msg->getBackground());
            m.setForeground(s.msg->getForeground());
            unsigned flags = s.msg->getFlags() & (~MESSAGE_RICHTEXT);
            if ((s.flags & SEND_MASK) == SEND_RTF)
                flags |= MESSAGE_RICHTEXT;
            m.setFlags(flags);
            m.setClient(dataName(s.screen.c_str()).c_str());
            Event e(EventSent, &m);
            e.process();
        }
    }
    string text;
    if (s.text.length() == 0){
        Event e(EventMessageSent, s.msg);
        e.process();
        delete s.msg;
    }else{
        sendQueue.push_front(s);
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
            msg->setRetry(MESSAGE_LIST | MESSAGE_URGENT);
            break;
        case ICQ_TCPxACK_DND:
            msg->setRetry(MESSAGE_LIST);
            break;
        }
        return false;
    }
    return true;
}

void ICQClient::sendAdvMessage(const char *screen, Buffer &msgText, unsigned plugin_index, const MessageId &id, bool bOffline, bool bPeek, bool bDirect)
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
    sendType2(screen, msgBuf, id, CAP_SRV_RELAY, bOffline, bPeek, bDirect);
}

void ICQClient::sendType2(const char *screen, Buffer &msgBuf, const MessageId &id, unsigned cap, bool bOffline, bool bPeek, bool bDirect)
{
    Buffer b;
    b << (unsigned short)0;
    b << id.id_l << id.id_h;
    b.pack((char*)capabilities[cap], sizeof(capability));
    b.tlv(0x0A, (unsigned short)0x01);
    if (bDirect){
        b.tlv(0x03, (unsigned long)htonl(get_ip(data.owner.RealIP)));
        b.tlv(0x05, (unsigned short)data.owner.Port);
    }
    b.tlv(0x0F);
    b.tlv(0x2711, msgBuf);
    if (bPeek)
        b.tlv(0x03);
    sendThroughServer(screen, 2, b, id.id_l, id.id_h, bOffline);
}

void ICQClient::clearMsgQueue()
{
    for (list<SendMsg>::iterator it = sendQueue.begin(); it != sendQueue.end(); ++it){
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
    sendQueue.clear();
    if (m_send.msg){
        m_send.msg->setError(I18N_NOOP("Client go offline"));
        Event e(EventMessageSent, m_send.msg);
        e.process();
        delete m_send.msg;
    }
    m_send.msg    = NULL;
    m_send.screen = "";
}

void ICQClient::parseAdvancedMessage(const char *screen, Buffer &msg, bool needAck, MessageId id)
{
    msg.incReadPos(8);
    capability cap;
    msg.unpack((char*)cap, sizeof(cap));
    if (!memcmp(cap, capabilities[CAP_DIRECT], sizeof(cap))){
        TlvList tlv(msg);
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
        log(L_DEBUG, "Setup reverse connect to %s %s:%u", screen, inet_ntoa(addr), localPort);
        DirectClient *direct = new DirectClient(data, this);
        m_sockets.push_back(direct);
        direct->reverseConnect(localIP, localPort);
        return;
    }

    if (memcmp(cap, capabilities[CAP_SRV_RELAY], sizeof(cap))){
        log(L_DEBUG, "Unknown capability in adavansed message");
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
            ICQUserData *data = findContact(screen, NULL, false, contact);
            if (data == NULL)
                return;
            if ((getInvisible() && (data->VisibleId == 0)) ||
                    (!getInvisible() && data->InvisibleId))
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
        if (real_ip || ip){
            Contact *contact;
            ICQUserData *data = findContact(screen, NULL, false, contact);
            if (data){
                if (real_ip && (get_ip(data->RealIP) == 0))
                    set_ip(&data->RealIP, real_ip);
                if (ip && (get_ip(data->IP) == 0))
                    set_ip(&data->IP, ip);
            }
        }
        adv >> msg;
        if (*msg.c_str() || (msgType == ICQ_MSGxEXT)){
            if (adv.readPos() < adv.writePos())
                copy.pack(adv.data(adv.readPos()), adv.writePos() - adv.readPos());
            log(L_DEBUG, "Msg size=%u type=%u", msg.size(), msgType);
            if (msg.size() || (msgType == ICQ_MSGxEXT)){
                Message *m = parseMessage(msgType, screen, msg, adv);
                if (m){
                    list<SendMsg>::iterator it;
                    for (it = replyQueue.begin(); it != replyQueue.end(); ++it){
                        SendMsg &s = *it;
                        if ((s.id == id) && (s.screen == screen))
                            break;
                    }
                    if (it == replyQueue.end()){
                        bool bAccept = true;
                        unsigned ackFlags = 0;
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
        }
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
    if (response && *response){
        Contact *contact;
        ICQUserData *data = findContact(screen, NULL, false, contact);
        string r = fromUnicode(QString::fromUtf8(response), data);
        m_socket->writeBuffer.pack((unsigned short)(r.size() + 1));
        m_socket->writeBuffer << r.c_str() << (char)0
        << 0x00000000L << 0xFFFFFF00L;
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

void ICQClient::sendMTN(const char *screen, unsigned short type)
{
    if (!getTypingNotification())
        return;
    snac(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_MTN);
    m_socket->writeBuffer << 0x00000000L << 0x00000000L << (unsigned short)0x0001;
    m_socket->writeBuffer.packScreen(screen);
    m_socket->writeBuffer << type;
    sendPacket();
}

void ICQClient::processSendQueue()
{
    m_sendTimer->stop();
    if (m_send.screen.length()){
        log(L_WARN, "Send timeout");
        if (m_send.msg){
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
    m_sendTimer->start(30000);
    for (;;){
        if ((getState() != Connected) || sendQueue.empty()){
            m_sendTimer->stop();
            return;
        }
        m_send = sendQueue.front();
        sendQueue.pop_front();

        Contact *contact;
        ICQUserData *data = findContact(m_send.screen.c_str(), NULL, false, contact);
        if ((data == NULL) && (m_send.flags != PLUGIN_RANDOMxCHAT)){
            m_send.msg->setError(I18N_NOOP("No contact"));
            Event e(EventMessageSent, m_send.msg);
            e.process();
            delete m_send.msg;
            m_send.msg = NULL;
            m_send.screen = "";
            continue;
        }

        if (m_send.msg){
            unsigned short type;
            Buffer b;
	    m_send.id.id_l = rand();
	    m_send.id.id_h = rand();
            switch (m_send.msg->type()){
            case MessageURL:
            case MessageContact:
                packMessage(b, m_send.msg, data, type, 0);
                sendThroughServer(screen(data).c_str(), 4, b, m_send.id.id_l, m_send.id.id_h, true);
                if (data->Status != ICQ_STATUS_OFFLINE)
                    ackMessage(m_send);
                return;
            case MessageFile:
                packMessage(b, m_send.msg, data, type, 0);
                sendAdvMessage(screen(data).c_str(), b, PLUGIN_NULL, m_send.id, false, false, true);
                return;
            case MessageCheckInvisible:
                b.pack(ICQ_MSGxAR_AWAY);
                b.pack((unsigned short)(fullStatus(m_status) & 0xFFFF));
                b << 0x0100 << 0x0100 << (char)0;
                sendAdvMessage(screen(data).c_str(), b, PLUGIN_NULL, m_send.id, false, true, false);
                return;
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
                Buffer msgBuf;
                if ((m_send.flags & SEND_MASK) == SEND_2GO){
                    string msg_text;
                    for (int i = 0; i < (int)m_send.part.length(); i++){
                        unsigned short c = m_send.part[i].unicode();
                        char c1 = (c >> 8) & 0xFF;
                        char c2 = c & 0xFF;
                        msg_text += c1;
                        msg_text += c2;
                    }
                    msgBuf << 0x00020000L;
                    msgBuf.pack(msg_text.c_str(), msg_text.length());
                }else{
                    QTextCodec *codec = getCodec(encoding.c_str());
                    string msg_text;
                    msg_text = codec->fromUnicode(m_send.part);
                    msgBuf << 0x0000L;
                    msgBuf << msg_text.c_str();
                }
                Buffer b;
                b.tlv(0x0501, "\x01", 1);
                b.tlv(0x0101, msgBuf);
                sendThroughServer(m_send.screen.c_str(), 1, b, m_send.id.id_l, m_send.id.id_h, true);
                if (data->Status != ICQ_STATUS_OFFLINE)
                    ackMessage(m_send);
                return;
            }

            Buffer msgBuf;
            unsigned short size = text.length() + 1;
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
            msgBuf << 0x26000000L;
            packCap(msgBuf, capabilities[((m_send.flags & SEND_MASK) == SEND_RTF) ? CAP_RTF : CAP_UTF]);
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            sendAdvMessage(m_send.screen.c_str(), msgBuf, PLUGIN_NULL, m_send.id, true, false, false);
            return;
        }
        if (m_send.socket){
            Buffer msgBuf;
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            msgBuf.pack(this->data.owner.Uin);
            msgBuf.pack(get_ip(this->data.owner.RealIP));
            msgBuf.pack((unsigned long)(m_send.socket->localPort()));
            msgBuf.pack((char)MODE_DIRECT);
            msgBuf.pack((unsigned long)(m_send.socket->remotePort()));
            msgBuf.pack((unsigned long)(m_send.socket->localPort()));
            msgBuf.pack((unsigned short)8);
            msgBuf.pack((unsigned long)m_nMsgSequence);
            sendType2(m_send.screen.c_str(), msgBuf, m_send.id, CAP_DIRECT, false, false, false);
            return;
        }
        if (m_send.flags == PLUGIN_AR){
            log(L_DEBUG, "Request auto response %s", m_send.screen.c_str());

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
            sendAdvMessage(screen(data).c_str(), msg, PLUGIN_NULL, m_send.id, false, false, false);
            return;
        }else if (m_send.flags == PLUGIN_RANDOMxCHAT){
            m_send.id.id_l = rand();
            m_send.id.id_h = rand();
            Buffer b;
            b << (char)1 << 0x00000000L << 0x00010000L;
            sendAdvMessage(m_send.screen.c_str(), b, PLUGIN_RANDOMxCHAT, m_send.id, false, false, false);
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
            sendAdvMessage(m_send.screen.c_str(), b, type ? PLUGIN_INFOxMANAGER : PLUGIN_STATUSxMANAGER, m_send.id, false, false, false);
            return;
        }
    }
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
    if (sendQueue.size() == 0){
        if (m_sendTimer->isActive() && m_send.screen.empty())
            m_sendTimer->stop();
        return;
    }
    if (!m_send.screen.empty())
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
