/***************************************************************************
                          various.cpp  -  description
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
#include "xml.h"
#include "log.h"

#include <memory>
#include <time.h>
#include <stdio.h>

const unsigned short ICQ_SNACxVAR_REQxSRV          = 0x0002;
const unsigned short ICQ_SNACxVAR_DATA		   = 0x0003;

const unsigned short ICQ_SRVxREQ_OFFLINE_MSG       = 0x3C00;
const unsigned short ICQ_SRVxREQ_ACK_OFFLINE_MSG   = 0x3E00;
const unsigned short ICQ_SRVxOFFLINE_MSG       	   = 0x4100;
const unsigned short ICQ_SRVxEND_OFFLINE_MSG	   = 0x4200;
const unsigned short ICQ_SRVxREQ_MORE              = 0xD007;
const unsigned short ICQ_SRVxANSWER_MORE	   = 0xDA07;

const unsigned short ICQ_SRVxREQ_FULL_INFO         = 0xB204;
const unsigned short ICQ_SRVxREQ_SHORT_INFO        = 0xBA04;
const unsigned short ICQ_SRVxREQ_SEND_SMS          = 0x8214;
const unsigned short ICQ_SRVxREQ_WP_INFO_UIN       = 0x6905;
const unsigned short ICQ_SRVxREQ_WP_SHORT          = 0x1505;
const unsigned short ICQ_SRVxREQ_WP_FULL           = 0x3305;
const unsigned short ICQ_SRVxREQ_CHANGE_PASSWD     = 0x2E04;
const unsigned short ICQ_SRVxREQ_PERMISSIONS       = 0x2404;
const unsigned short ICQ_SRVxREQ_XML_KEY		   = 0x9808;

const unsigned short ICQ_SRVxGENERAL_INFO	   = 0xC800;
const unsigned short ICQ_SRVxMORE_INFO		   = 0xDC00;
const unsigned short ICQ_SRVxEMAIL_INFO		   = 0xEB00;
const unsigned short ICQ_SRVxWORK_INFO		   = 0xD200;
const unsigned short ICQ_SRVxABOUT_INFO		   = 0xE600;
const unsigned short ICQ_SRVxINTERESTS_INFO	   = 0xF000;
const unsigned short ICQ_SRVxBACKGROUND_INFO   = 0xFA00;
const unsigned short ICQ_SRVxUNKNOWN_INFO	   = 0x0E01;

const unsigned short ICQ_SRVxREQ_MODIFY_MAIN	   = 0xEA03;
const unsigned short ICQ_SRVxREQ_MODIFY_HOME	   = 0xFD03;
const unsigned short ICQ_SRVxREQ_MODIFY_ABOUT	   = 0x0604;
const unsigned short ICQ_SRVxREQ_MODIFY_WORK       = 0xF303;
const unsigned short ICQ_SRVxREQ_MODIFY_MORE       = 0xFD03;
const unsigned short ICQ_SRVxREQ_MODIFY_INTERESTS  = 0x1004;
const unsigned short ICQ_SRVxREQ_MODIFY_BACKGROUND = 0x1A04;
const unsigned short ICQ_SRVxREQ_MODIFY_MAIL	   = 0x0B04;

const unsigned short ICQ_SRVxREQ_PHONE_UPDATE	   = 0x5406;

void ICQClientPrivate::snac_various(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxVAR_DATA:{
            TlvList tlv(sock->readBuffer);
            if (tlv(0x0001) == NULL){
                log(L_WARN, "Bad server response");
                break;
            }
            Buffer msg(*tlv(1));
            unsigned short len, nType, nId;
            unsigned long own_uin;
            msg >> len >> own_uin >> nType;
            msg.unpack(nId);
            switch (nType){
            case ICQ_SRVxEND_OFFLINE_MSG:
                log(L_DEBUG, "End offline messages");
                serverRequest(ICQ_SRVxREQ_ACK_OFFLINE_MSG);
                sendServerRequest();
                break;
            case ICQ_SRVxOFFLINE_MSG:{
                    log(L_DEBUG, "Offline message");
                    unsigned long uin;
                    char type, flag;
                    struct tm sendTM;
                    memset(&sendTM, 0, sizeof(sendTM));
                    string message;
                    unsigned short year;
                    char month, day, hours, min;
                    msg.unpack(uin);
                    msg.unpack(year);
                    msg.unpack(month);
                    msg.unpack(day);
                    msg.unpack(hours);
                    msg.unpack(min);
                    msg.unpack(type);
                    msg.unpack(flag);
                    msg.unpack(message);
#ifndef HAVE_TM_GMTOFF
                    sendTM.tm_sec  = -_timezone;
#else
                    time_t now = time (NULL);
                    sendTM = *localtime (&now);
                    sendTM.tm_sec  = -sendTM.tm_gmtoff;
#endif
                    sendTM.tm_year = year-1900;
                    sendTM.tm_mon  = month-1;
                    sendTM.tm_mday = day;
                    sendTM.tm_hour = hours;
                    sendTM.tm_min  = min;
                    sendTM.tm_isdst = -1;
                    time_t send_time = mktime(&sendTM);
                    log(L_DEBUG, "Offline message %u [%08lX] %02X %02X %s", uin, uin, type & 0xFF, flag  & 0xFF, message.c_str());
                    ICQMessage *m = parseMessage(type, uin, message, msg, 0, 0, 0, 0);
                    if (m){
                        m->Time = (unsigned long)send_time;
                        messageReceived(m);
                    }
                    break;
                }
            case ICQ_SRVxANSWER_MORE:{
                    unsigned short nSubtype;
                    char nResult;
                    msg >> nSubtype >> nResult;
                    log(L_DEBUG, "Server answer %02X %04X", nResult & 0xFF, nSubtype);
                    if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1E)){
                        ICQEvent *e = findVarEvent(nId);
                        if (e == NULL){
                            log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                            break;
                        }
                        e->failAnswer(this);
                        varEvents.remove(e);
                        delete e;
                        break;
                    }
                    ICQEvent *e = findVarEvent(nId);
                    if (e == NULL){
                        log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                        break;
                    }
                    bool nDelete = e->processAnswer(this, msg, nSubtype);
                    if (nDelete){
                        log(L_DEBUG, "Delete event");
                        varEvents.remove(e);
                        delete e;
                    }
                    break;
                }
            default:
                log(L_WARN, "Unknown SNAC(15,03) response type %04X", nType);
            }
            break;
        }
    default:
        log(L_WARN, "Unknown various family type %04X", type);
    }
}

void ICQClientPrivate::serverRequest(unsigned short cmd, unsigned short seq)
{
    snac(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxVAR_REQxSRV, true);
    sock->writeBuffer.tlv(0x0001, 0);
    sock->writeBuffer.pack((char*)&client->owner->Uin, 4);
    sock->writeBuffer << cmd;
    sock->writeBuffer.pack((unsigned short)(seq ? seq : m_nMsgSequence));
}

void ICQClientPrivate::sendServerRequest()
{
    char *packet = sock->writeBuffer.Data(m_nPacketStart);
    *((unsigned short*)(packet + 0x12)) = htons(sock->writeBuffer.size() - m_nPacketStart - 0x14);
    *((unsigned short*)(packet + 0x14)) = sock->writeBuffer.size() - m_nPacketStart - 0x16;
    sendPacket();
}

void ICQClientPrivate::sendMessageRequest()
{
    serverRequest(ICQ_SRVxREQ_OFFLINE_MSG);
    sendServerRequest();
}

class FullInfoEvent : public ICQEvent
{
public:
    FullInfoEvent(unsigned short id, unsigned long uin)
            : ICQEvent(EVENT_INFO_CHANGED, uin, EVENT_SUBTYPE_FULLINFO), m_nParts(0) { m_nId = id; }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    unsigned m_nParts;
};

bool FullInfoEvent::processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype)
{
    ICQUser *u;
    log(L_DEBUG, "Info about %lu %04X", Uin(), nSubtype);
    if (Uin() == client->client->owner->Uin){
        u = client->client->owner;
    }else{
        u = client->client->getUser(Uin(), false);
        if (u == NULL){
            log(L_WARN, "User %lu for info not found", Uin());
            return true;
        }
    }
    switch (nSubtype){
    case ICQ_SRVxGENERAL_INFO:{
            unsigned short n;
            char hideEmail;
            b
            >> u->Nick
            >> u->FirstName
            >> u->LastName
            >> u->EMail
            >> u->City
            >> u->State
            >> u->HomePhone
            >> u->HomeFax
            >> u->Address
            >> u->PrivateCellular
            >> u->Zip;
            b.unpack(n);
            u->Country = n;
            b
            >> u->TimeZone
            >> hideEmail;
            u->HiddenEMail = (hideEmail != 0);
            client->client->fromServer(u->Nick, u);
            client->client->fromServer(u->FirstName, u);
            client->client->fromServer(u->LastName, u);
            client->client->fromServer(u->City, u);
            client->client->fromServer(u->State, u);
            client->client->fromServer(u->Address, u);
            client->client->fromServer(u->Zip, u);
            client->client->fromServer(u->HomePhone, u);
            client->client->fromServer(u->HomeFax, u);
            client->client->fromServer(u->PrivateCellular, u);
            u->adjustEMails(NULL, Uin() == client->client->owner->Uin);
            if (*u->Alias.c_str() == 0)
                client->client->renameUser(u, u->Nick.c_str());
            break;
        }
    case ICQ_SRVxMORE_INFO:{
            char c;
            b
            >> u->Age
            >> c
            >> u->Gender
            >> u->Homepage;
            b.unpack(u->BirthYear);
            b
            >> u->BirthMonth
            >> u->BirthDay
            >> u->Language1
            >> u->Language2
            >> u->Language3;
            client->client->fromServer(u->Homepage, u);
            break;
        }
    case ICQ_SRVxEMAIL_INFO:{
            EMailList mails;
            char c;
            b >> c;
            for (;c > 0;c--){
                char d;
                b >> d;
                string s;
                b >> s;
                client->client->fromServer(s, u);
                if (s.length() == 0) continue;
                EMailInfo *email = new EMailInfo;
                email->Email = s;
                email->Hide = (d != 0);
                mails.push_back(email);
            }
            u->adjustEMails(&mails, Uin() == client->client->owner->Uin);
            break;
        }
    case ICQ_SRVxWORK_INFO:{
            unsigned short n;
            b
            >> u->WorkCity
            >> u->WorkState
            >> u->WorkPhone
            >> u->WorkFax
            >> u->WorkAddress
            >> u->WorkZip;
            b.unpack(n);
            u->WorkCountry = n;
            b
            >> u->WorkName
            >> u->WorkDepartment
            >> u->WorkPosition;
            b.unpack(n);
            u->Occupation = n;
            b >> u->WorkHomepage;
            client->client->fromServer(u->WorkCity, u);
            client->client->fromServer(u->WorkState, u);
            client->client->fromServer(u->WorkPhone, u);
            client->client->fromServer(u->WorkFax, u);
            client->client->fromServer(u->WorkZip, u);
            client->client->fromServer(u->WorkAddress, u);
            client->client->fromServer(u->WorkName, u);
            client->client->fromServer(u->WorkDepartment, u);
            client->client->fromServer(u->WorkPosition, u);
            client->client->fromServer(u->WorkHomepage, u);
            break;
        }
    case ICQ_SRVxABOUT_INFO:
        b >> u->About;
        client->client->fromServer(u->About, u);
        break;
    case ICQ_SRVxINTERESTS_INFO:{
            char n;
            b >> n;
            u->Interests.clear();
            for (; n > 0; n--){
                unsigned short c;
                b.unpack(c);
                string s;
                b >> s;
                client->client->fromServer(s, u);
                if (c == 0) continue;
                ExtInfo *interest = new ExtInfo;
                interest->Category = c;
                interest->Specific = s;
                u->Interests.push_back(interest);
            }
            break;
        }
    case ICQ_SRVxBACKGROUND_INFO:{
            char n;
            u->Backgrounds.clear();
            u->Affilations.clear();
            b >> n;
            for (; n > 0; n--){
                unsigned short c;
                b.unpack(c);
                string s;
                b >> s;
                client->client->fromServer(s, u);
                if (c == 0) continue;
                ExtInfo *info = new ExtInfo;
                info->Category = c;
                info->Specific = s;
                u->Backgrounds.push_back(info);
            }
            b >> n;
            for (; n > 0; n--){
                unsigned short c;
                b.unpack(c);
                string s;
                b >> s;
                client->client->fromServer(s, u);
                if (c == 0) continue;
                ExtInfo *info = new ExtInfo;
                info->Category = c;
                info->Specific = s;
                u->Affilations.push_back(info);
            }
            break;
        }
    case ICQ_SRVxUNKNOWN_INFO:
        break;
    default:
        log(L_WARN, "Unknwon info type %04X for %lu", nSubtype, Uin());
    }
    m_nParts++;
    if (m_nParts >= 8){
        u->adjustPhones();
        client->client->process_event(this);
        return true;
    }
    return false;
}

void ICQClientPrivate::requestKey(const char *key)
{
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_XML_KEY;
    string s = "<key>";
    s += key;
    s += "</key>";
    sock->writeBuffer << s;
    sendServerRequest();
}

bool ICQClientPrivate::requestInfo(unsigned long uin, bool)
{
    if (uin >= UIN_SPECIAL) return false;
    log(L_DEBUG, "Request info about %lu", uin);
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_FULL_INFO;
    sock->writeBuffer.pack(uin);
    sendServerRequest();
    varEvents.push_back(new FullInfoEvent(m_nMsgSequence, uin));
    return true;
}

ICQEvent *ICQClientPrivate::findVarEvent(unsigned short id)
{
    list<ICQEvent*>::iterator it;
    for (it = varEvents.begin(); it != varEvents.end(); it++){
        if ((*it)->m_nId == id)
            return *it;
    }
    return NULL;
}

static const char *w_days[] =
    {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Say"
    };

static const char *months[] =
    {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };

void ICQClientPrivate::processMsgQueueSMS()
{
    list<ICQEvent*>::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end();){
        if ((sock == NULL) || (sock->isError())) return;
        ICQEvent *e = *it;
        if (e->message() == NULL){
            it++;
            continue;
        }
        if (e->message()->Type() != ICQ_MSGxSMS){
            it++;
            continue;
        }
        ICQSMS *msg = static_cast<ICQSMS*>(e->message());
        XmlBranch xmltree("icq_sms_message");
        string destination = "+";
        for (const char *p = msg->Phone.c_str(); *p; p++){
            if ((*p >= '0') && (*p <= '9'))
                destination += *p;
        }
        string text = msg->Message.c_str();
        client->translate("utf8", msg->Charset.c_str(), text);
        text = client->clearHTML(text);
        string sender = client->owner->name(true);
        char uin[13];
        snprintf(uin, sizeof(uin), "%lu", client->owner->Uin);
        xmltree.pushnode(new XmlLeaf("destination",destination));
        xmltree.pushnode(new XmlLeaf("text",text));
        xmltree.pushnode(new XmlLeaf("codepage","1252"));
        xmltree.pushnode(new XmlLeaf("encoding","utf8"));
        xmltree.pushnode(new XmlLeaf("senders_UIN",uin));
        xmltree.pushnode(new XmlLeaf("senders_name",sender));
        xmltree.pushnode(new XmlLeaf("delivery_receipt","Yes"));

        /* Time string, format: Wkd, DD Mnm YYYY HH:MM:SS TMZ */
        char timestr[30];
        time_t t;
        struct tm *tm;
        time(&t);
        tm = gmtime(&t);

        snprintf(timestr, sizeof(timestr), "%s, %02u %s %04u %02u:%02u:%02u GMT",
                 w_days[tm->tm_wday], tm->tm_mday, months[tm->tm_mon], tm->tm_year + 1900,
                 tm->tm_hour, tm->tm_min, tm->tm_sec);
        xmltree.pushnode(new XmlLeaf("time",string(timestr)));
        string xmlstr = xmltree.toString(0);

        serverRequest(ICQ_SRVxREQ_MORE);
        sock->writeBuffer << ICQ_SRVxREQ_SEND_SMS
        << 0x00010016L << 0x00000000L << 0x00000000L
        << 0x00000000L << 0x00000000L << (unsigned long)(xmlstr.size());
        sock->writeBuffer << xmlstr.c_str();
        sendServerRequest();
        msgQueue.remove(e);
        e->m_nId = m_nMsgSequence;
        varEvents.push_back(e);
        it = msgQueue.begin();
    }
}

