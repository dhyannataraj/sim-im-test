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

#include "icqclient.h"
#include "log.h"

#include <time.h>
#include <errno.h>

#ifndef WIN32
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

DirectSocket::DirectSocket(int fd, const char *host, unsigned short port, ICQClient *_client)
        : ClientSocket(fd, host, port)
{
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
        : ClientSocket(-1, NULL, 0)
{
    m_bIncoming = false;
    ip = _ip;
    real_ip = _real_ip;
    port = _port;
    uin = u->Uin();
    version = u->Version();
    DCcookie = u->DCcookie;
    if (version > 8) version = 8;
    client = _client;
    state = NotConnected;
    init();
}

void DirectSocket::init()
{
    m_nSequence = 0xFFFF;
    writeBuffer.init(0);
    readBuffer.init(2);
    m_bHeader = true;
}

void DirectSocket::error_state()
{
    ClientSocket::error_state();
    if ((state == ConnectIP) || (state == ConnectRealIP)){
        connect();
    }else{
        remove();
    }
}

void DirectSocket::connect()
{
    writeBuffer.init(0);
    readBuffer.init(2);
    m_bHeader = true;
    if (port == 0){
        log(L_WARN, "Connect to unknown port");
        return;
    }
    if (state == NotConnected){
        state = ConnectRealIP;
        if (real_ip != 0){
            struct in_addr addr;
            addr.s_addr = real_ip;
            ClientSocket::connect(inet_ntoa(addr), port);
            return;
        }
    }
    if (state == ConnectRealIP){
        state = ConnectIP;
        if (ip != 0){
            struct in_addr addr;
            addr.s_addr = ip;
            ClientSocket::connect(inet_ntoa(addr), port);
            return;
        }
    }
    log(L_WARN, "Can't established direct connection");
    remove();
}

void DirectSocket::packet_ready()
{
    if (m_bHeader){
        unsigned short size;
        readBuffer.unpack(size);
        if (size){
            readBuffer.add(size);
            m_bHeader = false;
            return;
        }
    }
    dumpPacket(readBuffer, 0,"Direct read");
    switch (state){
    case Logged:{
            processPacket();
            break;
        }
    case WaitAck:{
            unsigned short s1, s2;
            readBuffer.unpack(s1);
            readBuffer.unpack(s2);
            if ((s1 != 1) || (s2 != 0)){
                log(L_WARN, "Bad ack %X %X", s1, s2);
                error(ErrorProtocol);
                return;
            }
            if (m_bIncoming){
                state = Logged;
                connected();
            }else{
                state = WaitInit;
            }
            break;
        }
    case WaitInit:{
            char cmd;
            readBuffer.unpack(cmd);
            if ((unsigned char)cmd != 0xFF){
                log(L_WARN, "Bad direct init command (%X)", cmd & 0xFF);
                error(ErrorProtocol);
                return;
            }
            readBuffer.unpack(version);
            if (version < 6){
                log(L_WARN, "Use old protocol");
                error(ErrorProtocol);
                return;
            }
            readBuffer.incReadPos(3);
            unsigned long my_uin;
            readBuffer.unpack(my_uin);
            if (my_uin != client->Uin()){
                log(L_WARN, "Bad UIN");
                error(ErrorProtocol);
                return;
            }
            readBuffer.incReadPos(6);
            unsigned long p_uin;
            readBuffer.unpack(p_uin);
            if (m_bIncoming){
                ICQUser *user = client->getUser(p_uin);
                if ((user == NULL) || user->inIgnore()){
                    log(L_WARN, "User %lu not found", uin);
                    error(ErrorProtocol);
                    return;
                }
                uin = p_uin;
            }else{
                if (p_uin != uin){
                    log(L_WARN, "Bad sender UIN");
                    error(ErrorProtocol);
                    return;
                }
            }
            readBuffer.incReadPos(13);
            unsigned long sessionId;
            readBuffer.unpack(sessionId);
            if (m_bIncoming){
                m_nSessionId = sessionId;
                sendInitAck();
                sendInit();
                state = WaitAck;
            }else{
                if (sessionId != m_nSessionId){
                    log(L_WARN, "Bad session ID");
                    error(ErrorProtocol);
                    return;
                }
                sendInitAck();
                state = Logged;
                connected();
            }
            break;
        }
    default:
        log(L_WARN, "Bad state");
        error(ErrorProtocol);
        return;
    }
    readBuffer.init(2);
    m_bHeader = true;
}

void DirectSocket::sendInit()
{
    log(L_DEBUG, "Send init packet");
    if (!m_bIncoming){
        if (DCcookie == 0){
            log(L_WARN, "No direct info");
            error(ErrorProtocol);
            return;
        }
        m_nSessionId = DCcookie;
    }

    char *host;
    unsigned short port;
    getLocalAddr(host, port);
    port = 0;

    unsigned pos = writeBuffer.writePos();
    writeBuffer.pack((unsigned short)((version >= 7) ? 0x0030 : 0x002c));
    writeBuffer.pack((char)0xFF);
    writeBuffer.pack((unsigned short)version);
    writeBuffer.pack((unsigned short)((version >= 7) ? 0x002b : 0x0027));
    writeBuffer.pack(uin);
    writeBuffer.pack((unsigned short)0x0000);
    writeBuffer.pack((unsigned long)port);
    writeBuffer.pack(client->Uin());
    writeBuffer.pack(client->IP());
    writeBuffer.pack((unsigned long)inet_addr(host));
    writeBuffer.pack((char)0x01);
    writeBuffer.pack((unsigned long)port);
    writeBuffer.pack(m_nSessionId);
    writeBuffer.pack(0x00000050L);
    writeBuffer.pack(0x00000003L);
    if (version >= 7)
        writeBuffer.pack(0x00000000L);
    dumpPacket(writeBuffer, pos, "Direct write");
}

void DirectSocket::sendInitAck()
{
    unsigned pos = writeBuffer.writePos();
    writeBuffer.pack((unsigned short)0x0004);
    writeBuffer.pack((unsigned short)0x0001);
    writeBuffer.pack((unsigned short)0x0000);
    dumpPacket(writeBuffer, pos, "Direct write");
}

void DirectSocket::connect_ready()
{
    ClientSocket::connect_ready();
    log(L_DEBUG, "Direct connect OK");
    sendInit();
    state = WaitAck;
    readBuffer.init(2);
    m_bHeader = true;
}

// ____________________________________________________________________________________________

ICQListener::ICQListener(ICQClient *_client)
{
    client = _client;
}

void ICQListener::accept(int fd, const char *host, unsigned short port)
{
    new DirectClient(fd, host, port, client);
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

DirectClient::DirectClient(int fd, const char *host, unsigned short port, ICQClient *client)
        : DirectSocket(fd, host, port, client)
{
    u = NULL;
    state = None;
}

DirectClient::~DirectClient()
{
    if (u && (u->direct == this)) u->direct = NULL;
}

void DirectClient::processPacket()
{
    switch (state){
    case None:
        log(L_WARN, "DirectClient::processPacket bad state");
        error(ErrorProtocol);
        return;
    case WaitInit2:
        if (m_bIncoming) sendInit2();
        state = Logged;
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

    unsigned int size = readBuffer.size()-correction;
    if (version >= 7) readBuffer.incReadPos(1);

    unsigned long check;
    readBuffer.unpack(check);

    // main XOR key
    key = 0x67657268 * size + check;

    unsigned char *p = (unsigned char*)readBuffer.Data(readBuffer.readPos()-4);
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
        error(ErrorProtocol);
        return;
    }

    X1 = p[M1] ^ 0xFF;
    if(((B1 >> 16) & 0xFF) != X1){
        log(L_WARN, "Decrypt packet failed");
        error(ErrorProtocol);
        return;
    }

    X2 = ((B1 >> 8) & 0xFF);
    if(X2 < 220) {
        X3 = client_check_data[X2] ^ 0xFF;
        if((B1 & 0xFF) != X3){
            log(L_WARN, "Decrypt packet failed");
            error(ErrorProtocol);
            return;
        }
    }
    dumpPacket(readBuffer, 0, "Decrypted packed");

    readBuffer.setReadPos(2);
    if (version >= 7){
        char startByte;
        readBuffer.unpack(startByte);
        if (startByte != 0x02){
            log(L_WARN, "Bad start byte");
            error(ErrorProtocol);
        }
    }
    unsigned long checksum;
    readBuffer.unpack(checksum);
    unsigned short command;
    readBuffer.unpack(command);
    readBuffer.incReadPos(2);
    unsigned short seq;
    readBuffer.unpack(seq);
    readBuffer.incReadPos(12);

    unsigned short type, ackFlags, msgFlags;
    readBuffer.unpack(type);
    readBuffer.unpack(ackFlags);
    readBuffer.unpack(msgFlags);
    string msg_str;
    readBuffer >> msg_str;
    ICQMessage *m = client->parseMessage(type, u->Uin(), msg_str, readBuffer, 0, 0, seq, 0);

    switch (command){
    case TCP_START:
        if (m == NULL){
            error(ErrorProtocol);
            return;
        }
        m->Received = true;
        m->Direct = true;
        client->messageReceived(m);
        sendAck(seq);
        break;
    case TCP_CANCEL:
        remove();
        client->cancelMessage(m, false);
        break;
    case TCP_ACK:
        break;
    default:
        log(L_WARN, "Unknown TCP command %X", command);
        error(ErrorProtocol);
    }
}

void DirectClient::connected()
{
    if (m_bIncoming){
        u = client->getUser(uin);
        if ((u == NULL) || u->inIgnore()){
            log(L_WARN, "Connection from unknown user");
            error(ErrorProtocol);
            return;
        }
        if (m_nSessionId != u->DCcookie){
            log(L_WARN, "Bad session for user");
            error(ErrorProtocol);
            return;
        }
        if (u->direct) u->direct->remove();
        u->direct = this;
        if (version >= 7){
            state = WaitInit2;
        }else{
            state = Logged;
        }
    }else{
        if (version >= 7)
            sendInit2();
        state = WaitInit2;
    }
}

void DirectClient::sendInit2()
{
    unsigned pos = writeBuffer.writePos();
    writeBuffer.pack((unsigned short)0x0021);
    writeBuffer.pack((char) 0x03);
    writeBuffer.pack(0x0000000AL);
    writeBuffer.pack(0x00000001L);
    writeBuffer.pack(m_bIncoming ? 0x00000001L : 0x00000000L);
    writeBuffer.pack(0x00000000L);
    writeBuffer.pack(0x00000000L);
    if (m_bIncoming) {
        writeBuffer.pack(0x00040001L);
        writeBuffer.pack(0x00000000L);
        writeBuffer.pack(0x00000000L);
    } else {
        writeBuffer.pack(0x00000000L);
        writeBuffer.pack(0x00000000L);
        writeBuffer.pack(0x00040001L);
    }
    dumpPacket(writeBuffer, pos, "Direct write");
}

void DirectClient::sendAck(unsigned short seq)
{
    startPacket(TCP_ACK, seq);
    sendPacket();
}

void DirectClient::startPacket(unsigned short cmd, unsigned short seq)
{
    m_packetOffs = writeBuffer.writePos();
    writeBuffer
    << (unsigned short)0;	// size
    if (version >= 7)
        writeBuffer << (char)0x02;
    if (seq == 0)
        seq = --m_nSequence;
    writeBuffer
    << (unsigned long)0		// checkSum
    << (unsigned short)htons(cmd)
    << (char) 0x0E << (char) 0
    << (unsigned short)htons(seq)
    << (unsigned long)0
    << (unsigned long)0
    << (unsigned long)0;
}

void DirectClient::sendPacket()
{
    unsigned size = writeBuffer.size() - m_packetOffs - 2;
    unsigned char *p = (unsigned char*)writeBuffer.Data(m_packetOffs);
    *((unsigned short*)p) = size;

    dumpPacket(writeBuffer, m_packetOffs, "Direct send");

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
    dumpPacket(writeBuffer, m_packetOffs, "Encrypted packet");
}

void DirectClient::acceptMessage(ICQMessage *m)
{
    startPacket(TCP_ACK, m->timestamp1);
    client->packMessage(writeBuffer, m, NULL, 0, 0, 0, true);
    sendPacket();
}

void DirectClient::declineMessage(ICQMessage *m, const char *reason)
{
    startPacket(TCP_ACK, m->timestamp1);
    client->packMessage(writeBuffer, m, reason, 1, 0, 0, true);
    sendPacket();
}

// ____________________________________________________________________________________________

FileTransferListener::FileTransferListener(ICQFile *_file, ICQClient *_client)
        : ServerSocket()
{
    file = _file;
    client = _client;
}

void FileTransferListener::accept(int fd, const char *host, unsigned short port)
{
    log(L_DEBUG, "Accept ft connection %s:%u", host, port);
    file->listener = NULL;
    remove();
    file->ft = new FileTransfer(fd, host, port, client, file);
}

FileTransfer::FileTransfer(int fd, const char *host, unsigned short port, ICQClient *client, ICQFile *_file)
        : DirectSocket(fd, host, port, client)
{
    state = None;
    file = _file;
    m_nSpeed = 100;
}

FileTransfer::FileTransfer(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClient *client, ICQFile *_file)
        : DirectSocket(ip, real_ip, port, u, client)
{
    state = None;
    file = _file;
    m_nSpeed = 100;
}

bool FileTransfer::have_data()
{
    if ((state == Send) && (writeBuffer.readPos() >= writeBuffer.writePos())) return true;
    return DirectSocket::have_data();
}

void FileTransfer::write_ready()
{
    if ((state != Send) || (writeBuffer.readPos() < writeBuffer.writePos())){
        DirectSocket::write_ready();
        return;
    }
    if (file->state >= file->Size()){
        state = None;
        client->closeFile(file);
        file->ft = NULL;
        remove();
        list<ICQEvent*>::iterator it;
        for (it = client->processQueue.begin(); it != client->processQueue.end(); ++it){
            ICQEvent *e = *it;
            if (e->message() != file) continue;
            client->processQueue.remove(e);
            e->state = ICQEvent::Success;
            e->setType(EVENT_DONE);
            client->process_event(e);
            delete file;
            delete e;
            break;
        }
        return;
    }
    unsigned long tail = file->Size() - file->state;
    if (tail > 2048) tail = 2048;
    startPacket(FT_DATA);
    unsigned long pos = writeBuffer.writePos();
    if (!client->readFile(file, writeBuffer, tail)){
        log(L_WARN, "Error read file");
        error(ErrorProtocol);
        return;
    }
    file->state += (writeBuffer.writePos() - pos);
    sendPacket();
}

void FileTransfer::resume(int mode)
{
    if (state != Wait) return;
    if (!client->createFile(file, mode)){
        if (file->wait){
            state = Wait;
            return;
        }
        error(ErrorProtocol);
        return;
    }
    startPacket(FT_START);
    writeBuffer.pack((unsigned long)file->state);
    writeBuffer.pack((unsigned long)0);
    writeBuffer.pack(m_nSpeed);
    writeBuffer.pack((unsigned long)1);
    sendPacket();
    state = Receive;
}

void FileTransfer::processPacket()
{
    char cmd;
    readBuffer >> cmd;
    if (cmd == FT_SPEED){
        readBuffer.unpack(m_nSpeed);
        log(L_DEBUG, "Set spped %lu", m_nSpeed);
        return;
    }
    switch (state){
    case WaitInit:{
            if (cmd != FT_INIT){
                log(L_WARN, "No init command");
                error(ErrorProtocol);
                return;
            }
            state = InitReceive;
            startPacket(FT_SPEED);
            writeBuffer.pack(m_nSpeed);
            sendPacket();
            startPacket(FT_INIT_ACK);
            writeBuffer.pack(m_nSpeed);
            char b[12];
            snprintf(b, sizeof(b), "%lu", client->Uin());
            string uin = b;
            writeBuffer << uin;
            sendPacket();
            break;
        }
    case InitReceive:
        if (cmd != FT_FILEINFO){
            log(L_WARN, "Bad command in init receive");
            error(ErrorProtocol);
            return;
        }
        state = Wait;
        resume(FT_DEFAULT);
        break;
    case InitSend:
        switch (cmd){
        case FT_INIT_ACK:{
                startPacket(FT_FILEINFO);
                writeBuffer.pack((char)0);
                string empty;
                string s = file->shortName();
                client->toServer(s);
                writeBuffer << s << empty;
                writeBuffer.pack(file->Size());
                writeBuffer.pack((unsigned long)0);
                writeBuffer.pack(m_nSpeed);
                sendPacket();
                break;
            }
        case FT_START:{
                unsigned long pos;
                readBuffer.unpack(pos);
                log(L_DEBUG, "Start send at %lu", pos);
                file->state = pos;
                if (!client->openFile(file) || !client->seekFile(file, pos)){
                    log(L_WARN, "Can't open file");
                    error(ErrorProtocol);
                    return;
                }
                state = Send;
            }
            break;
        default:
            log(L_WARN, "Bad init client command %X", cmd);
            error(ErrorProtocol);
        }
        break;
    case Receive:{
            if (cmd != FT_DATA){
                log(L_WARN, "Bad data command");
                error(ErrorProtocol);
                return;
            }
            unsigned short size = readBuffer.size() - readBuffer.readPos();
            file->state += size;
            if (!client->writeFile(file, readBuffer)){
                log(L_WARN, "Error write file");
                error(ErrorProtocol);
                return;
            }
            if (file->state >= file->Size()){
                log(L_DEBUG, "File transfer OK");
                client->closeFile(file);
                file->ft = NULL;
                remove();
                list<ICQEvent*>::iterator it;
                for (it = client->processQueue.begin(); it != client->processQueue.end(); ++it){
                    ICQEvent *e = *it;
                    if (e->message() != file) continue;
                    client->processQueue.remove(e);
                    e->setType(EVENT_DONE);
                    e->state = ICQEvent::Success;
                    client->process_event(e);
                    delete e;
                    delete file;
                    break;
                }
                state = None;
            }
            break;
        }
    default:
        log(L_WARN, "Packet in bad state");
        error(ErrorProtocol);
        return;
    }
}

void FileTransfer::error_state()
{
    state = None;
    file->ft = NULL;
    remove();
    client->cancelMessage(file);
}

void FileTransfer::connected()
{
    log(L_DEBUG, "Connected");
    if (m_bIncoming){
        state = WaitInit;
    }else{
        state = InitSend;
        startPacket(FT_INIT);
        writeBuffer.pack((unsigned long)0);
        writeBuffer.pack((unsigned long)1);		// nFiles
        writeBuffer.pack(file->Size());			// Total size
        writeBuffer.pack(m_nSpeed);				// speed
        char b[12];
        snprintf(b, sizeof(b), "%lu", client->Uin());
        string uin = b;
        writeBuffer << uin;
        sendPacket();
    }
}

void FileTransfer::startPacket(char cmd)
{
    m_packetOffs = writeBuffer.writePos();
    writeBuffer << (unsigned short)0;
    writeBuffer << cmd;
}

void FileTransfer::sendPacket()
{
    unsigned size = writeBuffer.size() - m_packetOffs - 2;
    unsigned char *p = (unsigned char*)writeBuffer.Data(m_packetOffs);
    *((unsigned short*)p) = size;
    dumpPacket(writeBuffer, m_packetOffs, "File transfer send");
}

// ___________________________________________________________________________________________

ChatListener::ChatListener(ICQChat *_chat, ICQClient *_client)
        : ServerSocket()
{
    chat = _chat;
    client = _client;
}

void ChatListener::accept(int fd, const char *host, unsigned short port)
{
    log(L_DEBUG, "Accept chat connection %s:%u", host, port);
    chat->listener = NULL;
    remove();
    chat->chat = new ChatSocket(fd, host, port, client, chat);
}

ChatSocket::ChatSocket(int fd, const char *host, unsigned short port, ICQClient *client, ICQChat *_chat)
        : DirectSocket(fd, host, port, client)
{
    chat = _chat;
    init();
}

ChatSocket::ChatSocket(unsigned long ip, unsigned long real_ip, unsigned short port, ICQUser *u, ICQClient *client, ICQChat *_chat)
        : DirectSocket(ip, real_ip, port, u, client)
{
    chat = _chat;
    init();
}

void ChatSocket::init()
{
    state = None;
    fontSize = 12;
    fontFace = 0;
    fontFamily = "MS Sans Serif";
    myFontFace = 0;
    curMyFontFace = 0;
    bgColor = 0xFFFFFF;
    fgColor = 0;
}

void ChatSocket::sendLine(const char *str)
{
    unsigned long writePos = writeBuffer.writePos();
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
                writeBuffer.pack(CHAT_ESCAPE);
                writeBuffer.pack(CHAT_FONT_FACE);
                writeBuffer.pack((unsigned long)4);
                writeBuffer.pack(myFontFace);
            }
            string s1 = client->clearHTML(s.c_str());
            client->toServer(s1);
            writeBuffer.pack(s1.c_str(), s1.size());
        }
        if (tag){
            tag++;
            end = strchr(tag, '>');
            if (end){
                string t;
                t.assign(tag, end - tag);
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
            }
        }
        str = end;
    }
    writeBuffer.pack(' ');
    writeBuffer.pack(CHAT_NEWLINE);
    dumpPacket(writeBuffer, writePos, "Chat send");
}

