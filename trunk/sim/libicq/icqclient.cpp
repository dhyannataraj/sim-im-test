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

#include "defs.h"
#include "icqclient.h"
#include "log.h"

#include <stdio.h>
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#define RECONNECT_TIMEOUT	5
#define PING_TIMEOUT		60

ICQClient::ICQClient()
        : ServerHost(this, "ServerHost", "login.icq.com"),
        ServerPort(this, "ServerPort", 5190),
        DecryptedPassword(this, "Password"),
        EncryptedPassword(this, "EncryptPassword"),
        WebAware(this, "WebAware"),
        Authorize(this, "Authorize"),
        HideIp(this, "HideIp"),
        RejectMessage(this, "RejectMessage"),
        RejectURL(this, "RejectURL"),
        RejectWeb(this, "RejectWeb"),
        RejectEmail(this, "RejectEmail"),
        RejectOther(this, "RejectOther"),
        RejectFilter(this, "RejectFilter"),
        DirectMode(this, "DirectMode"),
        BirthdayReminder(this, "BirthdayReminder", "birthday.wav"),
        FileDone(this, "FileDone", "filedone.wav"),
        BypassAuth(this, "BypassAuth"),
        contacts(this),
        m_bHeader(true), m_bBirthday(false)
{
    sock = NULL;
    listener = NULL;
    m_nProcessId = MSG_PROCESS_ID;
    advCounter = 0;
    time(&m_lastTime);
    m_reconnectTime = 0;
    lastInfoRequestTime = 0;
    lastPhoneRequestTime = 0;
    lastResponseRequestTime = 0;
    needPhonebookUpdate = false;
    needPhoneStatusUpdate = false;
    unsigned long now;
    time((time_t*)&now);
    PhoneBookTime = now;
    PhoneStatusTime = now;
    DCcookie = rand();
    bMyInfo = true;
    m_bRosters = false;
}

ICQClient::~ICQClient()
{
    close();
}

void ICQClient::storePassword(const char *p)
{
    char pswd[16];
    unsigned char xor_table[] = {
        0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
        0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c};
    int j;
    for (j = 0; j < 16; j++){
        if (p[j] == 0) break;
        pswd[j] = (p[j] ^ xor_table[j]);
    }
    pswd[j] = 0;
    EncryptedPassword = pswd;
    DecryptedPassword = "";
}

void ICQClient::process_event(ICQEvent *e)
{
    log(L_DEBUG,"Process event %u", e->type());
}

void ICQClient::create_socket()
{
    sock = new ClientSocket(this, this);
    listener = new ICQListener(this);
    if (!listener->created() || !sock->created()){
        close();
        return;
    }
    Port = listener->port();
}

void ICQClient::close()
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
        sock->remove();
        sock = NULL;
    }
}