SearchEvent::SearchEvent(unsigned short id)
        : ICQEvent(EVENT_SEARCH)
{
    m_nId = id;
    auth = 0;
    state = SEARCH_STATE_DISABLED;
    lastResult = false;
}

bool SearchEvent::processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype)
{
    log(L_DEBUG, "SearchEvent::Process %u", nSubtype);
    unsigned short n;
    b >> n;
    log(L_DEBUG, "n %04X", n);
    b.unpack(m_nUin);
    b
    >> nick
    >> firstName
    >> lastName
    >> email
    >> auth
    >> state;
    client->client->fromServer(nick, client->client->owner);
    client->client->fromServer(firstName, client->client->owner);
    client->client->fromServer(lastName, client->client->owner);
    client->client->fromServer(email, client->client->owner);
    lastResult = (nSubtype == 0xAE01);
    client->client->process_event(this);
    return (nSubtype == 0xAE01);
}

ICQEvent *ICQClient::searchWP(const char *szFirst, const char *szLast, const char *szNick,
                              const char *szEmail, char age, char nGender,
                              char nLanguage, const char *szCity, const char *szState,
                              unsigned short nCountryCode,
                              const char *szCoName, const char *szCoDept, const char *szCoPos,
                              char nOccupation,
                              unsigned short nPast, const char *szPast,
                              unsigned short nInterests, const char *szInterests,
                              unsigned short nAffiliation, const char *szAffiliation,
                              unsigned short nHomePage, const char *szHomePage,
                              bool bOnlineOnly)
{
    if (!isLogged()) return 0;
    p->serverRequest(ICQ_SRVxREQ_MORE);
    p->sock->writeBuffer << ICQ_SRVxREQ_WP_FULL;
    string sFirst = szFirst ? szFirst : "";
    string sLast = szLast ? szLast : "";
    string sNick = szNick ? szNick : "";
    string sEmail = szEmail ? szEmail : "";
    string sCity = szCity ? szCity : "";
    string sState = szState ? szState : "";
    string sCoName = szCoName ? szCoName : "";
    string sCoDept = szCoDept ? szCoDept : "";
    string sCoPos = szCoPos ? szCoPos : "";
    string sPast = szPast ? szPast : "";
    string sInterests = szInterests ? szInterests : "";
    string sAffiliation = szAffiliation ? szAffiliation : "";
    string sHomePage = szHomePage ? szHomePage : "";

    toServer(sFirst, owner);
    toServer(sLast, owner);
    toServer(sNick, owner);
    toServer(sEmail, owner);
    toServer(sCity, owner);
    toServer(sState, owner);
    toServer(sCoName, owner);
    toServer(sCoDept, owner);
    toServer(sCoPos, owner);
    toServer(sPast, owner);
    toServer(sInterests, owner);
    toServer(sAffiliation, owner);
    toServer(sHomePage, owner);

    unsigned short nMinAge = 0;
    unsigned short nMaxAge = 0;
    switch (age){
    case 1:
        nMinAge = 18;
        nMaxAge = 22;
        break;
    case 2:
        nMinAge = 23;
        nMaxAge = 29;
        break;
    case 3:
        nMinAge = 30;
        nMaxAge = 39;
        break;
    case 4:
        nMinAge = 40;
        nMaxAge = 49;
        break;
    case 5:
        nMinAge = 50;
        nMaxAge = 59;
        break;
    case 6:
        nMinAge = 60;
        nMaxAge = 120;
        break;
    }

    p->sock->writeBuffer
    << sFirst
    << sLast
    << sNick
    << sEmail;
    p->sock->writeBuffer.pack(nMinAge);
    p->sock->writeBuffer.pack(nMaxAge);
    p->sock->writeBuffer
    << nGender
    << nLanguage
    << sCity
    << sState;
    p->sock->writeBuffer.pack(nCountryCode);
    p->sock->writeBuffer
    << sCoName
    << sCoDept
    << sCoPos

    << nOccupation
    << nPast
    << sPast
    << nInterests
    << sInterests
    << nAffiliation
    << sAffiliation
    << nHomePage
    << sHomePage;

    char c = bOnlineOnly ? 1 : 0;
    p->sock->writeBuffer << c;

    p->sendServerRequest();

    ICQEvent *e = new SearchEvent(p->m_nMsgSequence);
    p->varEvents.push_back(e);
    return e;
}

