/***************************************************************************
                          various.cpp  -  description
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

const unsigned short ICQ_SRVxREQ_PHONE_INIT		   = 0x5807;
const unsigned short ICQ_SRVxREQ_PHONE_UPDATE	   = 0x5406;

void ICQClient::snac_various(unsigned short type, unsigned short)
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
            msg >> len >> own_uin >> nType >> nId;
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
                    msg >> uin
                    >> year >> month >> day >> hours >> min
                    >> type >> flag;
                    msg >> message;
                    uin = htonl(uin);
                    year = htons(year);
                    sendTM.tm_year = year-1900;
                    sendTM.tm_mon  = month-1;
                    sendTM.tm_mday = day;
                    sendTM.tm_hour = hours;
                    sendTM.tm_min  = min;
                    sendTM.tm_sec  = 0;
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
                        ICQEvent *e = findVarEvent(htons(nId));
                        if (e == NULL){
                            log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                            break;
                        }
                        e->failAnswer(this);
                        varEvents.remove(e);
                        delete e;
                        break;
                    }
                    ICQEvent *e = findVarEvent(htons(nId));
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

void ICQClient::serverRequest(unsigned short cmd, unsigned short seq)
{
    snac(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxVAR_REQxSRV, true);
    sock->writeBuffer.tlv(0x0001, 0);
    sock->writeBuffer.pack((char*)&Uin(), 4);
    sock->writeBuffer << cmd;
    sock->writeBuffer << htons(seq ? seq : m_nMsgSequence);
}

void ICQClient::sendServerRequest()
{
    char *packet = sock->writeBuffer.Data(m_nPacketStart);
    *((unsigned short*)(packet + 0x12)) = htons(sock->writeBuffer.size() - m_nPacketStart - 0x14);
    *((unsigned short*)(packet + 0x14)) = sock->writeBuffer.size() - m_nPacketStart - 0x16;
    sendPacket();
}

void ICQClient::sendMessageRequest()
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
    unsigned m_nParts;
};

bool FullInfoEvent::processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype)
{
    ICQUser *u;
    log(L_DEBUG, "Info about %lu %04X", Uin(), nSubtype);
    if (Uin() == client->Uin){
        u = client;
    }else{
        u = client->getUser(Uin(), false);
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
            client->fromServer(u->Nick, u);
            client->fromServer(u->FirstName, u);
            client->fromServer(u->LastName, u);
            client->fromServer(u->City, u);
            client->fromServer(u->State, u);
            client->fromServer(u->Address, u);
            client->fromServer(u->Zip, u);
            client->fromServer(u->HomePhone, u);
            client->fromServer(u->HomeFax, u);
            client->fromServer(u->PrivateCellular, u);
            u->adjustEMails(u->EMails);
            if (*u->Alias.c_str() == 0)
                client->renameUser(u, u->Nick);
            break;
        }
    case ICQ_SRVxMORE_INFO:{
            char c;
            b
            >> u->Age
            >> c
            >> u->Gender
            >> u->Homepage
            >> u->BirthYear
            >> u->BirthMonth
            >> u->BirthDay
            >> u->Language1
            >> u->Language2
            >> u->Language3;
            u->BirthYear = htons(u->BirthYear());
            client->fromServer(u->Homepage, u);
            break;
        }
    case ICQ_SRVxEMAIL_INFO:{
            EMailPtrList mails;
            char c;
            b >> c;
            for (;c > 0;c--){
                char d;
                b >> d;
                string s;
                b >> s;
                client->fromServer(s, u);
                if (s.length() == 0) continue;
                EMailInfo *email = new EMailInfo;
                email->Email = s;
                email->Hide = (d != 0);
                mails.push_back(email);
            }
            u->adjustEMails(mails);
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
            client->fromServer(u->WorkCity, u);
            client->fromServer(u->WorkState, u);
            client->fromServer(u->WorkPhone, u);
            client->fromServer(u->WorkFax, u);
            client->fromServer(u->WorkZip, u);
            client->fromServer(u->WorkAddress, u);
            client->fromServer(u->WorkName, u);
            client->fromServer(u->WorkDepartment, u);
            client->fromServer(u->WorkPosition, u);
            client->fromServer(u->WorkHomepage, u);
            break;
        }
    case ICQ_SRVxABOUT_INFO:
        b >> u->About;
        client->fromServer(u->About, u);
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
                client->fromServer(s, u);
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
                client->fromServer(s, u);
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
                client->fromServer(s, u);
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
        client->process_event(this);
        return true;
    }
    return false;
}

void ICQClient::requestKey(const char *key)
{
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_XML_KEY;
    string s = "<key>";
    s += key;
    s += "</key>";
    sock->writeBuffer << s;
    sendServerRequest();
}

void ICQClient::requestInfo(unsigned long uin)
{
    if (uin >= UIN_SPECIAL) return;
    log(L_DEBUG, "Request info about %lu", uin);
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_FULL_INFO;
    sock->writeBuffer << (unsigned long)htonl(uin);
    sendServerRequest();
    varEvents.push_back(new FullInfoEvent(m_nMsgSequence, uin));
}

ICQEvent *ICQClient::findVarEvent(unsigned short id)
{
    list<ICQEvent*>::iterator it;
    for (it = varEvents.begin(); it != varEvents.end(); it++){
        if ((*it)->m_nId == id)
            return *it;
    }
    return NULL;
}

void ICQClient::processMsgQueueSMS()
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
        UTFstring text = clearHTML(msg->Message.c_str());
        string sender = name(true);
        char uin[13];
        snprintf(uin, sizeof(uin), "%lu", Uin());
        xmltree.pushnode(new XmlLeaf("destination",destination));
        xmltree.pushnode(new XmlLeaf("text",text));
        xmltree.pushnode(new XmlLeaf("codepage","1252"));
        xmltree.pushnode(new XmlLeaf("encoding","urf8"));
        xmltree.pushnode(new XmlLeaf("senders_UIN",uin));
        xmltree.pushnode(new XmlLeaf("senders_name",sender));
        xmltree.pushnode(new XmlLeaf("delivery_receipt","Yes"));

        /* Time string, format: Wkd, DD Mnm YYYY HH:MM:SS TMZ */
        char timestr[30];
        time_t t;
        struct tm *tm;
        time(&t);
        tm = gmtime(&t);
        strftime(timestr, 30, "%a, %d %b %Y %T %Z", tm);
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

