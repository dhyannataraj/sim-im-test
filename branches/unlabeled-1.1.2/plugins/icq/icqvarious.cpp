/***************************************************************************
                          icqvarious.cpp  -  description
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

#include <time.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <qtimer.h>
#include "xml.h"

const unsigned short ICQ_SNACxVAR_ERROR			   = 0x0001;
const unsigned short ICQ_SNACxVAR_REQxSRV          = 0x0002;
const unsigned short ICQ_SNACxVAR_DATA			   = 0x0003;

const unsigned short ICQ_SRVxREQ_OFFLINE_MSG       = 0x3C00;
const unsigned short ICQ_SRVxREQ_ACK_OFFLINE_MSG   = 0x3E00;
const unsigned short ICQ_SRVxOFFLINE_MSG       	   = 0x4100;
const unsigned short ICQ_SRVxEND_OFFLINE_MSG	   = 0x4200;
const unsigned short ICQ_SRVxREQ_MORE              = 0xD007;
const unsigned short ICQ_SRVxANSWER_MORE           = 0xDA07;

const unsigned short ICQ_SRVxREQ_FULL_INFO         = 0xB204;
const unsigned short ICQ_SRVxREQ_SHORT_INFO        = 0xBA04;
const unsigned short ICQ_SRVxREQ_OWN_INFO		   = 0xD004;
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
const unsigned short ICQ_SRVxREQ_SET_CHAT_GROUP	   = 0x5807;
const unsigned short ICQ_SRVxREQ_RANDOM_CHAT	   = 0x4E07;

const char SEARCH_STATE_OFFLINE  = 0;
const char SEARCH_STATE_ONLINE   = 1;
const char SEARCH_STATE_DISABLED = 2;

const unsigned ERROR_RATE_LIMIT		= 0x0002;

const unsigned INFO_REQUEST_TIMEOUT = 60;
const unsigned INFO_PAUSE_TIMEOUT   = 300;

const unsigned short PAUSE_ID		= 0xFFFF;

class ServerRequest
{
public:
    ServerRequest(unsigned short id);
    virtual ~ServerRequest() {}
    unsigned short id() { return m_id; }
    virtual bool answer(Buffer&, unsigned short nSubType) = 0;
    virtual void fail(unsigned short error_code = 0);
protected:
    unsigned m_id;
};

ServerRequest::ServerRequest(unsigned short id)
{
    m_id = id;
}

ServerRequest *ICQClient::findServerRequest(unsigned short id)
{
    for (list<ServerRequest*>::iterator it = varRequests.begin(); it != varRequests.end(); ++it){
        if ((*it)->id() == id)
            return *it;
    }
    return NULL;
}

void ServerRequest::fail(unsigned short)
{
}

void ICQClient::clearServerRequests()
{
    for (list<ServerRequest*>::iterator it_req = varRequests.begin(); it_req != varRequests.end(); ++it_req){
        (*it_req)->fail();
        delete *it_req;
    }
    varRequests.clear();
    list<unsigned long>::iterator it;
    for (it = infoRequests.begin(); it != infoRequests.end(); ++it){
        Contact *contact = getContacts()->contact(*it);
        if (contact == NULL)
            continue;
        Event e(EventFetchInfoFail, contact);
        e.process();
    }
    infoRequests.clear();
}

void ICQClient::snac_various(unsigned short type, unsigned short id)
{
    switch (type){
    case ICQ_SNACxVAR_ERROR:{
            unsigned short error_code;
            m_socket->readBuffer >> error_code;
            ServerRequest *req = findServerRequest(id);
            if (req == NULL){
                log(L_WARN, "Various event ID %04X not found for error %04X", id, error_code);
                break;
            }
            req->fail(error_code);
            break;
        }
    case ICQ_SNACxVAR_DATA:{
            TlvList tlv(m_socket->readBuffer);
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
                serverRequest(ICQ_SRVxREQ_ACK_OFFLINE_MSG);
                sendServerRequest();
                setChatGroup();
                addFullInfoRequest(data.owner.Uin, false);
                m_bServerReady = true;
                infoRequest();
                processListRequest();
                break;
            case ICQ_SRVxOFFLINE_MSG:{
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
                    MessageId id;
                    Message *m = parseMessage(type, uin, message, msg, 0, 0, id);
                    if (m){
                        m->setTime(send_time);
                        messageReceived(m, uin);
                    }
                    break;
                }
            case ICQ_SRVxANSWER_MORE:{
                    unsigned short nSubtype;
                    char nResult;
                    msg >> nSubtype >> nResult;
                    if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1E)){
                        ServerRequest *req = findServerRequest(nId);
                        if (req == NULL){
                            log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                            break;
                        }
                        req->fail();
                        varRequests.remove(req);
                        delete req;
                        break;
                    }
                    ServerRequest *req = findServerRequest(nId);
                    if (req == NULL){
                        log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                        break;
                    }
                    if (req->answer(msg, nSubtype)){
                        varRequests.remove(req);
                        delete req;
                    }
                    break;
                }
                break;
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
    m_socket->writeBuffer.tlv(0x0001, 0);
    m_socket->writeBuffer.pack(data.owner.Uin);
    m_socket->writeBuffer << cmd;
    m_socket->writeBuffer.pack((unsigned short)(seq ? seq : m_nMsgSequence));
}

void ICQClient::sendServerRequest()
{
    Buffer &b = m_socket->writeBuffer;
    char *packet = b.data(b.packetStartPos());
    unsigned short packet_size = b.size() - b.packetStartPos();
    *((unsigned short*)(packet + 0x12)) = htons(packet_size - 0x14);
    *((unsigned short*)(packet + 0x14)) = packet_size - 0x16;
    sendPacket();
}

#if 0

void ICQClient::sendPhoneStatus()
{
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << (unsigned short)ICQ_SRVxREQ_PHONE_UPDATE;
    m_socket->writeBuffer.pack((const char*)(capabilities[CAP_PHONEBOOK]), sizeof(capability));
    char PhoneState = data.owner.PhoneState;
    m_socket->writeBuffer
    << (unsigned short)0x0200
    << PhoneState
    << (char)0
    << (unsigned short)0;
    sendServerRequest();
}

#endif

void ICQClient::sendMessageRequest()
{
    serverRequest(ICQ_SRVxREQ_OFFLINE_MSG);
    sendServerRequest();
}

// _________________________________________________________________________________________

class FullInfoRequest : public ServerRequest
{
public:
    FullInfoRequest(ICQClient *client, unsigned short id, unsigned long uin);
protected:
    virtual void fail(unsigned short error_code);
    bool answer(Buffer &b, unsigned short nSubtype);
    string unpack_list(Buffer &b);
    unsigned m_nParts;
    unsigned long m_uin;
    ICQClient *m_client;
};

FullInfoRequest::FullInfoRequest(ICQClient *client, unsigned short id, unsigned long uin)
        : ServerRequest(id)
{
    m_client = client;
    m_nParts = 0;
    m_uin = uin;
}

void FullInfoRequest::fail(unsigned short error_code)
{
    Contact *contact = NULL;
    if (m_nParts){
        if (m_client->data.owner.Uin == m_uin){
            Event e(EventClientChanged, m_client);
            e.process();
        }else{
            m_client->findContact(m_uin, NULL, false, contact);
            if (contact){
                Event e(EventContactChanged, contact);
                e.process();
            }
        }
    }
    if (contact){
        Event e(EventFetchInfoFail, contact);
        e.process();
    }
    if (error_code == ERROR_RATE_LIMIT){
        m_client->infoRequestPause();
    }else{
        m_client->removeFullInfoRequest(m_uin);
    }
}

string FullInfoRequest::unpack_list(Buffer &b)
{
    string res;
    char n;
    b >> n;
    for (; n > 0; n--){
        unsigned short c;
        b.unpack(c);
        string s;
        b >> s;
        if (c == 0) continue;
        if (res.length())
            res += ";";
        res += number(c);
        res += ",";
        res += quoteChars(s.c_str(), ";");
    }
    return res;
}

bool FullInfoRequest::answer(Buffer &b, unsigned short nSubtype)
{
    Contact *contact = NULL;
    ICQUserData *data;
    if (m_client->data.owner.Uin == m_uin){
        data = &m_client->data.owner;
    }else{
        data = m_client->findContact(m_uin, NULL, false, contact);
        if (data == NULL){
            log(L_DEBUG, "Info request %u not found", m_uin);
            m_client->removeFullInfoRequest(m_uin);
            return true;
        }
    }
    switch (nSubtype){
    case ICQ_SRVxGENERAL_INFO:{
            unsigned short n;
            char hideEmail;
            char TimeZone;
            b
            >> &data->Nick
            >> &data->FirstName
            >> &data->LastName
            >> &data->EMail
            >> &data->City
            >> &data->State
            >> &data->HomePhone
            >> &data->HomeFax
            >> &data->Address
            >> &data->PrivateCellular
            >> &data->Zip;
            b.unpack(n);
            data->Country = n;
            b
            >> TimeZone
            >> hideEmail;
            data->TimeZone = TimeZone;
            data->HiddenEMail = hideEmail;
            break;
        }
    case ICQ_SRVxMORE_INFO:{
            char c;
            b >> c;
            data->Age = c;
            b >> c;
            b >> c;
            data->Gender = c;
            b >> &data->Homepage;
            unsigned short year;
            b.unpack(year);
            data->BirthYear = year;
            b >> c;
            data->BirthMonth = c;
            b >> c;
            data->BirthDay = c;
            unsigned char lang[3];
            b
            >> lang[0]
            >> lang[1]
            >> lang[2];
            data->Language = (lang[2] << 16) + (lang[1] << 8) + lang[0];
            break;
        }
    case ICQ_SRVxEMAIL_INFO:{
            string mail;
            char c;
            b >> c;
            for (;c > 0;c--){
                char d;
                b >> d;
                string s;
                b >> s;
                s = quoteChars(s.c_str(), ";");
                if (mail.length())
                    mail += ";";
                mail += s;
                mail += '/';
                if (d)
                    mail += '-';
            }
            set_str(&data->EMails, mail.c_str());
            break;
        }
    case ICQ_SRVxWORK_INFO:{
            unsigned short n;
            b
            >> &data->WorkCity
            >> &data->WorkState
            >> &data->WorkPhone
            >> &data->WorkFax
            >> &data->WorkAddress
            >> &data->WorkZip;
            b.unpack(n);
            data->WorkCountry = n;
            b
            >> &data->WorkName
            >> &data->WorkDepartment
            >> &data->WorkPosition;
            b.unpack(n);
            data->Occupation = n;
            b >> &data->WorkHomepage;
            break;
        }
    case ICQ_SRVxABOUT_INFO:
        b >> &data->About;
        break;
    case ICQ_SRVxINTERESTS_INFO:
        set_str(&data->Interests, unpack_list(b).c_str());
        break;
    case ICQ_SRVxBACKGROUND_INFO:
        set_str(&data->Backgrounds, unpack_list(b).c_str());
        set_str(&data->Affilations, unpack_list(b).c_str());
        break;
    case ICQ_SRVxUNKNOWN_INFO:
        break;
    default:
        log(L_WARN, "Unknwon info type %04X for %lu", nSubtype, m_uin);
    }
    m_nParts++;
    if (m_nParts >= 8){
        data->InfoFetchTime = data->InfoUpdateTime ? data->InfoUpdateTime : 1;
        if (contact != NULL){
            m_client->setupContact(contact, data);
            Event e(EventContactChanged, contact);
            e.process();
        }else{
            int tz;
#ifndef HAVE_TM_GMTOFF
            tz = - _timezone;
#else
            time_t now;
            time(&now);
            struct tm *tm = localtime(&now);
            tz = tm->tm_gmtoff;
            if (tm->tm_isdst) tz -= (60 * 60);
#endif
            tz = - tz / (30 * 60);
            if (data->TimeZone != (unsigned)tz){
                data->TimeZone = tz;
                m_client->setMainInfo(data);
            }
            m_client->setupContact(getContacts()->owner(), data);
            Event eContact(EventContactChanged, getContacts()->owner());
            eContact.process();
            Event e(EventClientChanged, m_client);
            e.process();
        }
        m_client->removeFullInfoRequest(m_uin);
        return true;
    }
    return false;
}

void ICQClient::infoRequest()
{
    m_infoTimer->stop();
    if ((getState() != Connected) || infoRequests.empty() || !m_bServerReady)
        return;
    if (m_infoRequestId == PAUSE_ID)
        return;
    unsigned long uin = infoRequests.front();
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << ((uin == data.owner.Uin) ? ICQ_SRVxREQ_OWN_INFO : ICQ_SRVxREQ_FULL_INFO);
    m_socket->writeBuffer.pack(uin);
    sendServerRequest();
    m_infoTimer->start(INFO_REQUEST_TIMEOUT * 1000);
    m_infoRequestId = m_nMsgSequence;
    varRequests.push_back(new FullInfoRequest(this, m_infoRequestId, uin));
}

void ICQClient::infoRequestFail()
{
    m_infoTimer->stop();
    if (m_infoRequestId == PAUSE_ID){
        m_infoRequestId = 0;
        infoRequest();
        return;
    }
    ServerRequest *req = findServerRequest(m_infoRequestId);
    if (req)
        req->fail();
}

void ICQClient::infoRequestPause()
{
    m_infoRequestId = PAUSE_ID;
    m_infoTimer->stop();
    m_infoTimer->start(INFO_PAUSE_TIMEOUT * 1000);
}

void ICQClient::addFullInfoRequest(unsigned long uin, bool bInLast)
{
    bool bCanStart = infoRequests.empty();
    if (bInLast){
        list<unsigned long>::iterator it;
        for (it = infoRequests.begin(); it != infoRequests.end(); ++it){
            if ((*it) == uin)
                return;
        }
        infoRequests.push_back(uin);
    }else{
        if (!infoRequests.empty() && (infoRequests.front() == uin))
            return;
        list<unsigned long>::iterator it;
        for (it = infoRequests.begin(); it != infoRequests.end(); ++it){
            if ((*it) == uin){
                infoRequests.erase(it);
                break;
            }
        }
        infoRequests.push_front(uin);
    }
    if (bCanStart)
        infoRequest();
}

void ICQClient::removeFullInfoRequest(unsigned long uin)
{
    m_infoTimer->stop();
    list<unsigned long>::iterator it;
    for (it = infoRequests.begin(); it != infoRequests.end(); ++it){
        if ((*it) == uin){
            infoRequests.erase(it);
            break;
        }
    }
    if (infoRequests.empty())
        return;
    QTimer::singleShot(1000, this, SLOT(infoRequest()));
}

// _________________________________________________________________________________________

class SearchWPRequest : public ServerRequest
{
public:
    SearchWPRequest(ICQClient *client, unsigned short id);
protected:
    virtual void fail(unsigned short error_code);
    bool answer(Buffer &b, unsigned short nSubtype);
    ICQClient *m_client;
};

SearchWPRequest::SearchWPRequest(ICQClient *client, unsigned short id)
        : ServerRequest(id)
{
    m_client = client;
}

void SearchWPRequest::fail(unsigned short)
{
    SearchResult res;
    res.id = m_id;
    res.client = m_client;
    load_data(ICQProtocol::icqUserData, &res.data, NULL);
    Event e(static_cast<ICQPlugin*>(m_client->protocol()->plugin())->EventSearchDone, &res);
    e.process();
    free_data(ICQProtocol::icqUserData, &res.data);
}

bool SearchWPRequest::answer(Buffer &b, unsigned short nSubType)
{
    SearchResult res;
    res.id = m_id;
    res.client = m_client;
    load_data(ICQProtocol::icqUserData, &res.data, NULL);

    unsigned short n;
    b >> n;
    b.unpack(res.data.Uin);
    char waitAuth;
    char state;
    b
    >> &res.data.Nick
    >> &res.data.FirstName
    >> &res.data.LastName
    >> &res.data.EMail
    >> waitAuth
    >> state;

    if (waitAuth)
        res.data.WaitAuth = true;
    switch (state){
    case SEARCH_STATE_OFFLINE:
        res.data.Status = STATUS_OFFLINE;
        break;
    case SEARCH_STATE_ONLINE:
        res.data.Status = STATUS_ONLINE;
        break;
    case SEARCH_STATE_DISABLED:
        res.data.Status = STATUS_UNKNOWN;
        break;
    }

    Event e(static_cast<ICQPlugin*>(m_client->protocol()->plugin())->EventSearch, &res);
    e.process();
    free_data(ICQProtocol::icqUserData, &res.data);

    if (nSubType == 0xAE01){
        load_data(ICQProtocol::icqUserData, &res.data, NULL);
        Event e(static_cast<ICQPlugin*>(m_client->protocol()->plugin())->EventSearchDone, &res);
        e.process();
        free_data(ICQProtocol::icqUserData, &res.data);
        return true;
    }
    return false;
}

unsigned short ICQClient::findByUin(unsigned long uin)
{
    if ((getState() != Connected) || !m_bServerReady)
        return (unsigned short)(-1);
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer
    << ICQ_SRVxREQ_WP_INFO_UIN
    << 0x36010400L;
    m_socket->writeBuffer.pack(uin);
    sendServerRequest();
    varRequests.push_back(new SearchWPRequest(this, m_nMsgSequence));
    return m_nMsgSequence;
}

unsigned short ICQClient::findWP(const char *szFirst, const char *szLast, const char *szNick,
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
    if ((getState() != Connected) || !m_bServerReady)
        return (unsigned short)(-1);
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << ICQ_SRVxREQ_WP_FULL;

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

    m_socket->writeBuffer
    << sFirst
    << sLast
    << sNick
    << sEmail;
    m_socket->writeBuffer.pack(nMinAge);
    m_socket->writeBuffer.pack(nMaxAge);
    m_socket->writeBuffer
    << nGender
    << nLanguage
    << sCity
    << sState;
    m_socket->writeBuffer.pack(nCountryCode);
    m_socket->writeBuffer
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
    m_socket->writeBuffer << c;

    sendServerRequest();

    varRequests.push_back(new SearchWPRequest(this, m_nMsgSequence));
    return m_nMsgSequence;
}

// ______________________________________________________________________________________

class SetMainInfoRequest : public ServerRequest
{
public:
    SetMainInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string m_nick;
    string m_firstName;
    string m_lastName;
    string m_city;
    string m_state;
    string m_address;
    string m_zip;
    string m_email;
    string m_homePhone;
    string m_homeFax;
    string m_privateCellular;
    unsigned m_hiddenEMail;
    unsigned m_country;
    unsigned m_tz;
    ICQClient *m_client;
};

SetMainInfoRequest::SetMainInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    if (data->Nick)
        m_nick = data->Nick;
    if (data->FirstName)
        m_firstName = data->FirstName;
    if (data->LastName)
        m_lastName = data->LastName;
    if (data->City)
        m_city = data->City;
    if (data->State)
        m_state = data->State;
    if (data->Address)
        m_address = data->Address;
    if (data->Zip)
        m_zip = data->Zip;
    if (data->EMail)
        m_email = data->EMail;
    if (data->HomePhone)
        m_homePhone = data->HomePhone;
    if (data->HomeFax)
        m_homeFax = data->HomeFax;
    if (data->PrivateCellular)
        m_privateCellular = data->PrivateCellular;
    m_country = data->Country;
    m_tz = data->TimeZone;
    m_hiddenEMail = data->HiddenEMail;
}

bool SetMainInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.Nick, m_nick.c_str());
    set_str(&m_client->data.owner.FirstName, m_firstName.c_str());
    set_str(&m_client->data.owner.LastName, m_lastName.c_str());
    set_str(&m_client->data.owner.City, m_city.c_str());
    set_str(&m_client->data.owner.State, m_state.c_str());
    set_str(&m_client->data.owner.Address, m_address.c_str());
    set_str(&m_client->data.owner.Zip, m_zip.c_str());
    set_str(&m_client->data.owner.EMail, m_email.c_str());
    set_str(&m_client->data.owner.HomePhone, m_homePhone.c_str());
    set_str(&m_client->data.owner.HomeFax, m_homeFax.c_str());
    set_str(&m_client->data.owner.PrivateCellular, m_privateCellular.c_str());
    m_client->data.owner.Country = m_country;
    m_client->data.owner.TimeZone = m_tz;
    m_client->data.owner.HiddenEMail = m_hiddenEMail;
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetWorkInfoRequest : public ServerRequest
{
public:
    SetWorkInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string m_workCity;
    string m_workState;
    string m_workAddress;
    string m_workZip;
    unsigned m_workCountry;
    string m_workName;
    string m_workDepartment;
    string m_workPosition;
    string m_workPhone;
    string m_workFax;
    unsigned m_occupation;
    string m_workHomepage;
    ICQClient *m_client;
};

SetWorkInfoRequest::SetWorkInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    if (data->WorkCity)
        m_workCity = data->WorkCity;
    if (data->WorkState)
        m_workState = data->WorkState;
    if (data->WorkAddress)
        m_workAddress = data->WorkAddress;
    if (data->WorkZip)
        m_workZip = data->WorkZip;
    m_workCountry = data->WorkCountry;
    if (data->WorkName)
        m_workName = data->WorkName;
    if (data->WorkDepartment)
        m_workDepartment = data->WorkDepartment;
    if (data->WorkPosition)
        m_workPosition = data->WorkPosition;
    m_occupation = data->Occupation;
    if (data->WorkHomepage)
        m_workHomepage = data->WorkHomepage;
    if (data->WorkPhone)
        m_workPhone = data->WorkPhone;
    if (data->WorkFax)
        m_workFax = data->WorkFax;
}

bool SetWorkInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.WorkCity, m_workCity.c_str());
    set_str(&m_client->data.owner.WorkState, m_workState.c_str());
    set_str(&m_client->data.owner.WorkAddress, m_workAddress.c_str());
    set_str(&m_client->data.owner.WorkZip, m_workZip.c_str());
    set_str(&m_client->data.owner.WorkName, m_workName.c_str());
    set_str(&m_client->data.owner.WorkDepartment, m_workDepartment.c_str());
    set_str(&m_client->data.owner.WorkPosition, m_workPosition.c_str());
    set_str(&m_client->data.owner.WorkHomepage, m_workHomepage.c_str());
    set_str(&m_client->data.owner.WorkPhone, m_workPhone.c_str());
    set_str(&m_client->data.owner.WorkFax, m_workFax.c_str());
    m_client->data.owner.WorkCountry = m_workCountry;
    m_client->data.owner.Occupation = m_occupation;
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetMoreInfoRequest : public ServerRequest
{
public:
    SetMoreInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    unsigned m_age;
    unsigned m_gender;
    unsigned m_birthYear;
    unsigned m_birthMonth;
    unsigned m_birthDay;
    unsigned m_language;
    string   m_homepage;
    ICQClient *m_client;
};

SetMoreInfoRequest::SetMoreInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    m_age = data->Age;
    m_gender = data->Gender;
    m_birthYear = data->BirthYear;
    m_birthMonth = data->BirthMonth;
    m_birthDay = data->BirthDay;
    m_language = data->Language;
    string   m_homepage;
    if (data->Homepage)
        m_homepage = data->Homepage;
}

bool SetMoreInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.Homepage, m_homepage.c_str());
    m_client->data.owner.Age = m_age;
    m_client->data.owner.Gender = m_gender;
    m_client->data.owner.BirthYear = m_birthYear;
    m_client->data.owner.BirthMonth = m_birthMonth;
    m_client->data.owner.BirthDay = m_birthDay;
    m_client->data.owner.Language = m_language;
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetAboutInfoRequest : public ServerRequest
{
public:
    SetAboutInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string   m_about;
    ICQClient *m_client;
};

SetAboutInfoRequest::SetAboutInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    if (data->About)
        m_about = data->About;
}

bool SetAboutInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.About, m_about.c_str());
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetMailInfoRequest : public ServerRequest
{
public:
    SetMailInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string   m_emails;
    ICQClient *m_client;
};

SetMailInfoRequest::SetMailInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    if (data->EMails)
        m_emails = data->EMails;
}

bool SetMailInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.EMails, m_emails.c_str());
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetInterestsInfoRequest : public ServerRequest
{
public:
    SetInterestsInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string   m_interests;
    ICQClient *m_client;
};

SetInterestsInfoRequest::SetInterestsInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    if (data->Interests)
        m_interests = data->Interests;
}

bool SetInterestsInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.Interests, m_interests.c_str());
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetBackgroundsInfoRequest : public ServerRequest
{
public:
    SetBackgroundsInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string   m_backgrounds;
    string   m_affilations;
    ICQClient *m_client;
};

SetBackgroundsInfoRequest::SetBackgroundsInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    if (data->Backgrounds)
        m_backgrounds = data->Backgrounds;
    if (data->Affilations)
        m_affilations = data->Affilations;
}

bool SetBackgroundsInfoRequest::answer(Buffer&, unsigned short)
{
    set_str(&m_client->data.owner.Backgrounds, m_backgrounds.c_str());
    set_str(&m_client->data.owner.Affilations, m_affilations.c_str());
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

class SetSecurityInfoRequest : public ServerRequest
{
public:
    SetSecurityInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    bool m_bWebAware;
    bool m_bWaitAuth;
    ICQClient *m_client;
};

SetSecurityInfoRequest::SetSecurityInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client  = client;
    m_bWebAware = data->WebAware;
    m_bWaitAuth = data->WaitAuth;
}

bool SetSecurityInfoRequest::answer(Buffer&, unsigned short)
{
    if ((m_client->data.owner.WebAware != 0) != m_bWebAware){
        m_client->data.owner.WebAware = m_bWebAware;
        m_client->sendStatus();
    }
    m_client->data.owner.WaitAuth = m_bWaitAuth;
    Event e(EventClientChanged, m_client);
    e.process();
    return true;
}

void ICQClient::setMainInfo(ICQUserData *d)
{
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << ICQ_SRVxREQ_MODIFY_MAIN
    << &d->Nick
    << &d->FirstName
    << &d->LastName
    << &d->EMail
    << &d->City
    << &d->State
    << &d->HomePhone
    << &d->HomeFax
    << &d->Address
    << &d->PrivateCellular
    << &d->Zip;
    m_socket->writeBuffer.pack((unsigned short)(d->Country));
    m_socket->writeBuffer.pack((char)(d->TimeZone));
    m_socket->writeBuffer.pack((char)(d->HiddenEMail ? 1 : 0));
    sendServerRequest();

    varRequests.push_back(new SetMainInfoRequest(this, m_nMsgSequence, d));
}

void ICQClient::packInfoList(char *str)
{
    list<unsigned short> category;
    list<string> spec;

    if (str){
        string s = str;
        while (s.length()){
            string item = getToken(s, ';');
            string cat = getToken(item, ',');
            category.push_back(atol(cat.c_str()));
            spec.push_back(item);
        }
    }

    char n = category.size();
    m_socket->writeBuffer << n;

    list<unsigned short>::iterator itc = category.begin();
    list<string>::iterator its = spec.begin();

    for (; itc != category.end(); ++itc, ++its){
        m_socket->writeBuffer.pack(*itc);
        m_socket->writeBuffer << *its;
    }
}

void ICQClient::setClientInfo(void *_data)
{
    if (getState() != Connected)
        return;

    ICQUserData *d = (ICQUserData*)_data;

    set_str(&d->HomePhone, NULL);
    set_str(&d->HomeFax, NULL);
    set_str(&d->WorkPhone, NULL);
    set_str(&d->WorkFax, NULL);
    set_str(&d->PrivateCellular, NULL);
    set_str(&d->EMail, NULL);
    set_str(&d->EMails, NULL);

    QString phones = getContacts()->owner()->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        QString phoneValue = getToken(phoneItem, '/', false);
        if (phoneItem.length())
            continue;
        QString number = getToken(phoneValue, ',');
        QString type = getToken(phoneValue, ',');
        if (type == "Home Phone"){
            set_str(&d->HomePhone, fromUnicode(number, NULL).c_str());
        }else if (type == "Home Fax"){
            set_str(&d->HomeFax, fromUnicode(number, NULL).c_str());
        }else if (type == "Work Phone"){
            set_str(&d->WorkPhone, fromUnicode(number, NULL).c_str());
        }else if (type == "Work Fax"){
            set_str(&d->WorkFax, fromUnicode(number, NULL).c_str());
        }else if (type == "Private Cellular"){
            number += " SMS";
            set_str(&d->PrivateCellular, fromUnicode(number, NULL).c_str());
        }
    }
    d->HiddenEMail = false;
    QString mails = getContacts()->owner()->getEMails();
    string s;
    while (mails.length()){
        QString mailItem = getToken(mails, ';', false);
        QString mail = getToken(mailItem, '/');
        if (!s.empty())
            s += ';';
        s += fromUnicode(mail, NULL);
        s += '/';
        if (mailItem.length())
            s += '-';
        if (d->EMail == NULL){
            set_str(&d->EMail, fromUnicode(mail, NULL).c_str());
            d->HiddenEMail = !mailItem.isEmpty();
        }
    }
    set_str(&d->EMails, s.c_str());

    if ((d->Country != data.owner.Country) ||
            (d->HiddenEMail != data.owner.HiddenEMail) ||
            cmp(d->Nick, data.owner.Nick) ||
            cmp(d->FirstName, data.owner.FirstName) ||
            cmp(d->LastName, data.owner.LastName) ||
            cmp(d->EMail, data.owner.EMail) ||
            cmp(d->HomePhone, data.owner.HomePhone) ||
            cmp(d->HomeFax, data.owner.HomeFax) ||
            cmp(d->PrivateCellular, data.owner.PrivateCellular) ||
            cmp(d->City, data.owner.City) ||
            cmp(d->State, data.owner.State) ||
            cmp(d->Address, data.owner.Address) ||
            cmp(d->Zip, data.owner.Zip)){
        setMainInfo(d);
        m_nUpdates++;
    }

    if ((d->WorkCountry != data.owner.WorkCountry) ||
            (d->Occupation != data.owner.Occupation) ||
            cmp(d->WorkCity, data.owner.WorkCity) ||
            cmp(d->WorkState, data.owner.WorkState) ||
            cmp(d->WorkAddress, data.owner.WorkAddress) ||
            cmp(d->WorkZip, data.owner.WorkZip) ||
            cmp(d->WorkName, data.owner.WorkName) ||
            cmp(d->WorkDepartment, data.owner.WorkDepartment) ||
            cmp(d->WorkPosition, data.owner.WorkPosition) ||
            cmp(d->WorkPhone, data.owner.WorkPhone) ||
            cmp(d->WorkFax, data.owner.WorkFax) ||
            cmp(d->WorkHomepage, data.owner.WorkHomepage)){

        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer
        << ICQ_SRVxREQ_MODIFY_WORK
        << &d->WorkCity
        << &d->WorkState
        << &d->WorkPhone
        << &d->WorkFax
        << &d->WorkAddress
        << &d->WorkZip;
        m_socket->writeBuffer.pack((unsigned short)d->WorkCountry);
        m_socket->writeBuffer
        << &d->WorkName
        << &d->WorkDepartment
        << &d->WorkPosition;
        m_socket->writeBuffer.pack((char)d->Occupation);
        m_socket->writeBuffer
        << &d->WorkHomepage;
        sendServerRequest();
        varRequests.push_back(new SetWorkInfoRequest(this, m_nMsgSequence, d));
        m_nUpdates++;
    }
    if ((d->Age != data.owner.Age) ||
            (d->Gender != data.owner.Gender) ||
            (d->BirthYear != data.owner.BirthYear) ||
            (d->BirthMonth != data.owner.BirthMonth) ||
            (d->BirthDay != data.owner.BirthDay) ||
            (d->Language != data.owner.Language) ||
            cmp(d->Homepage, data.owner.Homepage)){

        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer << ICQ_SRVxREQ_MODIFY_MORE
        << (char)(d->Age)
        << (char)0
        << (char)(d->Gender)
        << &d->Homepage;
        m_socket->writeBuffer.pack((unsigned short)d->BirthYear);
        m_socket->writeBuffer
        << (char)(d->BirthMonth)
        << (char)(d->BirthDay)
        << (char)(d->Language & 0xFF)
        << (char)((d->Language >> 8) & 0xFF)
        << (char)((d->Language >> 16) & 0xFF);
        sendServerRequest();
        varRequests.push_back(new SetMoreInfoRequest(this, m_nMsgSequence, d));
        m_nUpdates++;
    }

    if (cmp(d->About, data.owner.About)){
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer   << ICQ_SRVxREQ_MODIFY_ABOUT
        << &d->About;
        sendServerRequest();
        varRequests.push_back(new SetAboutInfoRequest(this, m_nMsgSequence, d));
        m_nUpdates++;
    }

    if (cmp(d->EMails, data.owner.EMails)){
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer   << ICQ_SRVxREQ_MODIFY_MAIL;
        string emails;
        list<string> mails;
        list<bool> hiddens;
        if (d->EMails){
            emails = d->EMails;
            while (emails.length()){
                string mailItem = getToken(emails, ';', false);
                string mail = getToken(mailItem, '/');
                mails.push_back(mail);
                hiddens.push_back(mailItem.length());
            }
        }
        m_socket->writeBuffer << (char)(mails.size());
        list<string>::iterator its = mails.begin();
        list<bool>::iterator ith = hiddens.begin();
        for (; its != mails.end(); ++its, ++ith){
            char hide = (*ith) ? 1 : 0;
            m_socket->writeBuffer << hide << (*its);
        }
        sendServerRequest();
        varRequests.push_back(new SetMailInfoRequest(this, m_nMsgSequence, d));
        m_nUpdates++;
    }

    if (cmp(d->Interests, data.owner.Interests)){
        varRequests.push_back(new SetInterestsInfoRequest(this, m_nMsgSequence, d));
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer << ICQ_SRVxREQ_MODIFY_INTERESTS;
        packInfoList(d->Interests);
        sendServerRequest();
        varRequests.push_back(new SetInterestsInfoRequest(this, m_nMsgSequence, d));
        m_nUpdates++;
    }

    if (cmp(d->Backgrounds, data.owner.Backgrounds) ||
            cmp(d->Affilations, data.owner.Affilations)){
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer << ICQ_SRVxREQ_MODIFY_BACKGROUND;
        packInfoList(d->Backgrounds);
        packInfoList(d->Affilations);
        sendServerRequest();
        varRequests.push_back(new SetBackgroundsInfoRequest(this, m_nMsgSequence, d));
        m_nUpdates++;
    }

    if (((d->WaitAuth != 0) != (data.owner.WaitAuth != 0)) ||
            ((d->WebAware != 0) != (data.owner.WebAware != 0))){
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer  << ICQ_SRVxREQ_PERMISSIONS
        << (char)(d->WaitAuth ? 0 : 0x01)
        << (char)(d->WebAware ? 0x01 : 0)
        << (char)0x02
        << (char)0;
        sendServerRequest();
        varRequests.push_back(new SetSecurityInfoRequest(this, m_nMsgSequence, d));
    }
    setChatGroup();
    sendStatus();
}

class SetPasswordRequest : public ServerRequest
{
public:
    SetPasswordRequest(ICQClient *client, unsigned short id, const char *pwd);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    string m_pwd;
    ICQClient *m_client;
};

SetPasswordRequest::SetPasswordRequest(ICQClient *client, unsigned short id, const char *pwd)
        : ServerRequest(id)
{
    m_client  = client;
    m_pwd     = pwd;
}

bool SetPasswordRequest::answer(Buffer&, unsigned short)
{
    m_client->setPassword(QString::fromUtf8(m_pwd.c_str()));
    return true;
}

void ICQClient::changePassword(const char *new_pswd)
{
    QString pwd = QString::fromUtf8(new_pswd);
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer
    << ICQ_SRVxREQ_CHANGE_PASSWD
    << fromUnicode(pwd, NULL);
    sendServerRequest();
    varRequests.push_back(new SetPasswordRequest(this, m_nMsgSequence, new_pswd));
}

class SMSRequest : public ServerRequest
{
public:
    SMSRequest(ICQClient *client, unsigned short id);
    virtual bool answer(Buffer&, unsigned short nSubType);
    virtual void fail(unsigned short error_code);
protected:
    ICQClient *m_client;
};

#if 0
const char *translations[] =
    {
        I18N_NOOP("The Cellular network is currently unable to send your message to the recipient. Please try again later."),
        I18N_NOOP("INVALID NUMBER"),
        I18N_NOOP("RATE LIMIT")
    };
#endif

SMSRequest::SMSRequest(ICQClient *client, unsigned short id)
        : ServerRequest(id)
{
    m_client = client;
}

bool SMSRequest::answer(Buffer &b, unsigned short code)
{
    m_client->m_sendSmsId = 0;
    if (code == 0x0100){
        if (m_client->smsQueue.empty())
            return true;
        string errStr = b.data(b.readPos());
        SendMsg &s = m_client->smsQueue.front();
        SMSMessage *sms = static_cast<SMSMessage*>(s.msg);
        m_client->smsQueue.erase(m_client->smsQueue.begin());
        sms->setError(errStr.c_str());
        Event e(EventMessageSent, sms);
        e.process();
        delete sms;
    }else{
        b.incReadPos(6);
        string provider;
        string answer;
        b.unpackStr(provider);
        b.unpackStr(answer);

        string::iterator s = answer.begin();
        auto_ptr<XmlNode> top(XmlNode::parse(s, answer.end()));
        string error = I18N_NOOP("SMS send fail");
        string network;
        if (top.get()){
            XmlNode *n = top.get();
            if (n && n->isBranch()){
                XmlBranch *msg = static_cast<XmlBranch*>(n);
                XmlLeaf *l = msg->getLeaf("deliverable");
                if (l && (l->getValue() == "Yes")){
                    error = "";
                    l = msg->getLeaf("network");
                    if (l)
                        network = l->getValue();
                }else{
                    XmlBranch *param = msg->getBranch("param");
                    if (param){
                        XmlLeaf *l = param->getLeaf("error");
                        if (l)
                            error = l->getValue();
                    }
                }
            }
        }

        if (error.empty()){
            if (!m_client->smsQueue.empty()){
                SendMsg &s = m_client->smsQueue.front();
                SMSMessage *sms = static_cast<SMSMessage*>(s.msg);
                sms->setNetwork(network.c_str());
                if ((sms->getFlags() & MESSAGE_NOHISTORY) == 0){
                    SMSMessage m;
                    m.setContact(sms->contact());
                    m.setText(s.part);
                    m.setPhone(sms->getPhone());
                    m.setNetwork(network.c_str());
                    Event e(EventSent, &m);
                    e.process();
                }
            }
        }else{
            if (!m_client->smsQueue.empty()){
                SendMsg &s = m_client->smsQueue.front();
                s.msg->setError(error.c_str());
                Event e(EventMessageSent, s.msg);
                e.process();
                delete s.msg;
                m_client->smsQueue.erase(m_client->smsQueue.begin());
            }
        }
    }
    m_client->processSMSQueue();
    return true;
}

void SMSRequest::fail(unsigned short)
{
    if (m_client->smsQueue.empty())
        return;
    SendMsg &s = m_client->smsQueue.front();
    Message *sms = s.msg;
    sms->setError(I18N_NOOP("SMS send fail"));
    m_client->smsQueue.erase(m_client->smsQueue.begin());
    Event e(EventMessageSent, sms);
    e.process();
    delete sms;
    m_client->m_sendSmsId = 0;
    m_client->processSMSQueue();
}

const unsigned MAX_SMS_LEN_LATIN1	= 160;
const unsigned MAX_SMS_LEN_UNICODE	= 70;

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

void ICQClient::processSMSQueue()
{
    if (m_sendSmsId)
        return;
    for (;;){
        if (smsQueue.empty())
            break;
        SendMsg &s = smsQueue.front();
        if (s.text.isEmpty() || (!(s.flags & SEND_1STPART) && (s.msg->getFlags() & MESSAGE_1ST_PART))){
            Event e(EventMessageSent, s.msg);
            e.process();
            delete s.msg;
            smsQueue.erase(smsQueue.begin());
            continue;
        }
        SMSMessage *sms = static_cast<SMSMessage*>(s.msg);
        QString text = s.text;
        QString part = getPart(text, MAX_SMS_LEN_LATIN1);
        if (!isLatin(part)){
            text = s.text;
            part = getPart(text, MAX_SMS_LEN_UNICODE);
        }
        s.text = text;
        s.part = part;

        string	nmb = "+";
        QString	phone = sms->getPhone();
        for (int i = 0; i < (int)(phone.length()); i++){
            char c = phone[i].latin1();
            if ((c >= '0') && (c <= '9'))
                nmb += c;
        }
        XmlBranch xmltree("icq_sms_message");
        xmltree.pushnode(new XmlLeaf("destination",nmb));
        xmltree.pushnode(new XmlLeaf("text",(const char*)(part.utf8())));
        xmltree.pushnode(new XmlLeaf("codepage","1252"));
        xmltree.pushnode(new XmlLeaf("encoding","utf8"));
        xmltree.pushnode(new XmlLeaf("senders_UIN",number(data.owner.Uin).c_str()));
        xmltree.pushnode(new XmlLeaf("senders_name",""));
        xmltree.pushnode(new XmlLeaf("delivery_receipt","Yes"));

        char timestr[30];
        time_t t;
        struct tm *tm;
        time(&t);
        tm = gmtime(&t);
        snprintf(timestr, sizeof(timestr), "%s, %02u %s %04u %02u:%02u:%02u GMT",
                 w_days[tm->tm_wday], tm->tm_mday, months[tm->tm_mon], tm->tm_year + 1900,
                 tm->tm_hour, tm->tm_min, tm->tm_sec);
        xmltree.pushnode(new XmlLeaf("time",string(timestr)));
        string msg = xmltree.toString(0);

        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer << ICQ_SRVxREQ_SEND_SMS
        << 0x00010016L << 0x00000000L << 0x00000000L
        << 0x00000000L << 0x00000000L << (unsigned long)(msg.size());
        m_socket->writeBuffer << msg.c_str();
        sendServerRequest();
        varRequests.push_back(new SMSRequest(this, m_nMsgSequence));
        m_sendSmsId = m_nMsgSequence;
        return;
    }
}

void ICQClient::clearSMSQueue()
{
    for (list<SendMsg>::iterator it = smsQueue.begin(); it != smsQueue.end(); ++it){
        (*it).msg->setError(I18N_NOOP("Client go offline"));
        Event e(EventMessageSent, (*it).msg);
        e.process();
        delete (*it).msg;
    }
    smsQueue.clear();
    m_sendSmsId = 0;
}

void ICQClient::setChatGroup()
{
    if ((getState() != Connected) || (getRandomChatGroup() == getRandomChatGroupCurrent()))
        return;
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << (unsigned short)ICQ_SRVxREQ_SET_CHAT_GROUP;
    if (getRandomChatGroup()){
        m_socket->writeBuffer.pack((unsigned short)getRandomChatGroup());
        m_socket->writeBuffer
        << 0x00000310L
        << 0x00000000L
        << 0x00000000L
        << 0x00000000L
        << (char)4
        << (char)ICQ_TCP_VERSION
        << 0x00000000L
        << 0x00000050L
        << 0x00000003L
        << (unsigned short)0
        << (char)0;
    }else{
        m_socket->writeBuffer << (unsigned short)0;
    }
    sendServerRequest();
    setRandomChatGroupCurrent(getRandomChatGroup());
}

class RandomChatRequest : public ServerRequest
{
public:
    RandomChatRequest(ICQClient *client, unsigned short id);
protected:
    virtual void fail(unsigned short error_code);
    bool answer(Buffer &b, unsigned short nSubtype);
    ICQClient *m_client;
};


RandomChatRequest::RandomChatRequest(ICQClient *client, unsigned short id)
        : ServerRequest(id)
{
    m_client = client;
}

bool RandomChatRequest::answer(Buffer &b, unsigned short)
{
    unsigned long uin;
    b.unpack(uin);
    Event e(static_cast<ICQPlugin*>(m_client->protocol()->plugin())->EventRandomChat, (void*)uin);
    e.process();
    return true;
}

void RandomChatRequest::fail(unsigned short)
{
    Event e(static_cast<ICQPlugin*>(m_client->protocol()->plugin())->EventRandomChat, NULL);
    e.process();
}

void ICQClient::searchChat(unsigned short group)
{
    if (getState() != Connected){
        Event e(static_cast<ICQPlugin*>(protocol()->plugin())->EventRandomChat, NULL);
        e.process();
        return;
    }
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << (unsigned short)ICQ_SRVxREQ_RANDOM_CHAT;
    m_socket->writeBuffer.pack(group);
    sendServerRequest();
    varRequests.push_back(new RandomChatRequest(this, m_nMsgSequence));
}