ICQEvent *ICQClient::searchByName(const char *first, const char *last, const char *nick, bool bOnline)
{
    if (!isLogged()) return 0;
    p->serverRequest(ICQ_SRVxREQ_MORE);
    p->sock->writeBuffer << ICQ_SRVxREQ_WP_SHORT;
    string sFirst = first ? first : "";
    string sLast = last ? last : "";
    string sNick = nick ? nick : "";
    log(L_DEBUG, "-- [%s] [%s] [%s]", first, last, nick);
    toServer(sFirst, owner);
    toServer(sLast, owner);
    toServer(sNick, owner);
    p->sock->writeBuffer
    << sNick
    << sFirst
    << sNick;
    if (bOnline){
        p->sock->writeBuffer
        << 0x30020100L
        << (char)0x01;
    }
    p->sendServerRequest();
    ICQEvent *e = new SearchEvent(p->m_nMsgSequence);
    p->varEvents.push_back(e);
    return e;
}

ICQEvent *ICQClient::searchByUin(unsigned long uin)
{
    if (!isLogged()) return 0;
    p->serverRequest(ICQ_SRVxREQ_MORE);
    p->sock->writeBuffer
    << ICQ_SRVxREQ_WP_INFO_UIN
    << 0x36010400L;
    p->sock->writeBuffer.pack(uin);
    p->sendServerRequest();
    ICQEvent *e = new SearchEvent(p->m_nMsgSequence);
    p->varEvents.push_back(e);
    return e;
}