void ChatSocket::putText(string &s)
{
    if (s.size() == 0) return;
    client->fromServer(s);
    ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_TEXT, chat);
    e.text = s;
    client->process_event(&e);
    s = "";
}

void ChatSocket::read_ready()
{
    if (state != Connected){
        DirectSocket::read_ready();
        return;
    }
    char b[2048];
    int read = recv(m_fd, b, sizeof(b), 0);
    if (read == 0){
        error(ErrorDisconnect);
        return;
    }
    if (read < 0){
        if (errno != EAGAIN)
            error(ErrorRead);
        return;
    }
    readBuffer.pack(b, read);
    dumpPacket(readBuffer, readBuffer.readPos(), "Chat read");

    string chatText;
    for (;;){
        if (readBuffer.readPos() == readBuffer.size()) break;
        char c;
        readBuffer >> c;
        switch (c){
        case CHAT_ESCAPE:{
                putText(chatText);
                if (readBuffer.readPos() + 5 > readBuffer.size()){
                    readBuffer.incReadPos(-1);
                    return;
                }
                readBuffer >> c;
                unsigned long size;
                readBuffer.unpack(size);
                if (readBuffer.readPos() + size > readBuffer.size()){
                    readBuffer.incReadPos(-6);
                    return;
                }
                Buffer b;
                if (size){
                    b.init(size);
                    readBuffer.unpack(b.Data(0), size);
                    b.setWritePos(size);
                }
                switch (c){
                case CHAT_FONT_FACE:
                    b.unpack(fontFace);
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
    readBuffer.init(0);
    putText(chatText);
}

void ChatSocket::processPacket()
{
    switch (state){
    case WaitInit:{
            readBuffer.incReadPos(8);
            unsigned long uin;
            readBuffer.unpack(uin);
            startPacket();
            writeBuffer.pack((unsigned long)0x65);
            writeBuffer.pack(client->Uin());
            string alias = client->name();
            writeBuffer << alias;
            writeBuffer.pack(fgColor);
            writeBuffer.pack(bgColor);
            writeBuffer.pack((unsigned long)version);
            writeBuffer.pack((unsigned long)(client->Port()));
            writeBuffer.pack(client->RealIP());
            writeBuffer.pack(client->IP());
            writeBuffer.pack((char)0x01);
            unsigned short session = rand();
            writeBuffer.pack(session);
            writeBuffer.pack(fontSize);
            writeBuffer.pack(myFontFace);
            writeBuffer << fontFamily;
            unsigned short w = 0;
            writeBuffer .pack(w);
            char nClients = 0;
            writeBuffer.pack(nClients);
            sendPacket();
            state = WaitFont;
            break;
        }
    case WaitFont:
        {
            readBuffer.incReadPos(19);
            readBuffer.unpack(fontSize);
            readBuffer.unpack(fontFace);
            readBuffer.unpack(fontFamily);
            state = Connected;
            ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_CONNECT, chat);
            client->process_event(&e);
            break;
        }
    case WaitFontInfo:{
            readBuffer.incReadPos(4);
            unsigned long uin;
            string alias;
            unsigned long fgColor, bgColor;
            readBuffer.unpack(uin);
            readBuffer.unpack(alias);
            readBuffer.unpack(fgColor);
            readBuffer.unpack(bgColor);
            log(L_DEBUG, "Info %lu %s %lX %lX", uin, alias.c_str(), fgColor, bgColor);
            unsigned long version, port, ip, real_ip;
            char mode;
            unsigned short session;
            readBuffer.unpack(version);
            readBuffer.unpack(port);
            readBuffer.unpack(real_ip);
            readBuffer.unpack(ip);
            readBuffer.unpack(mode);
            readBuffer.unpack(session);
            struct in_addr addr;
            addr.s_addr = ip;
            string ip_str = inet_ntoa(addr);
            addr.s_addr = real_ip;
            string real_ip_str = inet_ntoa(addr);
            log(L_DEBUG, "Inof %lu %lu %s %s %u", version, port,
                ip_str.c_str(), real_ip_str.c_str(),
                mode);
            readBuffer.unpack(fontSize);
            readBuffer.unpack(fontFace);
            readBuffer.unpack(fontFamily);
            log(L_DEBUG, "Font info: %lu %lu %s", fontSize, fontFace, fontFamily.c_str());
            readBuffer.incReadPos(2);
            char nClients;
            readBuffer.unpack(nClients);
            log(L_DEBUG, "Clients: %u", nClients);
            for (; nClients > 0; nClients--){
                unsigned long version, port, uin, ip, real_ip, handshake;
                unsigned short session;
                char mode;
                readBuffer.unpack(version);
                readBuffer.unpack(port);
                readBuffer.unpack(uin);
                readBuffer.unpack(ip);
                readBuffer.unpack(real_ip);
                readBuffer.incReadPos(2);
                readBuffer.unpack(mode);
                readBuffer.unpack(session);
                readBuffer.unpack(handshake);
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
            writeBuffer.pack((unsigned long)version);
            writeBuffer.pack((unsigned long)(client->Port()));
            writeBuffer.pack(client->RealIP());
            writeBuffer.pack(client->IP());
            writeBuffer.pack((char)0x01);
            writeBuffer.pack(session);
            writeBuffer.pack(fontSize);
            writeBuffer.pack(myFontFace);
            writeBuffer << fontFamily;
            writeBuffer.pack((unsigned short)1);
            sendPacket();
            state = Connected;
            ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_CONNECT, chat);
            client->process_event(&e);
            break;
        }
    default:
        log(L_WARN, "Chat packet in unknown state");
        error(ErrorProtocol);
        return;
    }
}

void ChatSocket::connected()
{
    if (m_bIncoming){
        state = WaitInit;
    }else{
        startPacket();
        writeBuffer.pack((unsigned long)0x64);
        writeBuffer.pack((unsigned long)(-7));
        writeBuffer.pack(client->Uin());
        char b[12];
        snprintf(b, sizeof(b), "%lu", client->Uin());
        string uin = b;
        writeBuffer << uin;
        writeBuffer.pack(client->listener->port());
        writeBuffer.pack(fgColor);
        writeBuffer.pack(bgColor);
        writeBuffer.pack((char)0);
        sendPacket();
        state = WaitFontInfo;
    }
}

void ChatSocket::startPacket()
{
    m_packetOffs = writeBuffer.writePos();
    writeBuffer << (unsigned short)0;
}

void ChatSocket::sendPacket()
{
    unsigned size = writeBuffer.size() - m_packetOffs - 2;
    unsigned char *p = (unsigned char*)writeBuffer.Data(m_packetOffs);
    *((unsigned short*)p) = size;
    dumpPacket(writeBuffer, m_packetOffs, "Chat send");
}

void ChatSocket::error_state()
{
    DirectSocket::error_state();
    ICQEvent e(EVENT_CHAT, chat->getUin(), CHAT_CONNECT, chat);
    e.state = ICQEvent::Fail;
    client->process_event(&e);
}