bool SearchEvent::processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype)
{
    log(L_DEBUG, "SearchEvent::Process %u", nSubtype);
    unsigned short n;
    b >> n;
    log(L_DEBUG, "n %04X", n);
    unsigned long uin;
    b
    >> uin
    >> nick
    >> firstName
    >> lastName
    >> email
    >> auth
    >> state;
    m_nUin = htonl(uin);
    if (m_nUin != client->Uin()){
        client->fromServer(nick, client);
        client->fromServer(firstName, client);
        client->fromServer(lastName, client);
        client->fromServer(email, client);
        lastResult = (nSubtype == 0xAE01);
        client->process_event(this);
    }
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
    if (m_state != Logged) return 0;
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_WP_FULL;
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

    toServer(sFirst, this);
    toServer(sLast, this);
    toServer(sNick, this);
    toServer(sEmail, this);
    toServer(sCity, this);
    toServer(sState, this);
    toServer(sCoName, this);
    toServer(sCoDept, this);
    toServer(sCoPos, this);
    toServer(sPast, this);
    toServer(sInterests, this);
    toServer(sAffiliation, this);
    toServer(sHomePage, this);

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

    sock->writeBuffer
    << sFirst
    << sLast
    << sNick
    << sEmail
    << (unsigned short)htons(nMinAge)
    << (unsigned short)htons(nMaxAge)
    << nGender
    << nLanguage
    << sCity
    << sState
    << (unsigned short)htons(nCountryCode)
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
    sock->writeBuffer << c;

    sendServerRequest();

    ICQEvent *e = new SearchEvent(m_nMsgSequence);
    varEvents.push_back(e);
    return e;
}

ICQEvent *ICQClient::searchByName(const char *first, const char *last, const char *nick, bool bOnline)
{
    if (m_state != Logged) return 0;
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_WP_SHORT;
    string sFirst = first ? first : "";
    string sLast = last ? last : "";
    string sNick = nick ? nick : "";
    log(L_DEBUG, "-- [%s] [%s] [%s]", first, last, nick);
    toServer(sFirst, this);
    toServer(sLast, this);
    toServer(sNick, this);
    sock->writeBuffer
    << sNick
    << sFirst
    << sNick;
    if (bOnline){
        sock->writeBuffer
        << 0x30020100L
        << (char)0x01;
    }
    sendServerRequest();
    ICQEvent *e = new SearchEvent(m_nMsgSequence);
    varEvents.push_back(e);
    return e;
}

ICQEvent *ICQClient::searchByUin(unsigned long uin)
{
    if (m_state != Logged) return 0;
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer
    << ICQ_SRVxREQ_WP_INFO_UIN
    << 0x36010400L
    << (unsigned long)htonl(uin);
    sendServerRequest();
    ICQEvent *e = new SearchEvent(m_nMsgSequence);
    varEvents.push_back(e);
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
};

bool SetPasswordEvent::processAnswer(ICQClient *client, Buffer&, unsigned short)
{
    m_nUin = client->Uin();
    client->storePassword(passwd);
    return true;
}

void ICQClient::setPassword(const char *passwd)
{
    if (m_state != Logged) return;
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << ICQ_SRVxREQ_CHANGE_PASSWD;
    string p = passwd;
    toServer(p, this);
    sock->writeBuffer << p;
    sendServerRequest();
    SetPasswordEvent *e = new SetPasswordEvent(m_nMsgSequence, passwd);
    varEvents.push_back(e);
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
};