class SetPasswordEvent : public ICQEvent
{
public:
    SetPasswordEvent(unsigned short id, const char *_passwd) : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        passwd = strdup(_passwd);
    }
    ~SetPasswordEvent() { free(passwd); }
protected:
    char *passwd;
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
};

bool SetPasswordEvent::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    m_nUin = client->client->owner->Uin;
    client->client->storePassword(passwd);
    return true;
}

void ICQClient::setPassword(const char *passwd)
{
    if (!isLogged()) return;
    p->serverRequest(ICQ_SRVxREQ_MORE);
    p->sock->writeBuffer << ICQ_SRVxREQ_CHANGE_PASSWD;
    string pass = passwd;
    toServer(pass, owner);
    p->sock->writeBuffer << pass;
    p->sendServerRequest();
    SetPasswordEvent *e = new SetPasswordEvent(p->m_nMsgSequence, passwd);
    p->varEvents.push_back(e);
}

class SetSecurityInfo : public ICQEvent
{
public:
    SetSecurityInfo(unsigned short id, bool _bAuthorize, bool _bWebAware)
            : ICQEvent(EVENT_INFO_CHANGED), bAuthorize(_bAuthorize), bWebAware(_bWebAware)
    {
        m_nId = id;
    }
protected:
    bool bAuthorize;
    bool bWebAware;
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
};

