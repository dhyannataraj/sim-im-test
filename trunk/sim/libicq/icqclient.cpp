/***************************************************************************
                          icqclient.cpp -  description
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
#include "icqprivate.h"
#include "proxy.h"
#include "icqhttp.h"
#include "log.h"

#include <stdio.h>
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#define RECONNECT_TIMEOUT	5
#define PING_TIMEOUT		60

ICQClient::ICQClient(SocketFactory *factory)
        : contacts(this)
{
    ServerHost = "login.icq.com";
    ServerPort = 5190;
    WebAware = false;
    Authorize = false;
    HideIp = false;
    RejectMessage = false;
    RejectURL = false;
    RejectWeb = false;
    RejectEmail = false;
    RejectOther = false;
    DirectMode = 0;
    BypassAuth = false;
    owner = NULL;
    p = new ICQClientPrivate(this, factory);
}

void ICQClient::init()
{
    unsigned long now;
    time((time_t*)&now);
    owner = createUser();
    owner->PhoneBookTime = now;
    owner->PhoneStatusTime = now;
    owner->DCcookie = rand();
    owner->bMyInfo = true;
}

ICQUser *ICQClient::createUser()
{
    return new ICQUser;
}

ICQGroup *ICQClient::createGroup()
{
    return new ICQGroup;
}

ICQClientPrivate::ICQClientPrivate(ICQClient *_client, SocketFactory *_factory)
{
    client = _client;
    factory = _factory;
    m_bHeader = true;
    m_bBirthday = false;
    sock = NULL;
    listener = NULL;
    m_nProcessId = MSG_PROCESS_ID;
    unsigned long now;
    time((time_t*)&now);
    advCounter = 0;
    m_lastTime = now;
    m_reconnectTime = 0;
    lastInfoRequestTime = 0;
    lastPhoneRequestTime = 0;
    lastResponseRequestTime = 0;
    needPhonebookUpdate = false;
    needPhoneStatusUpdate = false;
    needShareUpdate = false;
    m_bRosters = false;
    check_time = now;
    checkBirthDay();
}

ICQClient::~ICQClient()
{
    close();
    delete p;
    if (owner) delete owner;
}

ICQClientPrivate::~ICQClientPrivate()
{
}

void ICQClientPrivate::checkBirthDay()
{
    bool oldValue = m_bBirthday;
    if (!client->owner) return;
    int year = client->owner->BirthYear;
    int month = client->owner->BirthMonth;
    int day = client->owner->BirthDay;
    m_bBirthday = false;
    if (day && month && year){
        time_t now;
        time(&now);
        struct tm *tm = localtime(&now);
        if (((tm->tm_mon + 1) == month) && ((tm->tm_mday + 1) == day))
            m_bBirthday = true;
    }
    if ((m_bBirthday != oldValue) && (m_state == Logged))
        sendStatus(fullStatus(client->owner->uStatus));
}

void ICQClient::storePassword(const char *p)
{
    EncryptedPassword = "";
    unsigned char xor_table[] = {
        0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
        0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c};
    int j;
    for (j = 0; j < 16; j++){
        if (p[j] == 0) break;
        char c = (p[j] ^ xor_table[j]);
        if (c == 0){
            EncryptedPassword += "\\";
            c = '0';
        }else if (c == '\\'){
            EncryptedPassword += "\\";
        }
        EncryptedPassword += c;
    }
    DecryptedPassword = "";
}

void ICQClient::process_event(ICQEvent *e)
{
    log(L_DEBUG,"Process event %u", e->type());
}

void ICQClientPrivate::create_socket()
{
    sock = new ClientSocket(this, factory);
    sock->setProxy(factory->getProxy());
    listener = new ICQListener(this);
    if (!listener->created() || !sock->created()){
        close();
        return;
    }
    client->owner->Port = listener->port();
}

void ICQClient::requestSecureChannel(ICQUser *u)
{
    u->requestSecureChannel(p);
}

void ICQClient::closeSecureChannel(ICQUser *u)
{
    u->closeSecureChannel(p);
}

bool ICQClient::isLogged()
{
    return (p->m_state == ICQClientPrivate::Logged);
}

bool ICQClient::isConnecting()
{
    return (p->m_state != ICQClientPrivate::Logged) && (p->m_state != ICQClientPrivate::Logoff);
}

void ICQClient::idle()
{
    p->idle();
}

SocketFactory *ICQClient::factory()
{
    return p->factory;
}

void ICQClient::close()
{
    p->close();
}

void ICQClientPrivate::close()
{
    if (listener){
        delete listener;
        listener = NULL;
    }
    if (sock){
        if (m_state == Logged){
            flap(ICQ_CHNxCLOSE);
            sendPacket();
        }
        delete sock;
        sock = NULL;
    }
}

void ICQClientPrivate::packet_ready()
{
    time(&m_lastTime);
    if (m_bHeader){
        char c;
        sock->readBuffer >> c;
        if (c != 0x2A){
            log(L_ERROR, "Server send bad packet start code: %02X", c);
            sock->error_state(ErrorProtocol);
            return;
        }
        sock->readBuffer >> m_nChannel;
        unsigned short sequence, size;
        sock->readBuffer >> sequence >> size;
        if (size){
            log(L_DEBUG, "? packet size=%u", size);
            sock->readBuffer.add(size);
            m_bHeader = false;
            return;
        }
    }
    dumpPacket(sock->readBuffer, 0,"Read");
    switch (m_nChannel){
    case ICQ_CHNxNEW:
        chn_login();
        break;
    case ICQ_CHNxCLOSE:
        chn_close();
        break;
    case ICQ_CHNxDATA:{
            unsigned short fam, type;
            unsigned short flags, seq, cmd;
            sock->readBuffer >> fam >> type >> flags >> seq >> cmd;
            switch (fam){
            case ICQ_SNACxFAM_SERVICE:
                snac_service(type, seq);
                break;
            case ICQ_SNACxFAM_LOCATION:
                snac_location(type, seq);
                break;
            case ICQ_SNACxFAM_BUDDY:
                snac_buddy(type, seq);
                break;
            case ICQ_SNACxFAM_MESSAGE:
                snac_message(type, seq);
                break;
            case ICQ_SNACxFAM_BOS:
                snac_bos(type, seq);
                break;
            case ICQ_SNACxFAM_PING:
                snac_ping(type, seq);
                break;
            case ICQ_SNACxFAM_LISTS:
                snac_lists(type, seq);
                break;
            case ICQ_SNACxFAM_VARIOUS:
                snac_various(type, seq);
                break;
            case ICQ_SNACxFAM_LOGIN:
                snac_login(type, seq);
                break;
            default:
                log(L_WARN, "Unknown family %02X", fam);
            }
            break;
        }
    default:
        log(L_ERROR, "Unknown channel %u", m_nChannel & 0xFF);
    }
    processInfoRequestQueue();
    processPhoneRequestQueue(0);
    processResponseRequestQueue(0);
    if (sock) sock->readBuffer.init(6);
    m_bHeader = true;
}

bool ICQClientPrivate::error_state(SocketError err)
{
    log(L_DEBUG, "Error: %u", err);
    switch (err){
    case ErrorProxyConnect:{
            m_reconnectTime = (time_t)(-1);
            m_state = ErrorState;
            ICQEvent e(EVENT_PROXY_ERROR);
            client->process_event(&e);
            break;
        }
    case ErrorProxyAuth:{
            m_reconnectTime = (time_t)(-1);
            m_state = ErrorState;
            ICQEvent e(EVENT_PROXY_BAD_AUTH);
            client->process_event(&e);
            break;
        }
    default:
        if (m_reconnectTime == (time_t)(-1)){
            m_reconnectTime = 0;
        }else{
            m_state = ForceReconnect;
        }
    }
    setStatus(ICQ_STATUS_OFFLINE);
    return false;
}

void ICQClient::setStatus(unsigned short status)
{
    p->setStatus(status);
}

void ICQClientPrivate::setStatus(unsigned short status)
{
    if (status == ICQ_STATUS_OFFLINE){
        switch (m_state){
        case Reconnect:{
                m_reconnectTime = 0;
                m_state = Logoff;
                ICQEvent e(EVENT_STATUS_CHANGED, client->owner->Uin);
                client->process_event(&e);
                break;
            }
        case Logoff:
            break;
        default:
            close();
            client->owner->uStatus = ICQ_STATUS_OFFLINE;
            if (m_state == ForceReconnect){
                m_state = Reconnect;
                time_t reconnect;
                time(&reconnect);
                reconnect += RECONNECT_TIMEOUT;
                if (m_reconnectTime < reconnect) m_reconnectTime = reconnect;
            }else{
                m_state = Logoff;
            }
            ICQEvent e(EVENT_STATUS_CHANGED, client->owner->Uin);
            client->process_event(&e);
            time_t now;
            time(&now);
            list<ICQUser*>::iterator it;
            for (it = client->contacts.users.begin(); it != client->contacts.users.end(); it++){
                if ((*it)->uStatus == ICQ_STATUS_OFFLINE) continue;
                (*it)->setOffline();
                ICQEvent e(EVENT_STATUS_CHANGED, (*it)->Uin);
                client->process_event(&e);
            }
        }
        return;
    }
    if (sock == NULL){
        cookie.init(0);
        create_socket();
        if (sock == NULL) return;
        m_bRosters = false;
        m_nSequence = rand() & 0x7FFFF;
        m_nMsgSequence = 1;
        m_reconnectTime = 0;
        m_state = client->owner->Uin ? Connect : Register;
        ICQEvent e(EVENT_STATUS_CHANGED, client->owner->Uin);
        client->process_event(&e);
        sock->connect(client->ServerHost.c_str(), client->ServerPort);
    }
    if (m_state != Logged){
        m_nLogonStatus = status;
        return;
    }
    m_nLogonStatus = status;
    sendStatus(status);
}

void ICQClientPrivate::flap(char channel)
{
    m_nSequence++;
    m_nPacketStart = sock->writeBuffer.writePos();
    sock->writeBuffer
    << (char)0x2A
    << channel
    << m_nSequence
    << 0;
}

void ICQClientPrivate::snac(unsigned short fam, unsigned short type, bool msgId)
{
    flap(ICQ_CHNxDATA);
    sock->writeBuffer
    << fam
    << type
    << 0x0000
    << (msgId ? m_nMsgSequence++ : 0x0000)
    << type;
}

void ICQClientPrivate::sendPacket()
{
    char *packet = sock->writeBuffer.Data(m_nPacketStart);
    *((unsigned short*)(packet + 4)) = htons(sock->writeBuffer.size() - m_nPacketStart - 6);
    dumpPacket(sock->writeBuffer, m_nPacketStart, "Write");
    time(&m_lastTime);
    sock->write();
}

void ICQClientPrivate::dropPacket()
{
    sock->writeBuffer.setWritePos(m_nPacketStart);
}

#define CHECK_INTERVAL	300

void ICQClientPrivate::idle()
{
    factory->idle();
    time_t now;
    time(&now);
    if ((unsigned)now >= check_time + CHECK_INTERVAL){
        checkBirthDay();
        check_time = now;
    }
    if (m_state == Logged){
        processInfoRequestQueue();
        processPhoneRequestQueue(0);
        processResponseRequestQueue(0);
        if (now <= m_lastTime + PING_TIMEOUT) return;
        flap(ICQ_CHNxPING);
        sendPacket();
        return;
    }
    if (m_state == Reconnect){
        if (now <= m_reconnectTime) return;
        setStatus(m_nLogonStatus);
    }
}

unsigned long ICQClientPrivate::fullStatus(unsigned long s)
{
    if (s & ICQ_STATUS_NA) s |= ICQ_STATUS_AWAY;
    if (s & ICQ_STATUS_OCCUPIED) s |= ICQ_STATUS_AWAY;
    if (s & ICQ_STATUS_DND) s |= (ICQ_STATUS_AWAY | ICQ_STATUS_OCCUPIED);
    s &= 0x0000FFFF;
    if (client->WebAware)
        s |= ICQ_STATUS_FxWEBxPRESENCE;
    if (client->HideIp)
        s |= ICQ_STATUS_FxHIDExIP;
    if (m_bBirthday)
        s |= ICQ_STATUS_FxBIRTHDAY;
    if (client->owner->inInvisible)
        s |= ICQ_STATUS_FxPRIVATE;
    switch (client->DirectMode){
    case 1:
        s |= ICQ_STATUS_FxDIRECTxLISTED;
        break;
    case 2:
        s |= ICQ_STATUS_FxDIRECTxAUTH;
        break;
    }
    return s;
}

void ICQClientPrivate::connect_ready()
{
    if (!factory->isHttpProxy())
        sock->setProxyConnected();
    sock->readBuffer.init(6);
    m_bHeader = true;
}

bool ICQClientPrivate::addRequest(unsigned long uin, bool bPriority, list<info_request> &queue)
{
    if (uin >= UIN_SPECIAL) return false;
    for (list<info_request>::iterator it = queue.begin(); it != queue.end(); it++){
        if (it->uin != uin) continue;
        if (!bPriority) return false;
        queue.remove(*it);
        break;
    }
    info_request ri;
    ri.uin = uin;
    ri.bAuto = !bPriority;
    if (bPriority){
        queue.push_front(ri);
    }else{
        queue.push_back(ri);
    }
    return true;
}

void ICQClient::addInfoRequest(unsigned long uin, bool bPriority)
{
    p->addRequest(uin, bPriority, p->infoRequestQueue);
}

void ICQClient::addPhoneRequest(unsigned long uin, bool bPriority)
{
    p->addRequest(uin, bPriority, p->phoneRequestQueue);
}

void ICQClient::addResponseRequest(unsigned long uin, bool bPriority)
{
    if (uin >= UIN_SPECIAL) return;
    ICQUser *u = getUser(uin);
    if (u == NULL) return;
    if (u->uStatus == ICQ_STATUS_OFFLINE) return;
    if ((u->Version <= 6) || u->hasCap(CAP_TRILLIAN) || (u->direct && u->direct->isLogged())){
        if (!bPriority){
            if (owner->inInvisible && !u->inVisible) return;
            if (!owner->inInvisible && u->inInvisible) return;
        }
        ICQMessage *msg = new ICQAutoResponse;
        msg->setType(ICQ_READxAWAYxMSG);
        if (u->uStatus & ICQ_STATUS_DND){
            msg->setType(ICQ_READxDNDxMSG);
        }else if (u->uStatus & ICQ_STATUS_OCCUPIED){
            msg->setType(ICQ_READxOCCUPIEDxMSG);
        }else if (u->uStatus & ICQ_STATUS_NA){
            msg->setType(ICQ_READxNAxMSG);
        }else if (u->uStatus & ICQ_STATUS_FREEFORCHAT){
            msg->setType(ICQ_READxFFCxMSG);
        }
        u->addMessage(msg, p);
        return;
    }
    p->addRequest(uin, bPriority, p->responseRequestQueue);
}

void ICQClientPrivate::processInfoRequestQueue()
{
    if (m_state != Logged) return;
    if (infoRequestQueue.size() == 0) return;
    time_t now;
    time(&now);
    if (((unsigned long)now < lastInfoRequestTime + 10)) return;
    for (;;){
        if (infoRequestQueue.size() == 0) return;
        info_request ri = infoRequestQueue.front();
        infoRequestQueue.remove(ri);
        lastInfoRequestTime = now;
        if (requestInfo(ri.uin, ri.bAuto)) return;
    }
}

void ICQClientPrivate::processPhoneRequestQueue(unsigned short seq)
{
    if (m_state != Logged) return;
    if (phoneRequestQueue.size() == 0) return;
    time_t now;
    time(&now);
    if ((seq != phoneRequestSeq) && (((unsigned long)now < lastPhoneRequestTime + 120))) return;
    for (;;){
        if (phoneRequestQueue.size() == 0) return;
        info_request ri = phoneRequestQueue.front();
        phoneRequestQueue.remove(ri);
        lastPhoneRequestTime = now;
        if (requestPhoneBook(ri.uin, ri.bAuto)) return;
    }
}

void ICQClientPrivate::processResponseRequestQueue(unsigned short seq)
{
    if (m_state != Logged) return;
    if (responseRequestQueue.size() == 0) return;
    time_t now;
    time(&now);
    if ((seq != responseRequestSeq) && (((unsigned long)now < lastResponseRequestTime + 20))) return;
    for (;;){
        if (responseRequestQueue.size() == 0) return;
        info_request ri = responseRequestQueue.front();
        responseRequestQueue.remove(ri);
        lastResponseRequestTime = now;
        if (requestAutoResponse(ri.uin, ri.bAuto)) return;
    }
}

unsigned long ICQClient::getFileSize(const char*, int *nSrcFiles, vector<fileName>&)
{
    *nSrcFiles = 0;
    return 0;
}

bool ICQClient::createFile(ICQFile*, int)
{
    return false;
}

bool ICQClient::openFile(ICQFile*)
{
    return false;
}

bool ICQClient::seekFile(ICQFile*, unsigned long)
{
    return false;
}

bool ICQClient::readFile(ICQFile*, Buffer&, unsigned short)
{
    return false;
}

bool ICQClient::writeFile(ICQFile*, Buffer&)
{
    return false;
}

void ICQClient::closeFile(ICQFile*)
{
}

ICQMessage *ICQClient::getProcessMessage(unsigned long offs)
{
    list<ICQEvent*>::iterator it;
    for (it = p->processQueue.begin(); it != p->processQueue.end(); ++it){
        if ((*it)->type() != EVENT_MESSAGE_RECEIVED) continue;
        if ((*it)->message() && ((*it)->message()->Id == offs))
            return (*it)->message();
    }
    return NULL;
}