bool SetSecurityInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
{
    m_nUin = client->Uin();
    client->Authorize = bAuthorize;
    client->WebAware = bWebAware;
    return true;
}

void ICQClient::setSecurityInfo(bool bAuthorize, bool bWebAware)
{
    if (m_state != Logged) return;
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer
    << ICQ_SRVxREQ_PERMISSIONS
    << (char)(bAuthorize ? 0 : 0x01)
    << (char)(bWebAware ? 0x01 : 0)
    << (char)0x02
    << (char)0;
    sendServerRequest();
    SetSecurityInfo *e = new SetSecurityInfo(m_nMsgSequence, bAuthorize, bWebAware);
    varEvents.push_back(e);
}

#define INIT(A)			A = u->A();
#define SET(A)			client->A = A;

#define SPARAM(A)		string s_##A = u->A;			\
						toServer(s_##A, u);				\
						if (A != u->A) bChange = true;
#define	NPARAM(A)		if (A != u->A) bChange = true;

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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
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

bool SetMainInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
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

bool ICQClient::setMainInfo(ICQUser *u)
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
    << s_Zip
    << (unsigned short)htons(u->Country())
    << u->TimeZone()
    << u->HiddenEMail();
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
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

bool SetWorkInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
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

bool ICQClient::setWorkInfo(ICQUser *u)
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
    << s_WorkZip
    << htons(u->WorkCountry())
    << s_WorkName
    << s_WorkDepartment
    << s_WorkPosition
    << htons(u->Occupation())
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
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

bool SetMoreInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
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
    return true;
}

bool ICQClient::setMoreInfo(ICQUser *u)
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
    << u->Age()
    << (char)0
    << u->Gender()
    << s_Homepage
    << (unsigned short)htons(u->BirthYear())
    << u->BirthMonth()
    << u->BirthDay()
    << u->Language1()
    << u->Language2()
    << u->Language3();
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
    string About;
};

bool SetAboutInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
{
    SET(About);
    return true;
}

bool ICQClient::setAboutInfo(ICQUser *u)
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
    ExtInfoPtrList Interests;
};

void ICQClient::packInfoList(const ExtInfoList &infoList)
{
    char n = infoList.size();
    sock->writeBuffer << n;
    for (ConfigPtrList::const_iterator it = infoList.begin(); it != infoList.end(); ++it){
        ExtInfo *info = static_cast<ExtInfo*>(*it);
        string spec = info->Specific;
        toServer(spec, this);
        sock->writeBuffer << htons(info->Category()) << spec;
    }
}

bool SetInterestsInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
{
    SET(Interests);
    return true;
}

bool ICQClient::setInterestsInfo(ICQUser *u)
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
    ExtInfoPtrList Backgrounds;
    ExtInfoPtrList Affilations;
};

bool SetBackgroundInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
{
    SET(Backgrounds);
    SET(Affilations);
    return true;
}

bool ICQClient::setBackgroundInfo(ICQUser *u)
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
    bool processAnswer(ICQClient *client, Buffer &b, unsigned short nSubtype);
    EMailPtrList EMails;
};

bool SetMailInfo::processAnswer(ICQClient *client, Buffer&, unsigned short)
{
    SET(EMails);
    return true;
}

bool ICQClient::setMailInfo(ICQUser *u)
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
        if (info->Email) s = info->Email;
        toServer(s, this);
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
    if (setMainInfo(u)) bChange = true;
    if (setMoreInfo(u)) bChange = true;
    if (setWorkInfo(u)) bChange = true;
    if (setAboutInfo(u)) bChange = true;
    if (setMailInfo(u)) bChange = true;
    if (setInterestsInfo(u)) bChange = true;
    if (setBackgroundInfo(u)) bChange = true;
    /*
        if (Phones != u->Phones){
            Phones = u->Phones;
            if (updatePhoneBook()) bChange = true;
        }
    */
    //    if (bChange) sendInfoUpdate();
}

void ICQClient::sendPhoneInit()
{
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << (unsigned short)ICQ_SRVxREQ_PHONE_INIT;
    sock->writeBuffer
    << 0x01000000L << 0x03200000L << 0x00000000L << 0x00000000L
    << 0x00000408L << 0x00000000L << 0x00000050L << 0x00000003L
    << (char)0 << (unsigned short)0;
    sendServerRequest();
}

void ICQClient::sendPhoneStatus()
{
    serverRequest(ICQ_SRVxREQ_MORE);
    sock->writeBuffer << (unsigned short)ICQ_SRVxREQ_PHONE_UPDATE;
    sock->writeBuffer.pack((char*)PHONEBOOK_SIGN, 0x10);
    sock->writeBuffer
    << (unsigned short)0x0200
    << PhoneState
    << (char)0
    << (unsigned short)0;
    sendServerRequest();
}