bool SetSecurityInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    m_nUin = client->client->owner->Uin;
    client->client->Authorize = bAuthorize;
    client->client->WebAware = bWebAware;
    return true;
}

void ICQClient::setSecurityInfo(bool bAuthorize, bool bWebAware)
{
    if (!isLogged()) return;
    p->serverRequest(ICQ_SRVxREQ_MORE);
    p->sock->writeBuffer
    << ICQ_SRVxREQ_PERMISSIONS
    << (char)(bAuthorize ? 0 : 0x01)
    << (char)(bWebAware ? 0x01 : 0)
    << (char)0x02
    << (char)0;
    p->sendServerRequest();
    SetSecurityInfo *e = new SetSecurityInfo(p->m_nMsgSequence, bAuthorize, bWebAware);
    p->varEvents.push_back(e);
}

#define INIT(A)			A = u->A;
#define SET(A)			client->client->owner->A = A;

#define SPARAM(A)		string s_##A = u->A;			\
						client->toServer(s_##A, u);				\
						if (client->owner->A != u->A) bChange = true;
#define	NPARAM(A)		if (client->owner->A != u->A) bChange = true;

class SetMainInfo : public ICQEvent
{
public:
    SetMainInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(Nick);
        INIT(FirstName);
        INIT(LastName);
        INIT(EMail);
        INIT(City);
        INIT(State);
        INIT(HomePhone);
        INIT(HomeFax);
        INIT(Address);
        INIT(PrivateCellular);
        INIT(Zip);
        INIT(Country);
        INIT(TimeZone);
        INIT(HiddenEMail);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    string Nick;
    string FirstName;
    string LastName;
    string EMail;
    string City;
    string State;
    string HomePhone;
    string HomeFax;
    string Address;
    string PrivateCellular;
    string Zip;
    unsigned short Country;
    char TimeZone;
    bool HiddenEMail;
};

bool SetMainInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(Nick);
    SET(FirstName);
    SET(LastName);
    SET(EMail);
    SET(City);
    SET(State);
    SET(HomePhone);
    SET(HomeFax);
    SET(Address);
    SET(PrivateCellular);
    SET(Zip);
    SET(Country);
    SET(TimeZone);
    SET(HiddenEMail);
    return true;
}

