/***************************************************************************
                          icqdirect.cpp  -  description
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

#include "core.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

const unsigned short TCP_START  = 0x07EE;
const unsigned short TCP_ACK    = 0x07DA;
const unsigned short TCP_CANCEL	= 0x07D0;

const unsigned short ICQ_TCPxACK_ONLINE            = 0x0000;
const unsigned short ICQ_TCPxACK_AWAY              = 0x0004;
const unsigned short ICQ_TCPxACK_OCCUPIED          = 0x0009;
const unsigned short ICQ_TCPxACK_DND               = 0x000A;
const unsigned short ICQ_TCPxACK_OCCUPIEDxCAR      = 0x000B;
const unsigned short ICQ_TCPxACK_NA                = 0x000E;
const unsigned short ICQ_TCPxACK_DNDxCAR           = 0x000F;

ICQListener::ICQListener(ICQClient *client)
{
    m_socket = getSocketFactory()->createServerSocket();
    m_socket->setNotify(this);
    m_client = client;
}

ICQListener::~ICQListener()
{
    if (m_socket)
        delete m_socket;
}

void ICQListener::accept(Socket *s)
{
    log(L_DEBUG, "Accept direct connection");
    new DirectClient(s, m_client);
}

unsigned short ICQListener::port()
{
    if (m_socket)
        return m_socket->port();
    return 0;
}

// ___________________________________________________________________________________________

DirectSocket::DirectSocket(Socket *s, ICQClient *client)
{
    m_socket = new ClientSocket(this);
    m_socket->setSocket(s);
    m_bIncoming = true;
    m_client = client;
    m_state = WaitInit;
    m_version = 0;
    m_data = 0;
    init();
}

DirectSocket::DirectSocket(ICQUserData *data, ICQClient *client)
{
    m_socket    = new ClientSocket(this);
    m_bIncoming = false;
    m_version   = data->Version;
    if ((m_version > 8) || (m_version == 0))
        m_version = 8;
    m_client = client;
    m_data  = data;
    m_state = NotConnected;
    init();
}

DirectSocket::~DirectSocket()
{
    if (m_socket)
        delete m_socket;
}

void DirectSocket::init()
{
    if (!m_socket->created())
        m_socket->error_state("Connect error");
    m_nSequence = 0xFFFF;
    m_socket->writeBuffer.init(0);
    m_socket->readBuffer.init(2);
    m_socket->readBuffer.packetStart();
    m_bHeader        = true;
}

bool DirectSocket::error_state(const char *error, unsigned)
{
    if ((m_state == ConnectIP1) || (m_state == ConnectIP2)){
        connect();
        return false;
    }
    log(L_WARN, "Direct socket error %s", error);
    return true;
}

void DirectSocket::connect()
{
    m_socket->writeBuffer.init(0);
    m_socket->readBuffer.init(2);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
    if (m_data->Port == 0){
        log(L_WARN, "Connect to unknown port");
        return;
    }
    if (m_state == NotConnected){
        m_state = ConnectIP1;
        if (get_ip(m_data->RealIP) && (get_ip(m_data->IP) == get_ip(m_client->data.owner.IP))){
            struct in_addr addr;
            addr.s_addr = get_ip(m_data->RealIP);
            m_socket->connect(inet_ntoa(addr), m_data->Port, NULL);
            return;
        }
    }
    if (m_state == ConnectIP1){
        m_state = ConnectIP2;
        if (get_ip(m_data->IP) && (get_ip(m_data->IP) != get_ip(m_data->RealIP))){
            struct in_addr addr;
            addr.s_addr = get_ip(m_data->IP);
            m_socket->connect(inet_ntoa(addr), m_data->Port, m_client->protocol()->description()->text);
            return;
        }
    }
    m_socket->error_state("Can't established direct connection");
}

void DirectSocket::packet_ready()
{
    if (m_bHeader){
        unsigned short size;
        m_socket->readBuffer.unpack(size);
        if (size){
            m_socket->readBuffer.add(size);
            m_bHeader = false;
            return;
        }
    }
	if (m_state != Logged){
		ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
		log_packet(m_socket->readBuffer, false, plugin->ICQDirectPacket);
	}
    switch (m_state){
    case Logged:{
            processPacket();
            break;
        }
    case WaitAck:{
            unsigned short s1, s2;
            m_socket->readBuffer.unpack(s1);
            m_socket->readBuffer.unpack(s2);
            if ((s1 != 1) || (s2 != 0)){
                m_socket->error_state("Bad ack");
                return;
            }
            if (m_bIncoming){
                m_state = Logged;
                connect_ready();
            }else{
                m_state = WaitInit;
            }
            break;
        }
    case WaitInit:{
            char cmd;
            m_socket->readBuffer.unpack(cmd);
            if ((unsigned char)cmd != 0xFF){
                m_socket->error_state("Bad direct init command");
                return;
            }
            m_socket->readBuffer.unpack(m_version);
            if (m_version < 6){
                m_socket->error_state("Use old protocol");
                return;
            }
            m_socket->readBuffer.incReadPos(3);
            unsigned long my_uin;
            m_socket->readBuffer.unpack(my_uin);
            if (my_uin != m_client->data.owner.Uin){
                m_socket->error_state("Bad owner UIN");
                return;
            }
            m_socket->readBuffer.incReadPos(6);
            unsigned long p_uin;
            m_socket->readBuffer.unpack(p_uin);
            if (m_bIncoming){
                Contact *contact;
                m_data = m_client->findContact(p_uin, NULL, false, contact);
                if ((m_data == NULL) || contact->getIgnore()){
                    m_socket->error_state("User not found");
                    return;
                }
                if ((m_client->getInvisible() && (m_data->VisibleId == 0)) ||
                        (!m_client->getInvisible() && m_data->InvisibleId)){
                    m_socket->error_state("User not found");
                    return;
                }
            }else{
                if (p_uin != m_data->Uin){
                    m_socket->error_state("Bad sender UIN");
                    return;
                }
            }
            m_socket->readBuffer.incReadPos(13);
            unsigned long sessionId;
            m_socket->readBuffer.unpack(sessionId);
            if (m_bIncoming){
                m_nSessionId = sessionId;
                sendInitAck();
                sendInit();
                m_state = WaitAck;
            }else{
                if (sessionId != m_nSessionId){
                    m_socket->error_state("Bad session ID");
                    return;
                }
                sendInitAck();
                m_state = Logged;
                connect_ready();
            }
            break;
        }
    default:
        m_socket->error_state("Bad session ID");
        return;
    }
    m_socket->readBuffer.init(2);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
}

void DirectSocket::sendInit()
{
    if (!m_bIncoming){
        if (m_data->DCcookie == 0){
            m_socket->error_state("No direct info");
            return;
        }
        m_nSessionId = m_data->DCcookie;
    }

    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer.pack((unsigned short)((m_version >= 7) ? 0x0030 : 0x002c));
    m_socket->writeBuffer.pack((char)0xFF);
    m_socket->writeBuffer.pack((unsigned short)m_version);
    m_socket->writeBuffer.pack((unsigned short)((m_version >= 7) ? 0x002b : 0x0027));
    m_socket->writeBuffer.pack(m_data->Uin);
    m_socket->writeBuffer.pack((unsigned short)0x0000);
    m_socket->writeBuffer.pack((unsigned long)m_data->Port);
    m_socket->writeBuffer.pack(m_client->data.owner.Uin);
    m_socket->writeBuffer.pack(get_ip(m_client->data.owner.IP));
    m_socket->writeBuffer.pack(get_ip(m_client->data.owner.RealIP));
    m_socket->writeBuffer.pack((char)0x01);
    m_socket->writeBuffer.pack(0x00000000L);
    m_socket->writeBuffer.pack(m_nSessionId);
    m_socket->writeBuffer.pack(0x00000050L);
    m_socket->writeBuffer.pack(0x00000003L);
    if (m_version >= 7)
        m_socket->writeBuffer.pack(0x00000000L);
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->ICQDirectPacket);
    m_socket->write();
}

void DirectSocket::sendInitAck()
{
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer.pack((unsigned short)0x0004);
    m_socket->writeBuffer.pack((unsigned short)0x0001);
    m_socket->writeBuffer.pack((unsigned short)0x0000);
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->ICQDirectPacket);
    m_socket->write();
}

void DirectSocket::connect_ready()
{
    sendInit();
    m_state = WaitAck;
    m_socket->readBuffer.init(2);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
}

// ___________________________________________________________________________________________

static unsigned char client_check_data[] =
    {
        "As part of this software beta version Mirabilis is "
        "granting a limited access to the ICQ network, "
        "servers, directories, listings, information and databases (\""
        "ICQ Services and Information\"). The "
        "ICQ Service and Information may databases (\""
        "ICQ Services and Information\"). The "
        "ICQ Service and Information may\0"
    };

DirectClient::DirectClient(Socket *s, ICQClient *client)
        : DirectSocket(s, client)
{
    m_state = WaitLogin;
}

DirectClient::DirectClient(ICQUserData *data, ICQClient *client)
        : DirectSocket(data, client)
{
    m_state = None;
}

DirectClient::~DirectClient()
{
    for (list<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
        if (!m_client->sendThruServer((*it).msg, m_data)){
            (*it).msg->setError(I18N_NOOP("Send message fail"));
            Event e(EventMessageSent, (*it).msg);
            delete (*it).msg;
        }
    }
    if (m_data && (m_data->Direct == this))
        m_data->Direct = NULL;
}

void DirectClient::processPacket()
{
    switch (m_state){
    case None:
        m_socket->error_state("Bad state process packet");
        return;
    case WaitInit2:
        if (m_bIncoming) sendInit2();
        m_state = Logged;
        processMsgQueue();
        return;
    default:
        break;
    }
    unsigned long hex, key, B1, M1;
    unsigned int i;
    unsigned char X1, X2, X3;

    unsigned int correction = 2;
    if (m_version >= 7)
        correction++;

    unsigned int size = m_socket->readBuffer.size() - correction;
    if (m_version >= 7) m_socket->readBuffer.incReadPos(1);

    unsigned long check;
    m_socket->readBuffer.unpack(check);

    // main XOR key
    key = 0x67657268 * size + check;

    unsigned char *p = (unsigned char*)m_socket->readBuffer.data(m_socket->readBuffer.readPos()-4);
    for(i=4; i<(size+3)/4; i+=4) {
        hex = key + client_check_data[i&0xFF];
        p[i] ^= hex&0xFF;
        p[i+1] ^= (hex>>8) & 0xFF;
        p[i+2] ^= (hex>>16) & 0xFF;
        p[i+3] ^= (hex>>24) & 0xFF;
    }

    B1 = (p[4] << 24) | (p[6] << 16) | (p[4] <<8) | (p[6]<<0);

    // special decryption
    B1 ^= check;

    // validate packet
    M1 = (B1 >> 24) & 0xFF;
    if(M1 < 10 || M1 >= size){
        m_socket->error_state("Decrypt packet failed");
        return;
    }

    X1 = p[M1] ^ 0xFF;
    if(((B1 >> 16) & 0xFF) != X1){
        m_socket->error_state("Decrypt packet failed");
        return;
    }

    X2 = ((B1 >> 8) & 0xFF);
    if(X2 < 220) {
        X3 = client_check_data[X2] ^ 0xFF;
        if((B1 & 0xFF) != X3){
            m_socket->error_state("Decrypt packet failed");
            return;
        }
    }
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->readBuffer, false, plugin->ICQDirectPacket);

    m_socket->readBuffer.setReadPos(2);
    if (m_version >= 7){
        char startByte;
        m_socket->readBuffer.unpack(startByte);
        if (startByte != 0x02){
            m_socket->error_state("Bad start byte");
            return;
        }
    }
    unsigned long checksum;
    m_socket->readBuffer.unpack(checksum);
    unsigned short command;
    m_socket->readBuffer.unpack(command);
    m_socket->readBuffer.incReadPos(2);
    unsigned short seq;
    m_socket->readBuffer.unpack(seq);
    m_socket->readBuffer.incReadPos(12);

    unsigned short type, ackFlags, msgFlags;
    unsigned short id1 = 0;
    m_socket->readBuffer.unpack(type);
    m_socket->readBuffer.unpack(ackFlags);
    m_socket->readBuffer.unpack(msgFlags);
    string msg_str;
    m_socket->readBuffer >> msg_str;
    Message *m;
    list<SendDirectMsg>::iterator it;
    switch (command){
    case TCP_START:
        switch (type){
        case ICQ_MSGxAR_AWAY:
        case ICQ_MSGxAR_OCCUPIED:
        case ICQ_MSGxAR_NA:
        case ICQ_MSGxAR_DND:
        case ICQ_MSGxAR_FFC:{
                unsigned req_status = STATUS_AWAY;
                switch (type){
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
                ar_request req;
                req.uin  = m_data->Uin;
                req.type = type;
                req.timestamp1 = seq;
                req.timestamp2 = 0;
                req.id1     = 0;
                req.id2     = 0;
                req.bDirect = true;
                m_client->arRequests.push_back(req);

                Contact *contact = NULL;
                m_client->findContact(m_data->Uin, NULL, false, contact);
                ARRequest ar;
                ar.contact  = contact;
                ar.param    = &m_client->arRequests.back();
                ar.receiver = m_client;
                ar.status   = req_status;
                Event e(EventARRequest, &ar);
                e.process();
                return;
            }
        }
        m = m_client->parseMessage(type, m_data->Uin, msg_str, m_socket->readBuffer, 0, 0, seq, 0);
        if (m == NULL){
            m_socket->error_state("Start without message");
            return;
        }
        m->setFlags(MESSAGE_RECEIVED | MESSAGE_DIRECT);
        sendAck(seq, type);
        m_client->messageReceived(m, m_data->Uin);
        break;
    case TCP_CANCEL:
    case TCP_ACK:
        for (it = m_queue.begin(); it != m_queue.end(); ++it){
            if ((*it).seq == seq){
                Message *msg = (*it).msg;
                if (command == TCP_CANCEL){
                    Event e(EventMessageCancel, msg);
                    e.process();
                }else{
                    if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
                        if ((msg->type() == MessageGeneric) && ((*it).type != CAP_RTF)){
                            Message m;
                            m.setContact(msg->contact());
                            m.setClient(msg->client());
                            m.setText(msg->getPlainText());
                            m.setFlags(MESSAGE_DIRECT);
                            if (msg->getBackground() != msg->getForeground()){
                                m.setForeground(msg->getForeground());
                                m.setBackground(msg->getBackground());
                            }
                            Event e(EventSent, &m);
                            e.process();
                        }else{
                            msg->setFlags(msg->getFlags() | MESSAGE_DIRECT);
                            Event e(EventSent, msg);
                            e.process();
                        }
                    }
                    Event e(EventMessageSent, msg);
                    e.process();
                    m_queue.erase(it);
                }
                delete msg;
                break;
            }
        }
        if (it == m_queue.end())
            log(L_WARN, "Message for ACK not found");
        break;
    default:
        m_socket->error_state("Unknown TCP command");
    }
}

void DirectClient::connect_ready()
{
    if (m_state == None){
        m_state = WaitLogin;
        DirectSocket::connect_ready();
        return;
    }
    if (m_bIncoming){
        Contact *contact;
        m_data = m_client->findContact(m_data->Uin, NULL, false, contact);
        if ((m_data == NULL) || contact->getIgnore()){
            m_socket->error_state("Connection from unknown user");
            return;
        }
        if (m_data->Direct){
            m_socket->error_state("Connect already established");
            return;
        }
        m_data->Direct = this;
        if (m_version >= 7){
            m_state = WaitInit2;
        }else{
            m_state = Logged;
            processMsgQueue();
        }
    }else{
        if (m_version >= 7){
            sendInit2();
            m_state = WaitInit2;
        }else{
            m_state = Logged;
            processMsgQueue();
        }
    }
}

void DirectClient::sendInit2()
{
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer.pack((unsigned short)0x0021);
    m_socket->writeBuffer.pack((char) 0x03);
    m_socket->writeBuffer.pack(0x0000000AL);
    m_socket->writeBuffer.pack(0x00000001L);
    m_socket->writeBuffer.pack(m_bIncoming ? 0x00000001L : 0x00000000L);
    m_socket->writeBuffer.pack(0x00000000L);
    m_socket->writeBuffer.pack(0x00000000L);
    if (m_bIncoming) {
        m_socket->writeBuffer.pack(0x00040001L);
        m_socket->writeBuffer.pack(0x00000000L);
        m_socket->writeBuffer.pack(0x00000000L);
    } else {
        m_socket->writeBuffer.pack(0x00000000L);
        m_socket->writeBuffer.pack(0x00000000L);
        m_socket->writeBuffer.pack(0x00040001L);
    }
    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->ICQDirectPacket);
    m_socket->write();
}

bool DirectClient::error_state(const char *err, unsigned code)
{
    if (!DirectSocket::error_state(err, code))
        return false;
    if (m_state == None)
        m_data->bNoDirect = true;
    return true;
}

void DirectClient::sendAck(unsigned short seq, unsigned short type)
{
    startPacket(TCP_ACK, seq);
    m_socket->writeBuffer.pack(type);
    m_socket->writeBuffer << 0x00000000L
    << (char)1
    << (unsigned short)0
    << 0x00000000L
    << 0xFFFFFFFFL;
    sendPacket();
}

void DirectClient::startPacket(unsigned short cmd, unsigned short seq)
{
    m_socket->writeBuffer.packetStart();
    m_socket->writeBuffer
    << (unsigned short)0;	// size
    if (m_version >= 7)
        m_socket->writeBuffer << (char)0x02;
    if (seq == 0)
        seq = --m_nSequence;
    m_socket->writeBuffer
    << (unsigned long)0;			// checkSum
    m_socket->writeBuffer.pack(cmd);
    m_socket->writeBuffer
    << (char) 0x0E
    << (char) 0;
    m_socket->writeBuffer.pack(seq);
    m_socket->writeBuffer
    << (unsigned long)0
    << (unsigned long)0
    << (unsigned long)0;
}

void DirectClient::sendPacket()
{
    unsigned size = m_socket->writeBuffer.size() - m_socket->writeBuffer.packetStartPos() - 2;
    unsigned char *p = (unsigned char*)(m_socket->writeBuffer.data(m_socket->writeBuffer.packetStartPos()));
    *((unsigned short*)p) = size;

    ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
    log_packet(m_socket->writeBuffer, true, plugin->ICQDirectPacket);

    unsigned long hex, key, B1, M1;
    unsigned long i, check;
    unsigned char X1, X2, X3;

    p += 2;
    if (m_version >= 7){
        size--;
        p++;
    }

    // calculate verification data
    M1 = (rand() % ((size < 255 ? size : 255)-10))+10;
    X1 = p[M1] ^ 0xFF;
    X2 = rand() % 220;
    X3 = client_check_data[X2] ^ 0xFF;

    B1 = (p[4] << 24) | (p[6]<<16) | (p[4]<<8) | (p[6]);

    // calculate checkcode
    check = (M1 << 24) | (X1 << 16) | (X2 << 8) | X3;
    check ^= B1;

    *((unsigned long*)p) = check;
    // main XOR key
    key = 0x67657268 * size + check;

    // XORing the actual data
    for(i=4; i<(size+3)/4; i+=4){
        hex = key + client_check_data[i & 0xFF];
        p[i] ^= hex & 0xFF;
        p[i+1] ^= (hex>>8) & 0xFF;
        p[i+2] ^= (hex>>16) & 0xFF;
        p[i+3] ^= (hex>>24) & 0xFF;
    }
    m_socket->write();
}

void DirectClient::acceptMessage(Message *m)
{
}

void DirectClient::declineMessage(Message *m, const char *reason)
{
}

bool DirectClient::sendMessage(Message *msg)
{
    SendDirectMsg sm;
    sm.msg	= msg;
    sm.seq	= 0;
    sm.type	= 0;
    m_queue.push_back(sm);
    processMsgQueue();
    return true;
}

void packCap(Buffer &b, const capability &c);

void DirectClient::processMsgQueue()
{
    if (m_state != Logged)
        return;
    for (list<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end();){
        SendDirectMsg &sm = *it;
        if (sm.seq){
            ++it;
            continue;
        }
        string message;
        Buffer &mb = m_socket->writeBuffer;
        switch (sm.msg->type()){
        case MessageGeneric:
            startPacket(TCP_START, 0);
            mb.pack((unsigned short)ICQ_MSGxMSG);
            mb.pack((unsigned short)(m_client->fullStatus(m_client->getStatus()) & 0xFF));
            mb.pack((unsigned short)1);
            if ((sm.msg->getFlags() & MESSAGE_RICHTEXT) && (m_client->hasCap(m_data, CAP_RTF))){
                message = m_client->createRTF(sm.msg->getRichText().utf8(), sm.msg->getForeground(), m_data->Encoding);
                sm.type = CAP_RTF;
            }else if (m_client->hasCap(m_data, CAP_UTF)){
                message = sm.msg->getPlainText().utf8();
                sm.type = CAP_UTF;
            }else{
                message = m_client->fromUnicode(sm.msg->getPlainText(), m_data);
            }
            mb << message;
            if (sm.msg->getBackground() == sm.msg->getForeground()){
                mb << 0x00000000L << 0xFFFFFF00L;
            }else{
                mb << (sm.msg->getForeground() << 8) << (sm.msg->getBackground() << 8);
            }
            if (sm.type){
                mb << 0x26000000L;
                packCap(mb, ICQClient::capabilities[sm.type]);
            }
            sendPacket();
            sm.seq = m_nSequence;
            sm.icq_type = ICQ_MSGxMSG;
            break;
        case MessageURL:
        case MessageContact:
            startPacket(TCP_START, 0);
            message = m_client->packMessage(sm.msg, m_data, sm.icq_type);
            mb.pack((unsigned short)sm.icq_type);
            mb.pack((unsigned short)(m_client->fullStatus(m_client->getStatus()) & 0xFF));
            mb.pack((unsigned short)1);
            mb << message;
            sendPacket();
            sm.seq = m_nSequence;
            break;
        default:
            sm.msg->setError(I18N_NOOP("Unknown message type"));
            Event e(EventMessageSent, sm.msg);
            e.process();
            delete sm.msg;
            m_queue.erase(it);
            it = m_queue.begin();
            continue;
        }
        ++it;
    }
}

bool DirectClient::cancelMessage(Message *msg)
{
    for (list<SendDirectMsg>::iterator it = m_queue.begin(); it != m_queue.end(); ++it){
        if ((*it).msg == msg){
            if ((*it).seq){
                Buffer &mb = m_socket->writeBuffer;
                startPacket(TCP_CANCEL, (*it).seq);
                mb.pack((unsigned short)(*it).icq_type);
                mb.pack((unsigned short)0);
                mb.pack((unsigned short)0);
                string message;
                mb << message;
                sendPacket();
            }
            m_queue.erase(it);
            return true;
        }
    }
    return false;
}

void DirectClient::sendAutoResponse(unsigned short seq, unsigned short type, const char *answer)
{
    startPacket(TCP_ACK, seq);
    m_socket->writeBuffer.pack(type);
    unsigned short status = 0;
    switch (m_client->getStatus()){
    case STATUS_AWAY:
        status = ICQ_TCPxACK_AWAY;
        break;
    case STATUS_OCCUPIED:
        status = ICQ_TCPxACK_OCCUPIEDxCAR;
        break;
    case STATUS_DND:
        status = ICQ_TCPxACK_DNDxCAR;
        break;
    default:
        status = ICQ_TCPxACK_NA;
    }
    string response = answer;
    m_socket->writeBuffer.pack(status);
    m_socket->writeBuffer.pack((unsigned short)0);
    m_socket->writeBuffer << response;
    m_socket->writeBuffer << 0x00000000L << 0x00000000L;
    sendPacket();
}