void ICQClient::packet_ready()
{
    time(&m_lastTime);
    if (m_bHeader){
        char c;
        sock->readBuffer >> c;
        if (c != 0x2A){
            log(L_ERROR, "Server send bad packet start code: %02X", c);
            sock->error();
            return;
        }
        sock->readBuffer >> m_nChannel;
        unsigned short sequence, size;
        sock->readBuffer >> sequence >> size;
        if (size){
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
    sock->readBuffer.init(6);
    m_bHeader = true;
}

void ICQClient::error_state(SocketError err)
{
    switch (err){
    case ErrorProxyConnect:{
            m_reconnectTime = (time_t)(-1);
            m_state = ErrorState;
            ICQEvent e(EVENT_PROXY_ERROR);
            process_event(&e);
            break;
        }
    case ErrorProxyAuth:{
            m_reconnectTime = (time_t)(-1);
            m_state = ErrorState;
            ICQEvent e(EVENT_PROXY_BAD_AUTH);
            process_event(&e);
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
}

void ICQClient::setStatus(unsigned short status)
{
    if (status == ICQ_STATUS_OFFLINE){
        switch (m_state){
        case Reconnect:{
                m_reconnectTime = 0;
                m_state = Logoff;
                ICQEvent e(EVENT_STATUS_CHANGED, Uin);
                process_event(&e);
                break;
            }
        case Logoff:
            break;
        default:
            close();
            uStatus = ICQ_STATUS_OFFLINE;
            if (m_state == ForceReconnect){
                m_state = Reconnect;
                time_t reconnect;
                time(&reconnect);
                reconnect += RECONNECT_TIMEOUT;
                if (m_reconnectTime < reconnect) m_reconnectTime = reconnect;
            }else{
                m_state = Logoff;
            }
            ICQEvent e(EVENT_STATUS_CHANGED, Uin);
            process_event(&e);
            time_t now;
            time(&now);
            list<ICQUser*>::iterator it;
            for (it = contacts.users.begin(); it != contacts.users.end(); it++){
                if ((*it)->uStatus == ICQ_STATUS_OFFLINE) continue;
                (*it)->setOffline();
                ICQEvent e(EVENT_STATUS_CHANGED, (*it)->Uin);
                process_event(&e);
            }
        }
        return;
    }
    if (sock == NULL){
        create_socket();
        if (sock == NULL) return;
        m_bRosters = false;
        m_nSequence = rand() & 0x7FFFF;
        m_nMsgSequence = 1;
        m_reconnectTime = 0;
        m_state = Uin ? Connect : Register;
        ICQEvent e(EVENT_STATUS_CHANGED, Uin);
        process_event(&e);
        sock->connect(ServerHost.c_str(), ServerPort());
    }
    if (m_state != Logged){
        m_nLogonStatus = status;
        return;
    }
    sendStatus(status);
}

void ICQClient::flap(char channel)
{
    m_nSequence++;
    m_nPacketStart = sock->writeBuffer.writePos();
    sock->writeBuffer
    << (char)0x2A
    << channel
    << m_nSequence
    << 0;
}

void ICQClient::snac(unsigned short fam, unsigned short type, bool msgId)
{
    flap(ICQ_CHNxDATA);
    sock->writeBuffer
    << fam
    << type
    << 0x0000
    << (msgId ? m_nMsgSequence++ : 0x0000)
    << type;
}

void ICQClient::sendPacket()
{
    char *packet = sock->writeBuffer.Data(m_nPacketStart);
    *((unsigned short*)(packet + 4)) = htons(sock->writeBuffer.size() - m_nPacketStart - 6);
    dumpPacket(sock->writeBuffer, m_nPacketStart, "Write");
    time(&m_lastTime);
    sock->write();
}

void ICQClient::dropPacket()
{
    sock->writeBuffer.setWritePos(m_nPacketStart);
}

void ICQClient::idle()
{
    SocketFactory::idle();
    for (list<DirectSocket*>::iterator it = removedSockets.begin(); it != removedSockets.end(); ++it){
        delete (*it);
    }
    removedSockets.clear();
    time_t now;
    time(&now);
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

unsigned long ICQClient::fullStatus(unsigned long s)
{
    if (s & ICQ_STATUS_NA) s |= ICQ_STATUS_AWAY;
    if (s & ICQ_STATUS_OCCUPIED) s |= ICQ_STATUS_AWAY;
    if (s & ICQ_STATUS_DND) s |= (ICQ_STATUS_AWAY | ICQ_STATUS_OCCUPIED);
    s &= 0x0000FFFF;
    if (WebAware())
        s |= ICQ_STATUS_FxWEBxPRESENCE;
    if (HideIp())
        s |= ICQ_STATUS_FxHIDExIP;
    if (m_bBirthday)
        s |= ICQ_STATUS_FxBIRTHDAY;
    if (inInvisible())
        s |= ICQ_STATUS_FxPRIVATE;
    switch (DirectMode()){
    case 1:
        s |= ICQ_STATUS_FxDIRECTxLISTED;
        break;
    case 2:
        s |= ICQ_STATUS_FxDIRECTxAUTH;
        break;
    }
    return s;
}

void ICQClient::getAutoResponse(unsigned long uin, string &res)
{
    res = "";
    ICQUser *u = getUser(uin);
    unsigned long status = 0;
    if (u) status = u->uStatus;
    if (status & ICQ_STATUS_DND){
        if (u)
            res = u->AutoResponseDND;
        if (*res.c_str() == 0)
            res = AutoResponseDND;
    }else if (status & ICQ_STATUS_OCCUPIED){
        if (u)
            res = u->AutoResponseOccupied;
        if (*res.c_str() == 0)
            res = AutoResponseOccupied;
    }else if (status & ICQ_STATUS_NA){
        if (u)
            res = u->AutoResponseNA;
        if (*res.c_str() == 0)
            res = AutoResponseNA;
    }else if (status & ICQ_STATUS_FREEFORCHAT){
        if (u)
            res = u->AutoResponseFFC;
        if (*res.c_str() == 0)
            res = AutoResponseFFC;
    }else{
        if (u)
            res = u->AutoResponseAway;
        if (*res.c_str() == 0)
            res = AutoResponseAway;
    }
}

void ICQClient::save(ostream &s)
{
    ConfigArray::save(s);
    s << "[ContactList]\n";
    contacts.save(s);
    for (vector<ICQGroup*>::iterator it_grp = contacts.groups.begin(); it_grp != contacts.groups.end(); it_grp++){
        s << "[Group]\n";
        (*it_grp)->save(s);
    }
    for (list<ICQUser*>::iterator it = contacts.users.begin(); it != contacts.users.end(); it++){
        if ((*it)->bIsTemp) continue;
        s << "[User]\n";
        (*it)->save(s);
    }
}

bool ICQClient::load(istream &s, string &nextPart)
{
    ICQUser::load(s, nextPart);
    for (;;){
        if (!strcmp(nextPart.c_str(), "[ContactList]")){
            contacts.load(s, nextPart);
            continue;
        }
        if (!strcmp(nextPart.c_str(), "[Group]")){
            ICQGroup *grp = new ICQGroup;
            if (grp->load(s, nextPart)){
                contacts.groups.push_back(grp);
            }else{
                delete grp;
            }
            continue;
        }
        if (!strcmp(nextPart.c_str(), "[User]")){
            ICQUser *u = new ICQUser;
            if (u->load(s, nextPart) && u->Uin() && (u->Uin() != Uin())){
                contacts.users.push_back(u);
            }else{
                delete u;
            }
            continue;
        }
        break;
    }
    if (*EncryptedPassword.c_str() == 0)
        storePassword(DecryptedPassword.c_str());
    ICQEvent e(EVENT_INFO_CHANGED);
    process_event(&e);
    return true;
}

void ICQClient::connect_ready()
{
    sock->readBuffer.init(6);
    m_bHeader = true;
}

void ICQClient::addInfoRequest(unsigned long uin, bool bPriority)
{
    if (uin >= UIN_SPECIAL) return;
    for (list<unsigned long>::iterator it = infoRequestQueue.begin(); it != infoRequestQueue.end(); it++){
        if ((*it) != uin) continue;
        if (!bPriority) return;
        infoRequestQueue.remove(uin);
        break;
    }
    if (bPriority){
        infoRequestQueue.push_front(uin);
    }else{
        infoRequestQueue.push_back(uin);
    }
}

void ICQClient::addPhoneRequest(unsigned long uin, bool bPriority)
{
    if (uin >= UIN_SPECIAL) return;
    for (list<unsigned long>::iterator it = phoneRequestQueue.begin(); it != phoneRequestQueue.end(); it++){
        if ((*it) == uin) return;
    }
    if (bPriority){
        phoneRequestQueue.push_front(uin);
    }else{
        phoneRequestQueue.push_back(uin);
    }
}

void ICQClient::addResponseRequest(unsigned long uin, bool bPriority)
{
    log(L_DEBUG, "Add response request %lu", uin);
    if (uin >= UIN_SPECIAL) return;
    ICQUser *u = getUser(uin);
    if (u == NULL) return;
    if (!u->CanResponse) return;
    if ((u->Version() <= 6) || (u->direct && u->direct->isLogged())){
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
        u->addMessage(msg, this);
        return;
    }
    for (list<unsigned long>::iterator it = responseRequestQueue.begin(); it != responseRequestQueue.end(); it++){
        if ((*it) != uin) continue;
        if (!bPriority) return;
        responseRequestQueue.remove(uin);
        break;
    }
    if (bPriority){
        responseRequestQueue.push_front(uin);
    }else{
        responseRequestQueue.push_back(uin);
    }
}

void ICQClient::processInfoRequestQueue()
{
    if (m_state != Logged) return;
    if (infoRequestQueue.size() == 0) return;
    time_t now;
    time(&now);
    if (((unsigned long)now < lastInfoRequestTime + 10)) return;
    unsigned long uin = infoRequestQueue.front();
    requestInfo(uin);
    infoRequestQueue.remove(uin);
    lastInfoRequestTime = now;
}

void ICQClient::processPhoneRequestQueue(unsigned short seq)
{
    if (m_state != Logged) return;
    if (phoneRequestQueue.size() == 0) return;
    time_t now;
    time(&now);
    if ((seq != phoneRequestSeq) && (((unsigned long)now < lastPhoneRequestTime + 120))) return;
    unsigned long uin = phoneRequestQueue.front();
    requestPhoneBook(uin);
    phoneRequestQueue.remove(uin);
    lastPhoneRequestTime = now;
    phoneRequestSeq = advCounter;
    log(L_DEBUG, "Request phones %X", advCounter);
}

void ICQClient::processResponseRequestQueue(unsigned short seq)
{
    if (m_state != Logged) return;
    if (responseRequestQueue.size() == 0) return;
    time_t now;
    time(&now);
    if ((seq != responseRequestSeq) && (((unsigned long)now < lastResponseRequestTime + 20))) return;
    unsigned long uin = responseRequestQueue.front();
    requestAutoResponse(uin);
    responseRequestQueue.remove(uin);
    lastResponseRequestTime = now;
    responseRequestSeq = advCounter;
    log(L_DEBUG, "Request response %X", advCounter);
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

UTFstring::UTFstring()
{
}

UTFstring::UTFstring(const char *p)
{
    if (p) *((string*)this) = p;
}

void ICQClient::dumpPacket(Buffer &b, unsigned long start, const char *operation)
{
    if ((log_level & L_PACKET) == 0) return;
    string res;
    log(L_PACKET, "%s %u bytes", operation, b.size() - start);
    char line[81];
    char *p1 = line;
    char *p2 = line;
    unsigned n = 20;
    unsigned offs = 0;
    for (unsigned i = start; i < b.size(); i++, n++){
        char buf[32];
        if (n == 16)
            log(L_PACKET | L_SILENT, "%s", line);
        if (n >= 16){
            memset(line, ' ', 80);
            line[80] = 0;
            snprintf(buf, sizeof(buf), "     %04X: ", offs);
            memcpy(line, buf, strlen(buf));
            p1 = line + strlen(buf);
            p2 = p1 + 52;
            n = 0;
            offs += 0x10;
        }
        if (n == 8) p1++;
        unsigned char c = (unsigned char)*(b.Data(i));
        *(p2++) = ((c >= ' ') && (c != 0x7F)) ? c : '.';
        snprintf(buf, sizeof(buf), "%02X ", c);
        memcpy(p1, buf, 3);
        p1 += 3;
    }
    if (n <= 16) log(L_PACKET | L_SILENT, "%s", line);
}