bool ICQClientPrivate::setMainInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    SPARAM(Nick);
    SPARAM(FirstName);
    SPARAM(LastName);
    SPARAM(EMail);
    SPARAM(City);
    SPARAM(State);
    SPARAM(HomePhone);
    SPARAM(HomeFax);
    SPARAM(Address);
    SPARAM(PrivateCellular);
    SPARAM(Zip);
    NPARAM(Country);
    NPARAM(TimeZone);
    NPARAM(HiddenEMail);

    // if (!bChange) return false;

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_MODIFY_MAIN
    << s_Nick
    << s_FirstName
    << s_LastName
    << s_EMail
    << s_City
    << s_State
    << s_HomePhone
    << s_HomeFax
    << s_Address
    << s_PrivateCellular
    << s_Zip;
    sock->writeBuffer.pack(u->Country);
    sock->writeBuffer
    << u->TimeZone
    << u->HiddenEMail;
    sendServerRequest();

    SetMainInfo *e = new SetMainInfo(m_nMsgSequence, u);
    varEvents.push_back(e);
    return true;
}


class SetWorkInfo : public ICQEvent
{
public:
    SetWorkInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(WorkCity);
        INIT(WorkState);
        INIT(WorkPhone);
        INIT(WorkFax);
        INIT(WorkAddress);
        INIT(WorkZip);
        INIT(WorkCountry);
        INIT(WorkName);
        INIT(WorkDepartment);
        INIT(WorkPosition);
        INIT(Occupation);
        INIT(WorkHomepage);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    string WorkCity;
    string WorkState;
    string WorkPhone;
    string WorkFax;
    string WorkAddress;
    string WorkZip;
    unsigned short WorkCountry;
    string WorkName;
    string WorkDepartment;
    string WorkPosition;
    unsigned short Occupation;
    string WorkHomepage;
};

