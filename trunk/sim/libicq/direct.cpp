/***************************************************************************
                          direct.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "icqclient.h"
#include "icqssl.h"
#include "log.h"

#include <time.h>
#include <errno.h>

#ifndef WIN32
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#endif

const char FT_INIT	= 0;
const char FT_INIT_ACK = 1;
const char FT_FILEINFO = 2;
const char FT_START	= 3;
const char FT_SPEED = 5;
const char FT_DATA	= 6;

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

DirectSocket::DirectSocket(Socket *s, ICQClient *_client)
{
    sock = new ClientSocket(this, _client);
    sock->setSocket(s);
    m_bIncoming = true;
    client = _client;
    state = WaitInit;
    ip = 0;
    real_ip = 0;
    port = 0;
    uin = 0;
    DCcookie = 0;
    init();
}

DirectSocket::DirectSocket(unsigned long _ip, unsigned long _real_ip, unsigned short _port,
                           ICQUser *u, ICQClient *_client)
{
    sock = new ClientSocket(this, _client);
    m_bIncoming = false;
    ip = _ip;
    real_ip = _real_ip;
    port = _port;
    uin = u->Uin;
    version = u->Version;
    DCcookie = u->DCcookie;
    if (version > 8) version = 8;
    client = _client;
    state = NotConnected;
    init();
}

DirectSocket::~DirectSocket()
{
    if (sock) delete sock;
}

void DirectSocket::init()
{
    if (!sock->created()) sock->error_state(ErrorConnect);
    m_nSequence = 0xFFFF;
    sock->writeBuffer.init(0);
    sock->readBuffer.init(2);
    m_bHeader = true;
    m_bUseInternalIP = true;
}

bool DirectSocket::error_state(SocketError)
{
    if ((state == ConnectIP1) || (state == ConnectIP2)){
        connect();
        return false;
    }
    return true;
}

void DirectSocket::connect()
{
    sock->writeBuffer.init(0);
    sock->readBuffer.init(2);
    m_bHeader = true;
    if (port == 0){
        log(L_WARN, "Connect to unknown port");
        return;
    }
    if (state == NotConnected){
        m_bUseInternalIP = true;
        if ((ip != 0) && ((ip & 0xFFFFFF) != (client->owner->IP & 0xFFFFFF)))
            m_bUseInternalIP = false;
        log(L_DEBUG, "Use internal... %u", m_bUseInternalIP);
        state = ConnectIP1;
        if (real_ip != 0){
            struct in_addr addr;
            addr.s_addr = m_bUseInternalIP ? real_ip : ip;
            sock->setProxy(m_bUseInternalIP ? NULL : client->getProxy());
            sock->connect(inet_ntoa(addr), port);
            return;
        }
    }
    if (state == ConnectIP1){
        state = ConnectIP2;
        if (ip != 0){
            struct in_addr addr;
            addr.s_addr = m_bUseInternalIP ? ip : real_ip;
            sock->setProxy(m_bUseInternalIP ? NULL : client->getProxy());
            sock->connect(inet_ntoa(addr), port);
            return;
        }
    }
    log(L_WARN, "Can't established direct connection");
    sock->error_state(ErrorConnect);
}

void DirectSocket::packet_ready()
{
    if (m_bHeader){
        unsigned short size;
        sock->readBuffer.unpack(size);
        if (size){
            sock->readBuffer.add(size);
            m_bHeader = false;
            return;
        }
    }
    dumpPacket(sock->readBuffer, 0,"Direct read");
    switch (state){
    case Logged:{
            processPacket();
            break;
        }
    case WaitAck:{
            unsigned short s1, s2;
            sock->readBuffer.unpack(s1);
            sock->readBuffer.unpack(s2);
            if ((s1 != 1) || (s2 != 0)){
                log(L_WARN, "Bad ack %X %X", s1, s2);
                sock->error_state(ErrorProtocol);
                return;
            }
            if (m_bIncoming){
                state = Logged;
                connect_ready();
            }else{
                state = WaitInit;
            }
            break;
        }
    case WaitInit:{
            char cmd;
            sock->readBuffer.unpack(cmd);
            if ((unsigned char)cmd != 0xFF){
                log(L_WARN, "Bad direct init command (%X)", cmd & 0xFF);
                sock->error_state(ErrorProtocol);
                return;
            }
            sock->readBuffer.unpack(version);
            if (version < 6){
                log(L_WARN, "Use old protocol");
                sock->error_state(ErrorProtocol);
                return;
            }
            sock->readBuffer.incReadPos(3);
            unsigned long my_uin;
            sock->readBuffer.unpack(my_uin);
            if (my_uin != client->owner->Uin){
                log(L_WARN, "Bad UIN");
                sock->error_state(ErrorProtocol);
                return;
            }
            sock->readBuffer.incReadPos(6);
            unsigned long p_uin;
            sock->readBuffer.unpack(p_uin);
            if (m_bIncoming){
                ICQUser *user = client->getUser(p_uin, true, true);
                if ((user == NULL) || user->inIgnore){
                    log(L_WARN, "User %lu not found", p_uin);
                    sock->error_state(ErrorProtocol);
                    return;
                }
                uin = p_uin;
            }else{
                if (p_uin != uin){
                    log(L_WARN, "Bad sender UIN");
                    sock->error_state(ErrorProtocol);
                    return;
                }
            }
            sock->readBuffer.incReadPos(13);
            unsigned long sessionId;
            sock->readBuffer.unpack(sessionId);
            if (m_bIncoming){
                m_nSessionId = sessionId;
                sendInitAck();
                sendInit();
                state = WaitAck;
            }else{
                if (sessionId != m_nSessionId){
                    log(L_WARN, "Bad session ID");
                    sock->error_state(ErrorProtocol);
                    return;
                }
                sendInitAck();
                state = Logged;
                connect_ready();
            }
            break;
        }
    default:
        log(L_WARN, "Bad state");
        sock->error_state(ErrorProtocol);
        return;
    }
    sock->readBuffer.init(2);
    m_bHeader = true;
}

void DirectSocket::sendInit()
{
    if (!m_bIncoming){
        if (DCcookie == 0){
            log(L_WARN, "No direct info");
            sock->error_state(ErrorProtocol);
            return;
        }
        m_nSessionId = DCcookie;
    }

    unsigned pos = sock->writeBuffer.writePos();
    sock->writeBuffer.pack((unsigned short)((version >= 7) ? 0x0030 : 0x002c));
    sock->writeBuffer.pack((char)0xFF);
    sock->writeBuffer.pack((unsigned short)version);
    sock->writeBuffer.pack((unsigned short)((version >= 7) ? 0x002b : 0x0027));
    sock->writeBuffer.pack(uin);
    sock->writeBuffer.pack((unsigned short)0x0000);
    sock->writeBuffer.pack((unsigned long)port);
    sock->writeBuffer.pack(client->owner->Uin);
    sock->writeBuffer.pack(client->owner->IP);
    sock->writeBuffer.pack(client->owner->RealIP);
    sock->writeBuffer.pack((char)0x01);
    sock->writeBuffer.pack(0x00000000L);
    sock->writeBuffer.pack(m_nSessionId);
    sock->writeBuffer.pack(0x00000050L);
    sock->writeBuffer.pack(0x00000003L);
    if (version >= 7)
        sock->writeBuffer.pack(0x00000000L);
    dumpPacket(sock->writeBuffer, pos, "Direct write");
    sock->write();
}

void DirectSocket::sendInitAck()
{
    unsigned pos = sock->writeBuffer.writePos();
    sock->writeBuffer.pack((unsigned short)0x0004);
    sock->writeBuffer.pack((unsigned short)0x0001);
    sock->writeBuffer.pack((unsigned short)0x0000);
    dumpPacket(sock->writeBuffer, pos, "Direct write");
    sock->write();
}

void DirectSocket::connect_ready()
{
    log(L_DEBUG, "Direct connect OK");
    sock->setProxyConnected();
    sendInit();
    state = WaitAck;
    sock->readBuffer.init(2);
    m_bHeader = true;
    sock->read_ready();
}

// ____________________________________________________________________________________________

ICQListener::ICQListener(ICQClient *_client)
{
    sock = _client->createServerSocket();
    sock->setNotify(this);
    client = _client;
}

ICQListener::~ICQListener()
{
    if (sock) delete sock;
}

void ICQListener::accept(Socket *s)
{
    new DirectClient(s, client);
}

unsigned short ICQListener::port()
{
    if (sock) return sock->port();
    return 0;
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
    u = NULL;
    state = WaitLogin;
#ifdef USE_OPENSSL
    ssl = NULL;
#endif
}

DirectClient::DirectClient(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *_u, ICQClient *client)
        : DirectSocket(ip, real_ip, port, _u, client)
{
    u = _u;
    state = None;
#ifdef USE_OPENSSL
    ssl = NULL;
#endif
}

DirectClient::~DirectClient()
{
    if (u && (u->direct == this)) u->direct = NULL;
    secureStop(false);
}

bool DirectClient::isSecure()
{
#ifdef USE_OPENSSL
    return ((ssl != NULL) && ssl->connected());
#else
    return false;
#endif
}

void DirectClient::secureConnect()
{
#ifdef USE_OPENSSL
    if (ssl != NULL) return;
    ssl = new SSLClient(sock->socket());
    if (!ssl->init()){
        delete ssl;
        ssl = NULL;
        return;
    }
    sock->setSocket(ssl);
    state = SSLconnect;
    ssl->connect();
    ssl->process();
#endif
}

void DirectClient::secureListen()
{
#ifdef USE_OPENSSL
    if (ssl != NULL){
        log(L_DEBUG, "secureListen - already secure");
        return;
    }
    ssl = new SSLClient(sock->socket());
    if (!ssl->init()){
        delete ssl;
        ssl = NULL;
        return;
    }
    sock->setSocket(ssl);
    state = SSLconnect;
    ssl->accept();
    ssl->process();
#endif
}

void DirectClient::secureStop(
#ifdef USE_OPENSSL
    bool bShutdown)
#else
    bool)
#endif
{
#ifdef USE_OPENSSL
    if (ssl){
        if (bShutdown){
            ssl->shutdown();
            ssl->process();
        }
        sock->setSocket(ssl->socket());
        delete ssl;
        ssl = NULL;
        if (client && u){
            ICQEvent e(EVENT_STATUS_CHANGED, u->Uin);
            client->process_event(&e);
        }
    }
#endif
}

void DirectClient::processPacket()
{
    switch (state){
    case None:
        log(L_WARN, "DirectClient::processPacket bad state");
        sock->error_state(ErrorProtocol);
        return;
    case WaitInit2:
        if (m_bIncoming) sendInit2();
        state = Logged;
        u->processMsgQueue(client);
        return;
    default:
        break;
    }
    unsigned long hex, key, B1, M1;
    unsigned int i;
    unsigned char X1, X2, X3;

    unsigned int correction = 2;
    if (version >= 7)
        correction++;

    unsigned int size = sock->readBuffer.size()-correction;
    if (version >= 7) sock->readBuffer.incReadPos(1);

    unsigned long check;
    sock->readBuffer.unpack(check);

    // main XOR key
    key = 0x67657268 * size + check;

    unsigned char *p = (unsigned char*)sock->readBuffer.Data(sock->readBuffer.readPos()-4);
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
        log(L_WARN, "Decrypt packet failed");
        sock->error_state(ErrorProtocol);
        return;
    }

    X1 = p[M1] ^ 0xFF;
    if(((B1 >> 16) & 0xFF) != X1){
        log(L_WARN, "Decrypt packet failed");
        sock->error_state(ErrorProtocol);
        return;
    }

    X2 = ((B1 >> 8) & 0xFF);
    if(X2 < 220) {
        X3 = client_check_data[X2] ^ 0xFF;
        if((B1 & 0xFF) != X3){
            log(L_WARN, "Decrypt packet failed");
            sock->error_state(ErrorProtocol);
            return;
        }
    }
    dumpPacket(sock->readBuffer, 0, "Decrypted packed");

    sock->readBuffer.setReadPos(2);
    if (version >= 7){
        char startByte;
        sock->readBuffer.unpack(startByte);
        if (startByte != 0x02){
            log(L_WARN, "Bad start byte");
            sock->error_state(ErrorProtocol);
        }
    }
    unsigned long checksum;
    sock->readBuffer.unpack(checksum);
    unsigned short command;
    sock->readBuffer.unpack(command);
    sock->readBuffer.incReadPos(2);
    unsigned short seq;
    sock->readBuffer.unpack(seq);
    sock->readBuffer.incReadPos(12);

    unsigned short type, ackFlags, msgFlags;
    unsigned short id1 = 0;
    sock->readBuffer.unpack(type);
    sock->readBuffer.unpack(ackFlags);
    sock->readBuffer.unpack(msgFlags);
    string msg_str;
    sock->readBuffer >> msg_str;
    ICQMessage *m = client->parseMessage(type, u->Uin, msg_str, sock->readBuffer, 0, 0, seq, 0);
    switch (command){
    case TCP_START:
        if (m == NULL){
            sock->error_state(ErrorProtocol);
            return;
        }
        m->Received = true;
        m->Direct = true;
        switch (m->Type()){
        case ICQ_MSGxSECURExOPEN:
        case ICQ_MSGxSECURExCLOSE:{
                startPacket(TCP_ACK, seq);
                const char *answer = NULL;
#ifdef USE_OPENSSL
                answer = "1";
#endif
                client->packMessage(sock->writeBuffer, m, answer, 0, 0, 0, true, true);
                sendPacket();
                if (m->Type() == ICQ_MSGxSECURExOPEN){
                    secureListen();
                }else{
                    secureStop(true);
                }
                delete m;
                break;
            }
        case ICQ_READxAWAYxMSG:
        case ICQ_READxOCCUPIEDxMSG:
        case ICQ_READxNAxMSG:
        case ICQ_READxDNDxMSG:
        case ICQ_READxFFCxMSG:{
                startPacket(TCP_ACK, seq);
                sock->writeBuffer.pack(m->Type());
                unsigned short status = 0;
                switch (client->owner->uStatus & 0xFF){
                case ICQ_STATUS_AWAY:
                    status = ICQ_TCPxACK_AWAY;
                    break;
                case ICQ_STATUS_OCCUPIED:
                    status = ICQ_TCPxACK_OCCUPIEDxCAR;
                    break;
                case ICQ_STATUS_DND:
                    status = ICQ_TCPxACK_DNDxCAR;
                    break;
                default:
                    status = ICQ_TCPxACK_NA;
                }
                sock->writeBuffer.pack(status);
                sock->writeBuffer.pack((unsigned short)0);
                string response;
                client->getAutoResponse(u->Uin, response);
                client->toServer(response, u);
                sock->writeBuffer << response;
                sock->writeBuffer << 0x00000000L << 0x00000000L;
                sendPacket();
                delete m;
                break;
            }
        default:
            if ((m->Type() != ICQ_MSGxFILE) && (m->Type() != ICQ_MSGxCHAT))
                sendAck(seq, m->Type());
            client->messageReceived(m);
        }
        break;
    case TCP_CANCEL:
        if (m) client->cancelMessage(m, false);
        break;
    case TCP_ACK:
        if (m){
            id1 = m->id1;
            delete m;
        }
        if (u){
            list<ICQEvent*>::iterator it;
            for (it = u->msgQueue.begin(); it != u->msgQueue.end(); ++it){
                ICQEvent *e = *it;
                if (e->type() != EVENT_MESSAGE_SEND) continue;
                if (e->state != ICQEvent::Send) continue;
                ICQMessage *msg = e->message();
                if (msg == NULL) continue;
                if (msg->id2 == seq){
                    if (ackFlags){
                        client->fromServer(msg_str, u);
                        switch (msg->Type()){
                        case ICQ_READxAWAYxMSG:
                        case ICQ_READxOCCUPIEDxMSG:
                        case ICQ_READxNAxMSG:
                        case ICQ_READxDNDxMSG:
                        case ICQ_READxFFCxMSG:{
                                u->AutoReply = msg_str;
                                ICQEvent eInfo(EVENT_INFO_CHANGED, uin, EVENT_SUBTYPE_AUTOREPLY);
                                client->process_event(&eInfo);
                                u->msgQueue.remove(e);
                                delete e;
                                delete msg;
                                return;
                            }
                        default:
                            break;
                        }
                        msg->DeclineReason = msg_str;
                        client->cancelMessage(msg, false);
                    }else{
                        bool bToProcess = false;
                        switch (msg->Type()){
                        case ICQ_MSGxFILE:{
                                bToProcess = true;
                                ICQFile *file = static_cast<ICQFile*>(msg);
                                file->ft = new FileTransfer(u->IP, u->RealIP, id1, u, client, file);
                                file->ft->connect();
                                break;
                            }
                        case ICQ_MSGxCHAT:{
                                bToProcess = true;
                                ICQChat *chat = static_cast<ICQChat*>(msg);
                                chat->chat = new ChatSocket(u->IP, u->RealIP, id1, u, client, chat);
                                chat->chat->connect();
                                break;
                            }
                        case ICQ_MSGxMSG:
                        case ICQ_MSGxURL:
                            break;
                        case ICQ_MSGxSECURExCLOSE:
                            u->msgQueue.remove(e);
                            delete e;
                            delete msg;
                            secureStop(true);
                            return;
                        case ICQ_MSGxSECURExOPEN:
                            if (!msg_str.c_str()){
                                ICQEvent eSend(EVENT_MESSAGE_SEND, msg->getUin());
                                eSend.setMessage(msg);
                                eSend.state = ICQEvent::Fail;
                                client->process_event(&eSend);
                                u->msgQueue.remove(e);
                                delete e;
                                delete msg;
                            }else{
                                u->msgQueue.remove(e);
                                delete e;
                                delete msg;
                                secureConnect();
                            }
                            return;
                        default:
                            log(L_WARN, "Unknown accept message type");
                            client->cancelMessage(msg, false);
                            return;
                        }
                        u->msgQueue.remove(e);
                        if (bToProcess){
                            client->processQueue.push_back(e);
                            ICQEvent eAck(EVENT_ACKED, msg->getUin());
                            eAck.setMessage(msg);
                            client->process_event(&eAck);
                        }else{
                            ICQEvent eSend(EVENT_MESSAGE_SEND, msg->getUin());
                            eSend.setMessage(msg);
                            eSend.state = ICQEvent::Success;
                            client->process_event(&eSend);
                        }
                    }
                    break;
                }
            }
        }
        break;
    default:
        log(L_WARN, "Unknown TCP command %X", command);
        sock->error_state(ErrorProtocol);
    }
}

void DirectClient::connect_ready()
{
    if (state == None){
        state = WaitLogin;
        DirectSocket::connect_ready();
        return;
    }
    if (state == SSLconnect){
        ICQEvent e(EVENT_STATUS_CHANGED, u->Uin);
        client->process_event(&e);
        state = Logged;
        return;
    }
    if (m_bIncoming){
        u = client->getUser(uin);
        if ((u == NULL) || u->inIgnore){
            log(L_WARN, "Connection from unknown user");
            sock->error_state(ErrorProtocol);
            return;
        }
        if (u->direct){
            sock->error_state(ErrorProtocol);
            return;
        }
        u->direct = this;
        if (version >= 7){
            state = WaitInit2;
        }else{
            state = Logged;
            u->processMsgQueue(client);
        }
    }else{
        if (version >= 7){
            sendInit2();
            state = WaitInit2;
        }else{
            state = Logged;
            u->processMsgQueue(client);
        }
    }
}

void DirectClient::sendInit2()
{
    unsigned pos = sock->writeBuffer.writePos();
    sock->writeBuffer.pack((unsigned short)0x0021);
    sock->writeBuffer.pack((char) 0x03);
    sock->writeBuffer.pack(0x0000000AL);
    sock->writeBuffer.pack(0x00000001L);
    sock->writeBuffer.pack(m_bIncoming ? 0x00000001L : 0x00000000L);
    sock->writeBuffer.pack(0x00000000L);
    sock->writeBuffer.pack(0x00000000L);
    if (m_bIncoming) {
        sock->writeBuffer.pack(0x00040001L);
        sock->writeBuffer.pack(0x00000000L);
        sock->writeBuffer.pack(0x00000000L);
    } else {
        sock->writeBuffer.pack(0x00000000L);
        sock->writeBuffer.pack(0x00000000L);
        sock->writeBuffer.pack(0x00040001L);
    }
    dumpPacket(sock->writeBuffer, pos, "Direct write");
    sock->write();
}

bool DirectClient::error_state(SocketError err)
{
    if (!DirectSocket::error_state(err)) return false;
    if (u){
        list<ICQEvent*>::iterator it;
        for (it = u->msgQueue.begin(); it != u->msgQueue.end(); it = u->msgQueue.begin()){
            ICQEvent *e = *it;
            u->msgQueue.remove(e);
            e->state = ICQEvent::Fail;
            e->message()->bDelete = true;
            client->process_event(e);
            if (e->message()->bDelete) delete e->message();
            delete e;
        }
    }
    return true;
}

void DirectClient::sendAck(unsigned short seq, unsigned short type)
{
    startPacket(TCP_ACK, seq);
    sock->writeBuffer.pack(type);
    sock->writeBuffer << 0x00000000L << 0x00000000L << 0x00000000L;
    sendPacket();
}

void DirectClient::startPacket(unsigned short cmd, unsigned short seq)
{
    m_packetOffs = sock->writeBuffer.writePos();
    sock->writeBuffer
    << (unsigned short)0;	// size
    if (version >= 7)
        sock->writeBuffer << (char)0x02;
    if (seq == 0)
        seq = --m_nSequence;
    sock->writeBuffer
    << (unsigned long)0;			// checkSum
    sock->writeBuffer.pack(cmd);
    sock->writeBuffer
    << (char) 0x0E << (char) 0;
    sock->writeBuffer.pack(seq);
    sock->writeBuffer
    << (unsigned long)0
    << (unsigned long)0
    << (unsigned long)0;
}

void DirectClient::sendPacket()
{
    unsigned size = sock->writeBuffer.size() - m_packetOffs - 2;
    unsigned char *p = (unsigned char*)(sock->writeBuffer.Data(m_packetOffs));
    *((unsigned short*)p) = size;

    dumpPacket(sock->writeBuffer, m_packetOffs, "Direct send");

    unsigned long hex, key, B1, M1;
    unsigned long i, check;
    unsigned char X1, X2, X3;

    p += 2;
    if (version >= 7){
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
    dumpPacket(sock->writeBuffer, m_packetOffs, "Encrypted packet");
    sock->write();
}

void DirectClient::acceptMessage(ICQMessage *m)
{
    startPacket(TCP_ACK, m->timestamp1);
    client->packMessage(sock->writeBuffer, m, NULL, 0, 0, 0, true, true);
    sendPacket();
}

void DirectClient::declineMessage(ICQMessage *m, const char *reason)
{
    startPacket(TCP_ACK, m->timestamp1);
    client->packMessage(sock->writeBuffer, m, reason, 1, 0, 0, true, true);
    sendPacket();
}

unsigned short DirectClient::sendMessage(ICQMessage *msg)
{
    bool bConvert = true;
    string message;
    switch (msg->Type()){
    case ICQ_MSGxFILE:{
            ICQFile *file = static_cast<ICQFile*>(msg);
            message = file->Description;
            break;
        }
    case ICQ_MSGxCHAT:{
            ICQChat *chat = static_cast<ICQChat*>(msg);
            message = chat->Reason;
            chat->id1 = client->owner->Port;
            break;
        }
    case ICQ_MSGxMSG:{
            ICQMsg *m = static_cast<ICQMsg*>(msg);
            message = client->makeMessageText(m, u);
            break;
        }
    case ICQ_MSGxURL:{
            ICQUrl *url = static_cast<ICQUrl*>(msg);
            message = client->clearHTML(url->Message.c_str());
            client->toServer(message, u);
            message += '\xFE';
            message += url->URL.c_str();
            bConvert = false;
            break;
        }
    case ICQ_MSGxCONTACTxLIST:{
            ICQContacts *m = static_cast<ICQContacts*>(msg);
            message = "";
            unsigned nContacts = m->Contacts.size();
            char u[13];
            snprintf(u, sizeof(u), "%u", nContacts);
            message += u;
            for (ContactList::iterator it_msg = m->Contacts.begin(); it_msg != m->Contacts.end(); it_msg++){
                Contact *contact = static_cast<Contact*>(*it_msg);
                message += '\xFE';
                snprintf(u, sizeof(u), "%lu", contact->Uin);
                message += u;
                message += '\xFE';
                string alias = contact->Alias;
                client->toServer(alias, u);
                message += alias.c_str();
            }
            message += '\xFE';
            bConvert = false;
            break;
        }
    case ICQ_MSGxSECURExOPEN:
    case ICQ_MSGxSECURExCLOSE:
    case ICQ_READxAWAYxMSG:
    case ICQ_READxOCCUPIEDxMSG:
    case ICQ_READxNAxMSG:
    case ICQ_READxDNDxMSG:
    case ICQ_READxFFCxMSG:
        break;
    default:
        log(L_WARN, "Unknown type %u for direct send", msg->Type());
        return 0;
    }
    startPacket(TCP_START, 0);
    client->packMessage(sock->writeBuffer, msg, message.c_str(), 0, 0, 0, true, bConvert);
    sendPacket();
    return m_nSequence;
}

// ____________________________________________________________________________________________

FileTransferListener::FileTransferListener(ICQFile *_file, ICQClient *_client)
{
    sock = _client->createServerSocket();
    sock->setNotify(this);
    file = _file;
    client = _client;
}

void FileTransferListener::accept(Socket *s)
{
    file->listener = NULL;
    file->ft = new FileTransfer(s, client, file);
    delete this;
}

unsigned short FileTransferListener::port()
{
    if (sock) return sock->port();
    return 0;
}

FileTransfer::FileTransfer(Socket *s, ICQClient *client, ICQFile *_file)
        : DirectSocket(s, client)
{
    state = WaitLogin;
    file = _file;
    init();
}

FileTransfer::FileTransfer(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClient *client, ICQFile *_file)
        : DirectSocket(ip, real_ip, port, u, client)
{
    state = None;
    file = _file;
    init();
}

void FileTransfer::init()
{
    m_nSpeed = 100;
    m_sendTime = 0;
    m_sendSize = 0;
    m_fileSize = 0;
    m_totalSize = 0;
}

void FileTransfer::write_ready()
{
    if (state != Send) {
        DirectSocket::write_ready();
        return;
    }
    log(L_DEBUG, "> %u %u %u %u", m_curFile, m_nFiles, m_curSize, m_fileSize);
    if (m_fileSize >= m_curSize){
        state = None;
        client->closeFile(file);
        m_curFile++;
        if (m_curFile >= m_nFiles){
            file->ft = NULL;
            list<ICQEvent*>::iterator it;
            for (it = client->processQueue.begin(); it != client->processQueue.end(); ++it){
                ICQEvent *e = *it;
                if (e->message() != file) continue;
                client->processQueue.remove(e);
                e->state = ICQEvent::Success;
                e->setType(EVENT_DONE);
                client->process_event(e);
                file->ft = NULL;
                delete file;
                delete e;
                file = NULL;
                break;
            }
            sock->error_state(ErrorCancel);
            return;
        }
        curName = file->files[m_curFile].name;
        m_curSize = file->files[m_curFile].size;
        m_fileSize = 0;
        state = InitSend;
        sendFileInfo();
        return;
    }
    time_t now;
    time(&now);
    if ((unsigned)now != m_sendTime){
        m_sendTime = now;
        m_sendSize = 0;
    }
    if (m_sendSize > m_nSpeed * 2048){
        sock->pause(1);
        return;
    }
    unsigned long tail = m_curSize - m_fileSize;
    if (tail > 2048) tail = 2048;
    startPacket(FT_DATA);
    unsigned long pos = sock->writeBuffer.writePos();
    if (!client->readFile(file, sock->writeBuffer, tail)){
        log(L_WARN, "Error read file");
        sock->error_state(ErrorProtocol);
        return;
    }
    m_fileSize += (sock->writeBuffer.writePos() - pos);
    m_totalSize += (sock->writeBuffer.writePos() - pos);
    sendPacket(false);
    m_sendSize += tail;
}


void FileTransfer::resume(int mode)
{
    if (state != Wait) return;
    if (mode == FT_SKIP){
        m_fileSize = m_curSize;
    }else{
        if (!client->createFile(file, mode)){
            if (file->wait){
                state = Wait;
                return;
            }
            m_fileSize = m_curSize;
        }
    }
    startPacket(FT_START);
    sock->writeBuffer.pack((unsigned long)m_fileSize);
    sock->writeBuffer.pack((unsigned long)0);
    sock->writeBuffer.pack(m_nSpeed);
    sock->writeBuffer.pack((unsigned long)(m_curFile+1));
    sendPacket();
    state = Receive;
}

void FileTransfer::setSpeed(int nSpeed)
{
    m_nSpeed = nSpeed;
    startPacket(FT_SPEED);
    sock->writeBuffer.pack(m_nSpeed);
    sendPacket();
}

void FileTransfer::sendFileInfo()
{
    startPacket(FT_FILEINFO);
    sock->writeBuffer.pack((char)0);
    string empty;
    string s = curName;
    ICQUser *u = client->getUser(file->getUin());
    client->toServer(s, u);
    sock->writeBuffer << s << empty;
    sock->writeBuffer.pack(m_curSize);
    sock->writeBuffer.pack((unsigned long)0);
    sock->writeBuffer.pack(m_nSpeed);
    sendPacket();
}

void FileTransfer::processPacket()
{
    char cmd;
    sock->readBuffer >> cmd;
    if (cmd == FT_SPEED){
        sock->readBuffer.unpack(m_nSpeed);
        return;
    }
    switch (state){
    case WaitInit:{
            if (cmd != FT_INIT){
                log(L_WARN, "No init command");
                sock->error_state(ErrorProtocol);
                return;
            }
            unsigned long n;
            sock->readBuffer.unpack(n);
            sock->readBuffer.unpack(m_nFiles);
            sock->readBuffer.unpack(n);
            file->Size = n;
            state = InitReceive;
            startPacket(FT_SPEED);
            sock->writeBuffer.pack(m_nSpeed);
            sendPacket();
            startPacket(FT_INIT_ACK);
            sock->writeBuffer.pack(m_nSpeed);
            char b[12];
            snprintf(b, sizeof(b), "%lu", client->owner->Uin);
            string uin = b;
            sock->writeBuffer << uin;
            sendPacket();
            break;
        }
    case InitReceive:{
            if (cmd != FT_FILEINFO){
                log(L_WARN, "Bad command in init receive");
                sock->error_state(ErrorProtocol);
                return;
            }
            sock->readBuffer.incReadPos(1);
            sock->readBuffer >> curName;
            ICQUser *u = client->getUser(file->getUin());
            client->fromServer(curName, u);
            string empty;
            sock->readBuffer >> empty;
            sock->readBuffer.unpack(m_curSize);
            state = Wait;
            resume(FT_DEFAULT);
            break;
        }
    case InitSend:
        switch (cmd){
        case FT_INIT_ACK:{
                sendFileInfo();
                break;
            }
        case FT_START:{
                unsigned long pos, empty, speed, curFile;
                sock->readBuffer.unpack(pos);
                sock->readBuffer.unpack(empty);
                sock->readBuffer.unpack(speed);
                sock->readBuffer.unpack(curFile);
                log(L_DEBUG, "Start send at %lu %lu", pos, curFile);
                m_fileSize = pos;
                if (!client->openFile(file) || !client->seekFile(file, pos)){
                    log(L_WARN, "Can't open file");
                    sock->error_state(ErrorProtocol);
                    return;
                }
                state = Send;
                write_ready();
            }
            break;
        default:
            log(L_WARN, "Bad init client command %X", cmd);
            sock->error_state(ErrorProtocol);
        }
        break;
    case Receive:{
            if (cmd != FT_DATA){
                log(L_WARN, "Bad data command");
                sock->error_state(ErrorProtocol);
                return;
            }
            unsigned short size = sock->readBuffer.size() - sock->readBuffer.readPos();
            m_fileSize += size;
            m_totalSize += size;
            if (!client->writeFile(file, sock->readBuffer)){
                log(L_WARN, "Error write file");
                sock->error_state(ErrorProtocol);
                return;
            }
            if (m_fileSize >= m_curSize){
                client->closeFile(file);
                m_curFile++;
                m_fileSize = 0;
                if (m_curFile >= m_nFiles){
                    log(L_DEBUG, "File transfer OK");
                    file->ft = NULL;
                    list<ICQEvent*>::iterator it;
                    for (it = client->processQueue.begin(); it != client->processQueue.end(); ++it){
                        ICQEvent *e = *it;
                        if (e->message() != file) continue;
                        client->processQueue.remove(e);
                        e->setType(EVENT_DONE);
                        e->state = ICQEvent::Success;
                        client->process_event(e);
                        file->ft = NULL;
                        delete e;
                        delete file;
                        file = NULL;
                        break;
                    }
                    sock->error_state(ErrorCancel);
                    state = None;
                    return;
                }
                state = InitReceive;
                return;
            }
            break;
        }
    default:
        log(L_WARN, "Packet in bad state");
        sock->error_state(ErrorProtocol);
        return;
    }
}

bool FileTransfer::error_state(SocketError err)
{
    if (!DirectSocket::error_state(err))
        return false;
    state = None;
    if (file){
        file->ft = NULL;
        client->cancelMessage(file);
    }
    return true;
}

void FileTransfer::connect_ready()
{
    if (state == None){
        state = WaitLogin;
        DirectSocket::connect_ready();
        return;
    }
    m_curFile = 0;
    log(L_DEBUG, "Connected");
    if (m_bIncoming){
        state = WaitInit;
    }else{
        m_nFiles = file->files.size();
        state = InitSend;
        startPacket(FT_INIT);
        sock->writeBuffer.pack((unsigned long)0);
        sock->writeBuffer.pack((unsigned long)(m_nFiles));		// nFiles
        sock->writeBuffer.pack(file->Size);						// Total size
        sock->writeBuffer.pack(m_nSpeed);							// speed
        char b[12];
        snprintf(b, sizeof(b), "%lu", client->owner->Uin);
        string uin = b;
        sock->writeBuffer << uin;
        sendPacket();
        if (m_nFiles == 0) sock->error_state(ErrorCancel);
        curName = file->files[0].name;
        m_curSize = file->files[0].size;
    }
}

void FileTransfer::startPacket(char cmd)
{
    m_packetOffs = sock->writeBuffer.writePos();
    sock->writeBuffer << (unsigned short)0;
    sock->writeBuffer << cmd;
}

void FileTransfer::sendPacket(bool dump)
{
    unsigned size = sock->writeBuffer.size() - m_packetOffs - 2;
    unsigned char *p = (unsigned char*)(sock->writeBuffer.Data(m_packetOffs));
    *((unsigned short*)p) = size;
    if (dump) dumpPacket(sock->writeBuffer, m_packetOffs, "File transfer send");
    sock->write();
}

// ___________________________________________________________________________________________

ChatListener::ChatListener(ICQChat *_chat, ICQClient *_client)
{
    sock = _client->createServerSocket();
    sock->setNotify(this);
    chat = _chat;
    client = _client;
}

void ChatListener::accept(Socket *s)
{
    chat->listener = NULL;
    chat->chat = new ChatSocket(s, client, chat);
    delete this;
}

unsigned short ChatListener::port()
{
    if (sock) return sock->port();
    return 0;
}

ChatSocket::ChatSocket(Socket *s, ICQClient *client, ICQChat *_chat)
        : DirectSocket(s, client)
{
    chat = _chat;
    state = WaitLogin;
    init();
}

ChatSocket::ChatSocket(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClient *client, ICQChat *_chat)
        : DirectSocket(ip, real_ip, port, u, client)
{
    chat = _chat;
    state = None;
    init();
}

void ChatSocket::init()
{
    fontSize = 12;
    fontFace = 0;
    fontFamily = "MS Sans Serif";
    myFontFace = 0;
    curMyFontFace = 0;
    myFgColor = 0;
    curMyFgColor = 0;
    bgColor = 0xFFFFFF;
    fgColor = 0;
}

void ChatSocket::sendLine(const char *str)
{
    unsigned long writePos = sock->writeBuffer.writePos();
    for (;str;){
        string s;
        char *end = NULL;
        char *tag = strchr(str, '<');
        if (tag){
            s.assign(str, tag - str);
        }else{
            s = str;
        }
        if (s.size()){
            if (curMyFontFace != myFontFace){
                myFontFace = curMyFontFace;
                sock->writeBuffer.pack(CHAT_ESCAPE);
                sock->writeBuffer.pack(CHAT_FONT_FACE);
                sock->writeBuffer.pack((unsigned long)4);
                sock->writeBuffer.pack(myFontFace);
            }
            if (curMyFgColor != myFgColor){
                myFgColor = curMyFgColor;
                sock->writeBuffer.pack(CHAT_ESCAPE);
                sock->writeBuffer.pack(CHAT_COLORxFG);
                sock->writeBuffer.pack((unsigned long)4);
                sock->writeBuffer.pack(myFgColor);
            }
            string s1 = client->clearHTML(s.c_str());
            ICQUser *u = client->getUser(chat->getUin());
            client->toServer(s1, u);
            sock->writeBuffer.pack(s1.c_str(), s1.size());
        }
        if (tag){
            tag++;
            end = strchr(tag, '>');
            if (end){
                string t;
                string opt;
                t.assign(tag, end - tag);
                const char *tend = strchr(t.c_str(), ' ');
                if (tend){
                    opt = t.substr(tend - t.c_str());
                    t = t.substr(0, tend - t.c_str());
                }
                end++;
                if ((t == string("b")) || (t == string("B")))
                    curMyFontFace |= FONT_BOLD;
                if ((t == string("/b")) || (t == string("/B")))
                    curMyFontFace &= (~FONT_BOLD);
                if ((t == string("i")) || (t == string("I")))
                    curMyFontFace |= FONT_ITALIC;
                if ((t == string("/i")) || (t == string("/I")))
                    curMyFontFace &= (~FONT_ITALIC);
                if ((t == string("u")) || (t == string("U")))
                    curMyFontFace |= FONT_UNDERLINE;
                if ((t == string("/u")) || (t == string("/U")))
                    curMyFontFace &= (~FONT_UNDERLINE);
                if (t == string("font")){
                    char COLOR[] = "color=";
                    const char *p = opt.c_str();
                    for (; *p; p++){
                        for (; *p; p++)
                            if (*p != ' ') break;
                        if (strlen(p) < strlen(COLOR))
                            break;
                        if (memcmp(p, COLOR, strlen(COLOR)) == 0){
                            p += strlen(COLOR);
                            if (*p == '\"') p++;
                            if (*p == '#') p++;
                            unsigned newColor = 0;
                            for (; *p; p++){
                                char c = *p;
                                if ((c >= '0') && (c <= '9')){
                                    newColor = (newColor << 4) + (c - '0');
                                    continue;
                                }
                                if ((c >= 'A') && (c <= 'F')){
                                    newColor = (newColor << 4) + (c - 'A' + 10);
                                    continue;
                                }
                                if ((c >= 'a') && (c <= 'f')){
                                    newColor = (newColor << 4) + (c - 'a' + 10);
                                    continue;
                                }
                                break;
                            }
                            curMyFgColor = ((newColor >> 16) & 0xFF) +
                                           (((newColor >> 8) & 0xFF) << 8) +
                                           ((newColor & 0xFF) << 16);
                        }
                        for (; *p; p++)
                            if (*p == ' ') break;
                    }
                }
            }
        }
        str = end;
    }
    sock->writeBuffer.pack(' ');
    sock->writeBuffer.pack(CHAT_NEWLINE);
    dumpPacket(sock->writeBuffer, writePos, "Chat send");
    sock->write();
}

void ChatSocket::putText(string &s)
{
    if (s.size() == 0) return;
    ICQUser *u = client->getUser(chat->getUin());
    client->fromServer(s, u);
    ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_TEXT, chat);
    e.text = s;
    client->process_event(&e);
    s = "";
}

void ChatSocket::packet_ready()
{
    if (state != Connected){
        DirectSocket::packet_ready();
        return;
    }

    dumpPacket(sock->readBuffer, sock->readBuffer.readPos(), "Chat read");

    string chatText;
    for (;;){
        if (sock->readBuffer.readPos() >= sock->readBuffer.size()) break;
        char c;
        sock->readBuffer >> c;
        switch (c){
        case CHAT_ESCAPE:{
                putText(chatText);
                if (sock->readBuffer.readPos() + 5 > sock->readBuffer.size()){
                    sock->readBuffer.incReadPos(-1);
                    return;
                }
                sock->readBuffer >> c;
                unsigned long size;
                sock->readBuffer.unpack(size);
                if (sock->readBuffer.readPos() + size > sock->readBuffer.size()){
                    sock->readBuffer.incReadPos(-6);
                    return;
                }
                Buffer b;
                if (size){
                    b.init(size);
                    sock->readBuffer.unpack(b.Data(0), size);
                    b.setWritePos(size);
                }
                switch (c){
                case CHAT_FONT_FACE:
                    b.unpack(fontFace);
                    break;
                case CHAT_COLORxFG:
                    b.unpack(fgColor);
                    break;
                case CHAT_COLORxBG:
                    b.unpack(bgColor);
                    break;
                }
                ICQEvent e(EVENT_CHAT, chat->getUin(), c, chat);
                client->process_event(&e);
                break;
            }
        case CHAT_BACKSPACE:{
                putText(chatText);
                ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_BACKSPACE, chat);
                client->process_event(&e);
                break;
            }
        case CHAT_NEWLINE:{
                putText(chatText);
                ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_NEWLINE, chat);
                client->process_event(&e);
                break;
            }
        default:
            chatText += c;
        }
    }
    sock->readBuffer.init(0);
    putText(chatText);
}

void ChatSocket::processPacket()
{
    switch (state){
    case WaitInit:{
            sock->readBuffer.incReadPos(8);
            unsigned long uin;
            sock->readBuffer.unpack(uin);
            startPacket();
            sock->writeBuffer.pack((unsigned long)0x65);
            sock->writeBuffer.pack(client->owner->Uin);
            string alias = client->owner->name();
            sock->writeBuffer << alias;
            sock->writeBuffer.pack(fgColor);
            sock->writeBuffer.pack(bgColor);
            sock->writeBuffer.pack((unsigned long)version);
            sock->writeBuffer.pack((unsigned long)(client->owner->Port));
            sock->writeBuffer.pack(client->owner->RealIP);
            sock->writeBuffer.pack(client->owner->IP);
            sock->writeBuffer.pack((char)0x01);
            unsigned short session = rand();
            sock->writeBuffer.pack(session);
            sock->writeBuffer.pack(fontSize);
            sock->writeBuffer.pack(myFontFace);
            sock->writeBuffer << fontFamily;
            unsigned short w = 0;
            sock->writeBuffer .pack(w);
            char nClients = 0;
            sock->writeBuffer.pack(nClients);
            sendPacket();
            state = WaitFont;
            break;
        }
    case WaitFont:
        {
            sock->readBuffer.incReadPos(19);
            sock->readBuffer.unpack(fontSize);
            sock->readBuffer.unpack(fontFace);
            sock->readBuffer.unpack(fontFamily);
            state = Connected;
            ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_CONNECT, chat);
            client->process_event(&e);
            sock->setRaw(true);
            break;
        }
    case WaitFontInfo:{
            sock->readBuffer.incReadPos(4);
            unsigned long uin;
            string alias;
            unsigned long fgColor, bgColor;
            sock->readBuffer.unpack(uin);
            sock->readBuffer.unpack(alias);
            sock->readBuffer.unpack(bgColor);
            sock->readBuffer.unpack(fgColor);
            log(L_DEBUG, "Info %lu %s %lX %lX", uin, alias.c_str(), fgColor, bgColor);
            unsigned long version, port, ip, real_ip;
            char mode;
            unsigned short session;
            sock->readBuffer.unpack(version);
            sock->readBuffer.unpack(port);
            sock->readBuffer.unpack(real_ip);
            sock->readBuffer.unpack(ip);
            sock->readBuffer.unpack(mode);
            sock->readBuffer.unpack(session);
            struct in_addr addr;
            addr.s_addr = ip;
            string ip_str = inet_ntoa(addr);
            addr.s_addr = real_ip;
            string real_ip_str = inet_ntoa(addr);
            log(L_DEBUG, "Inof %lu %lu %s %s %u", version, port,
                ip_str.c_str(), real_ip_str.c_str(),
                mode);
            sock->readBuffer.unpack(fontSize);
            sock->readBuffer.unpack(fontFace);
            sock->readBuffer.unpack(fontFamily);
            log(L_DEBUG, "Font info: %lu %lu %s", fontSize, fontFace, fontFamily.c_str());
            sock->readBuffer.incReadPos(2);
            char nClients;
            sock->readBuffer.unpack(nClients);
            log(L_DEBUG, "Clients: %u", nClients);
            for (; nClients > 0; nClients--){
                unsigned long version, port, uin, ip, real_ip, handshake;
                unsigned short session;
                char mode;
                sock->readBuffer.unpack(version);
                sock->readBuffer.unpack(port);
                sock->readBuffer.unpack(uin);
                sock->readBuffer.unpack(ip);
                sock->readBuffer.unpack(real_ip);
                sock->readBuffer.incReadPos(2);
                sock->readBuffer.unpack(mode);
                sock->readBuffer.unpack(session);
                sock->readBuffer.unpack(handshake);
                struct in_addr addr;
                addr.s_addr = ip;
                string ip_str = inet_ntoa(addr);
                addr.s_addr = real_ip;
                string real_ip_str = inet_ntoa(addr);
                log(L_DEBUG, "Client %lu %lu %lu %ls %s %u %u %lu",
                    uin, version, port, ip_str.c_str(), real_ip_str.c_str(),
                    mode, session, handshake);
            }

            startPacket();
            sock->writeBuffer.pack((unsigned long)version);
            sock->writeBuffer.pack((unsigned long)(client->owner->Port));
            sock->writeBuffer.pack(client->owner->RealIP);
            sock->writeBuffer.pack(client->owner->IP);
            sock->writeBuffer.pack((char)0x01);
            sock->writeBuffer.pack(session);
            sock->writeBuffer.pack(fontSize);
            sock->writeBuffer.pack(myFontFace);
            sock->writeBuffer << fontFamily;
            sock->writeBuffer.pack((unsigned short)1);
            sendPacket();
            state = Connected;
            ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_CONNECT, chat);
            client->process_event(&e);
            sock->setRaw(true);
            break;
        }
    default:
        log(L_WARN, "Chat packet in unknown state");
        sock->error_state(ErrorProtocol);
        return;
    }
}

void ChatSocket::connect_ready()
{
    if (state == None){
        state = WaitLogin;
        DirectSocket::connect_ready();
        return;
    }
    if (m_bIncoming){
        state = WaitInit;
    }else{
        startPacket();
        sock->writeBuffer.pack((unsigned long)0x64);
        sock->writeBuffer.pack((unsigned long)(-7));
        sock->writeBuffer.pack(client->owner->Uin);
        char b[12];
        snprintf(b, sizeof(b), "%lu", client->owner->Uin);
        string uin = b;
        sock->writeBuffer << uin;
        sock->writeBuffer.pack(client->listener->port());
        sock->writeBuffer.pack(fgColor);
        sock->writeBuffer.pack(bgColor);
        sock->writeBuffer.pack((char)0);
        sendPacket();
        state = WaitFontInfo;
    }
}

void ChatSocket::startPacket()
{
    m_packetOffs = sock->writeBuffer.writePos();
    sock->writeBuffer << (unsigned short)0;
}

void ChatSocket::sendPacket()
{
    unsigned size = sock->writeBuffer.size() - m_packetOffs - 2;
    unsigned char *p = (unsigned char*)sock->writeBuffer.Data(m_packetOffs);
    *((unsigned short*)p) = size;
    dumpPacket(sock->writeBuffer, m_packetOffs, "Chat send");
    sock->write();
}

bool ChatSocket::error_state(SocketError err)
{
    if (!DirectSocket::error_state(err))
        return false;
    ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_CONNECT, chat);
    e.state = ICQEvent::Fail;
    client->process_event(&e);
    return false;
}

ICQEvent *ICQUser::addMessage(ICQMessage *msg, ICQClient *client)
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); it++){
        if ((*it)->message() == msg) return NULL;
    }
    msg->Id = client->m_nProcessId++;
    ICQEvent *e = new ICQEvent(EVENT_MESSAGE_SEND);
    e->setMessage(msg);
    msgQueue.push_back(e);
    processMsgQueue(client);
    return e;
}

void ICQUser::processMsgQueue(ICQClient *client)
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end();){
        if ((*it)->message()->state == ICQEvent::Send){
            ++it;
            continue;
        }
        if (direct == NULL){
            if ((Port == 0) || ((IP == 0) && (RealIP == 0))){
                for (it = msgQueue.begin(); it != msgQueue.end(); ){
                    ICQEvent *e = *it;
                    e->state = ICQEvent::Fail;
                    e->message()->bDelete = true;
                    client->process_event(e);
                    if (e->message()->bDelete) delete e->message();
                    msgQueue.remove(e);
                    delete e;
                    it = msgQueue.begin();
                }
                return;
            }
            direct = new DirectClient(IP, RealIP, Port, this, client);
            direct->connect();
            return;
        }
        if (direct->state != DirectClient::Logged) return;
        ICQEvent *e = *it;
        unsigned short seq = direct->sendMessage(e->message());
        if (seq){
            e->state = ICQEvent::Send;
            e->message()->id2 = seq;
            ++it;
            continue;
        }
        it = msgQueue.begin();
        msgQueue.remove(e);
        e->state = ICQEvent::Fail;
        e->message()->bDelete = true;
        client->process_event(e);
        if (e->message()->bDelete) delete e->message();
        delete e;
        e = NULL;
        it = msgQueue.begin();
    }
}

void ICQUser::requestSecureChannel(ICQClient *client)
{
    if (direct && direct->isSecure())
        return;
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); ++it){
        if ((*it)->message()->Type() == ICQ_MSGxSECURExOPEN) return;
    }
    ICQSecureOn *msg = new ICQSecureOn;
    msg->Uin.push_back(Uin);
    addMessage(msg, client);
}

void ICQUser::closeSecureChannel(ICQClient *client)
{
    if ((direct == NULL) || !direct->isSecure())
        return;
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); ++it){
        if ((*it)->message()->Type() == ICQ_MSGxSECURExCLOSE) return;
    }
    ICQSecureOff *msg = new ICQSecureOff;
    msg->Uin.push_back(Uin);
    addMessage(msg, client);
}