bool SetWorkInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(WorkCity);
    SET(WorkState);
    SET(WorkAddress);
    SET(WorkZip);
    SET(WorkCountry);
    SET(WorkName);
    SET(WorkDepartment);
    SET(WorkPosition);
    SET(Occupation);
    SET(WorkHomepage);
    return true;
}

bool ICQClientPrivate::setWorkInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    SPARAM(WorkCity);
    SPARAM(WorkState);
    SPARAM(WorkPhone);
    SPARAM(WorkFax);
    SPARAM(WorkAddress);
    SPARAM(WorkZip);
    NPARAM(WorkCountry);
    SPARAM(WorkName);
    SPARAM(WorkDepartment);
    SPARAM(WorkPosition);
    NPARAM(Occupation);
    SPARAM(WorkHomepage);

    // if (!bChange) return false;

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer
    << ICQ_SRVxREQ_MODIFY_WORK
    << s_WorkCity
    << s_WorkState
    << s_WorkPhone
    << s_WorkFax
    << s_WorkAddress
    << s_WorkZip;
    sock->writeBuffer.pack(u->WorkCountry);
    sock->writeBuffer
    << s_WorkName
    << s_WorkDepartment
    << s_WorkPosition;
    sock->writeBuffer.pack(u->Occupation);
    sock->writeBuffer
    << s_WorkHomepage;
    sendServerRequest();
    SetWorkInfo *e = new SetWorkInfo(m_nMsgSequence, u);
    varEvents.push_back(e);
    return true;
}

class SetMoreInfo : public ICQEvent
{
public:
    SetMoreInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(Age);
        INIT(Gender);
        INIT(Homepage);
        INIT(BirthYear);
        INIT(BirthMonth);
        INIT(BirthDay);
        INIT(Language1);
        INIT(Language2);
        INIT(Language3);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    char Age;
    char Gender;
    string Homepage;
    unsigned short BirthYear;
    char BirthMonth;
    char BirthDay;
    char Language1;
    char Language2;
    char Language3;
};

bool SetMoreInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(Age);
    SET(Gender);
    SET(Homepage);
    SET(BirthYear);
    SET(BirthMonth);
    SET(BirthDay);
    SET(Language1);
    SET(Language2);
    SET(Language3);
    client->checkBirthDay();
    return true;
}

bool ICQClientPrivate::setMoreInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    NPARAM(Age);
    NPARAM(Gender);
    NPARAM(BirthYear);
    NPARAM(BirthMonth);
    NPARAM(BirthDay);
    NPARAM(Language1);
    NPARAM(Language2);
    NPARAM(Language3);
    SPARAM(Homepage);

    // if (!bChange) return false;

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_MODIFY_MORE
    << u->Age
    << (char)0
    << u->Gender
    << s_Homepage;
    sock->writeBuffer.pack(u->BirthYear);
    sock->writeBuffer
    << u->BirthMonth
    << u->BirthDay
    << u->Language1
    << u->Language2
    << u->Language3;
    sendServerRequest();
    SetMoreInfo *e = new SetMoreInfo(m_nMsgSequence, u);
    varEvents.push_back(e);
    return true;
}

class SetAboutInfo : public ICQEvent
{
public:
    SetAboutInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(About);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    string About;
};

bool SetAboutInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(About);
    return true;
}

bool ICQClientPrivate::setAboutInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    SPARAM(About);

    // if (!bChange) return false;

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer
    << ICQ_SRVxREQ_MODIFY_ABOUT
    << s_About;
    sendServerRequest();
    SetAboutInfo *e = new SetAboutInfo(m_nMsgSequence, u);
    varEvents.push_back(e);
    return true;
}

class SetInterestsInfo : public ICQEvent
{
public:
    SetInterestsInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(Interests);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    ExtInfoList Interests;
};

void ICQClientPrivate::packInfoList(const ExtInfoList &infoList)
{
    char n = infoList.size();
    sock->writeBuffer << n;
    for (ExtInfoList::const_iterator it = infoList.begin(); it != infoList.end(); ++it){
        ExtInfo *info = static_cast<ExtInfo*>(*it);
        string spec = info->Specific;
        client->toServer(spec, client->owner);
        sock->writeBuffer.pack(info->Category);
        sock->writeBuffer << spec;
    }
}

bool SetInterestsInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(Interests);
    return true;
}

bool ICQClientPrivate::setInterestsInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    NPARAM(Interests);

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_MODIFY_INTERESTS;
    packInfoList(u->Interests);
    sendServerRequest();
    SetInterestsInfo *e = new SetInterestsInfo(m_nMsgSequence, u);
    varEvents.push_back(e);
    return true;
}

class SetBackgroundInfo : public ICQEvent
{
public:
    SetBackgroundInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(Backgrounds);
        INIT(Affilations);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    ExtInfoList Backgrounds;
    ExtInfoList Affilations;
};

bool SetBackgroundInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(Backgrounds);
    SET(Affilations);
    return true;
}

bool ICQClientPrivate::setBackgroundInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    NPARAM(Backgrounds);
    NPARAM(Affilations);

    // if (!bChange) return false;

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_MODIFY_BACKGROUND;
    packInfoList(u->Backgrounds);
    packInfoList(u->Affilations);
    sendServerRequest();
    SetBackgroundInfo *e = new SetBackgroundInfo(m_nMsgSequence, u);
    varEvents.push_back(e);

    return true;
}

class SetMailInfo : public ICQEvent
{
public:
    SetMailInfo(unsigned short id, ICQUser *u)
            : ICQEvent(EVENT_INFO_CHANGED)
    {
        m_nId = id;
        INIT(EMails);
    }
protected:
    bool processAnswer(ICQClientPrivate *client, Buffer &b, unsigned short nSubtype);
    EMailList EMails;
};

bool SetMailInfo::processAnswer(ICQClientPrivate *client, Buffer&, unsigned short)
{
    SET(EMails);
    return true;
}

bool ICQClientPrivate::setMailInfo(ICQUser *u)
{
    if (m_state != Logged) return false;

    bool bChange = false;
    NPARAM(EMails);

    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_MODIFY_MAIL;

    char c = u->EMails.size();
    c--;
    if (c < 0) c = 0;
    sock->writeBuffer << c;

    bool bFirst = true;
    for (EMailList::iterator it = u->EMails.begin(); it != u->EMails.end(); ++it){
        EMailInfo *info = static_cast<EMailInfo*>(*it);
        if (bFirst){
            bFirst = false;
            continue;
        }
        string s;
        if (info->Email.size()) s = info->Email;
        client->toServer(s, client->owner);
        char hide = info->Hide ? 1 : 0;
        sock->writeBuffer << hide << s;
    }

    sendServerRequest();
    SetMailInfo *e = new SetMailInfo(m_nMsgSequence, u);
    varEvents.push_back(e);
    return true;
}

void ICQClient::setInfo(ICQUser *u)
{
    bool bChange = false;
    if (p->setMainInfo(u)) bChange = true;
    if (p->setMoreInfo(u)) bChange = true;
    if (p->setWorkInfo(u)) bChange = true;
    if (p->setAboutInfo(u)) bChange = true;
    if (p->setMailInfo(u)) bChange = true;
    if (p->setInterestsInfo(u)) bChange = true;
    if (p->setBackgroundInfo(u)) bChange = true;
    if (owner->Phones != u->Phones){
        owner->Phones = u->Phones;
        if (updatePhoneBook()) bChange = true;
    }
    if (bChange) p->sendInfoUpdate();
}

void ICQClientPrivate::sendPhoneStatus()
{
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << (unsigned short)ICQ_SRVxREQ_PHONE_UPDATE;
    sock->writeBuffer.pack((char*)PHONEBOOK_SIGN, 0x10);
    sock->writeBuffer
    << (unsigned short)0x0200
    << client->owner->PhoneState
    << (char)0
    << (unsigned short)0;
    sendServerRequest();
}

void ICQClientPrivate::sendShareUpdate()
{
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << (unsigned short)ICQ_SRVxREQ_PHONE_UPDATE;
    sock->writeBuffer.pack((char*)SHARED_FILES_SIGN, 0x10);
    sock->writeBuffer.pack((unsigned short)4);
    sock->writeBuffer.pack((unsigned long)(client->ShareOn ? 1 : 0));
    sendServerRequest();
}
