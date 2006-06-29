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
#include <stdio.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <qtimer.h>
#include "xml.h"

using namespace std;
using namespace SIM;

const unsigned short ICQ_SNACxVAR_ERROR             = 0x0001;
const unsigned short ICQ_SNACxVAR_REQxSRV           = 0x0002;
const unsigned short ICQ_SNACxVAR_DATA              = 0x0003;

const unsigned short ICQ_SRVxREQ_OFFLINE_MSG        = 0x3C00;
const unsigned short ICQ_SRVxREQ_ACK_OFFLINE_MSG    = 0x3E00;
const unsigned short ICQ_SRVxOFFLINE_MSG            = 0x4100;
const unsigned short ICQ_SRVxEND_OFFLINE_MSG        = 0x4200;
const unsigned short ICQ_SRVxREQ_MORE               = 0xD007;
const unsigned short ICQ_SRVxANSWER_MORE            = 0xDA07;

const unsigned short ICQ_SRVxREQ_FULL_INFO          = 0xB204;
const unsigned short ICQ_SRVxREQ_SHORT_INFO         = 0xBA04;
const unsigned short ICQ_SRVxREQ_OWN_INFO           = 0xD004;
const unsigned short ICQ_SRVxREQ_SEND_SMS           = 0x8214;
const unsigned short ICQ_SRVxREQ_WP_UIN             = 0x6905;
const unsigned short ICQ_SRVxREQ_WP_MAIL            = 0x7305;
const unsigned short ICQ_SRVxREQ_WP_FULL            = 0x5F05;
const unsigned short ICQ_SRVxREQ_CHANGE_PASSWD      = 0x2E04;
const unsigned short ICQ_SRVxREQ_PERMISSIONS        = 0x2404;
const unsigned short ICQ_SRVxREQ_XML_KEY            = 0x9808;

const unsigned short ICQ_SRVxGENERAL_INFO           = 0xC800;
const unsigned short ICQ_SRVxMORE_INFO              = 0xDC00;
const unsigned short ICQ_SRVxEMAIL_INFO             = 0xEB00;
const unsigned short ICQ_SRVxWORK_INFO              = 0xD200;
const unsigned short ICQ_SRVxABOUT_INFO             = 0xE600;
const unsigned short ICQ_SRVxINTERESTS_INFO         = 0xF000;
const unsigned short ICQ_SRVxBACKGROUND_INFO        = 0xFA00;
const unsigned short ICQ_SRVxUNKNOWN_INFO           = 0x0E01;

const unsigned short ICQ_SRVxREQ_MODIFY_MAIN        = 0xEA03;
const unsigned short ICQ_SRVxREQ_MODIFY_HOME        = 0xFD03;
const unsigned short ICQ_SRVxREQ_MODIFY_ABOUT       = 0x0604;
const unsigned short ICQ_SRVxREQ_MODIFY_WORK        = 0xF303;
const unsigned short ICQ_SRVxREQ_MODIFY_MORE        = 0xFD03;
const unsigned short ICQ_SRVxREQ_MODIFY_INTERESTS   = 0x1004;
const unsigned short ICQ_SRVxREQ_MODIFY_BACKGROUND  = 0x1A04;
const unsigned short ICQ_SRVxREQ_MODIFY_MAIL        = 0x0B04;

const unsigned short ICQ_SRVxREQ_PHONE_UPDATE       = 0x5406;
const unsigned short ICQ_SRVxREQ_SET_CHAT_GROUP     = 0x5807;
const unsigned short ICQ_SRVxREQ_RANDOM_CHAT        = 0x4E07;

const unsigned short ICQ_SRVxCLI_SET_FULLINFO       = 0x3A0C;

const unsigned short TLV_UIN                        = 0x0136;
const unsigned short TLV_FIRST_NAME                 = 0x0140;
const unsigned short TLV_LAST_NAME                  = 0x014A;
const unsigned short TLV_NICK                       = 0x0154;
const unsigned short TLV_EMAIL                      = 0x015E;
const unsigned short TLV_AGE_RANGE                  = 0x0168;
const unsigned short TLV_AGE                        = 0x0172;
const unsigned short TLV_GENDER                     = 0x017C;
const unsigned short TLV_LANGUAGE                   = 0x0186;
const unsigned short TLV_CITY                       = 0x0190;
const unsigned short TLV_STATE                      = 0x019A;
const unsigned short TLV_COUNTRY                    = 0x01A4;
const unsigned short TLV_WORK_COMPANY               = 0x01AE;
const unsigned short TLV_WORK_DEPARTMENT            = 0x01B8;
const unsigned short TLV_WORK_POSITION              = 0x01C2;
const unsigned short TLV_WORK_OCCUPATION            = 0x01CC;
const unsigned short TLV_AFFILATIONS                = 0x01D6;
const unsigned short TLV_INTERESTS                  = 0x01EA;
const unsigned short TLV_PAST                       = 0x01FE;
const unsigned short TLV_HOMEPAGE_CATEGORY          = 0x0212;
const unsigned short TLV_HOMEPAGE                   = 0x0213;
const unsigned short TLV_KEYWORDS                   = 0x0226;
const unsigned short TLV_SEARCH_ONLINE              = 0x0230;
const unsigned short TLV_BIRTHDAY                   = 0x023A;
const unsigned short TLV_NOTES                      = 0x0258;
const unsigned short TLV_STREET                     = 0x0262;
const unsigned short TLV_ZIP                        = 0x026C;
const unsigned short TLV_PHONE                      = 0x0276;
const unsigned short TLV_FAX                        = 0x0280;
const unsigned short TLV_CELLULAR                   = 0x028A;
const unsigned short TLV_WORK_STREET                = 0x0294;
const unsigned short TLV_WORK_CITY                  = 0x029E;
const unsigned short TLV_WORK_STATE                 = 0x02A8;
const unsigned short TLV_WORK_COUNTRY               = 0x02B2;
const unsigned short TLV_WORK_ZIP                   = 0x02BC;
const unsigned short TLV_WORK_PHONE                 = 0x02C6;
const unsigned short TLV_WORK_FAX                   = 0x02D0;
const unsigned short TLV_WORK_HOMEPAGE              = 0x02DA;
const unsigned short TLV_SHOW_WEB                   = 0x02F8;
const unsigned short TLV_NEED_AUTH                  = 0x030C;
const unsigned short TLV_TIMEZONE                   = 0x0316;
const unsigned short TLV_ORIGINALLY_CITY            = 0x0320;
const unsigned short TLV_ORIGINALLY_STATE           = 0x032A;
const unsigned short TLV_ORIGINALLY_COUNTRY         = 0x0334;

const char SEARCH_STATE_OFFLINE  = 0;
const char SEARCH_STATE_ONLINE   = 1;
const char SEARCH_STATE_DISABLED = 2;

const unsigned INFO_REQUEST_TIMEOUT = 60;

class ServerRequest
{
public:
    ServerRequest(unsigned short id);
    virtual ~ServerRequest() {}
    unsigned short id() { return m_id; }
    virtual bool answer(Buffer&, unsigned short nSubType) = 0;
    virtual void fail(unsigned short error_code = 0);
protected:
    unsigned short m_id;
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
    list<InfoRequest>::iterator it;
    for (it = infoRequests.begin(); it != infoRequests.end(); ++it){
        Contact *contact = getContacts()->contact((*it).uin);
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
            if (id == m_offlineMessagesRequestId)
            {
                log(L_WARN, "Server responded with error %04X for offline messages request.", error_code);
                // We'll never get ICQ_SRVxEND_OFFLINE_MSG, so we finish initing here instead.
            }
            else
            {
                ServerRequest *req = findServerRequest(id);
                if (req == NULL){
                    log(L_WARN, "Various event ID %04X not found for error %04X", id, error_code);
                    break;
                }
                req->fail(error_code);
            }
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
                addFullInfoRequest(data.owner.Uin.toULong());
                m_bReady = true;
                processSendQueue();
                break;
            case ICQ_SRVxOFFLINE_MSG:{
                    unsigned long uin;
                    unsigned char type, flag;
                    struct tm sendTM;
                    memset(&sendTM, 0, sizeof(sendTM));
                    QCString message;
                    unsigned short year;
                    unsigned char month, day, hours, min;
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
                    sendTM.tm_sec  = -timezone;
                #else
                    time_t now = time (NULL);
                    sendTM = *localtime (&now);
                    sendTM.tm_sec  = sendTM.tm_gmtoff - (sendTM.tm_isdst == 1 ? 3600 : 0);
                #endif
                    sendTM.tm_year = year-1900;
                    sendTM.tm_mon  = month-1;
                    sendTM.tm_mday = day;
                    sendTM.tm_hour = hours;
                    sendTM.tm_min  = min;
                    sendTM.tm_isdst = -1;
                    time_t send_time = mktime(&sendTM);
                    MessageId id;
                    Message *m = parseMessage(type, QString::number(uin), message, msg, id, 0);
                    if (m){
                        m->setTime(send_time);
                        messageReceived(m, QString::number(uin));
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
    m_socket->writeBuffer.pack(data.owner.Uin.toULong());
    m_socket->writeBuffer << cmd;
    m_socket->writeBuffer.pack((unsigned short)(seq ? seq : m_nMsgSequence));
}

void ICQClient::sendServerRequest()
{
    Buffer &b = m_socket->writeBuffer;
    char *packet = b.data(b.packetStartPos());
    unsigned short packet_size = (unsigned short)(b.size() - b.packetStartPos());
    unsigned short size = (unsigned short)(packet_size - 0x14);
    packet[0x12] = (char)((size >> 8) & 0xFF);
    packet[0x13] = (char)(size & 0xFF);
    size = (unsigned short)(packet_size - 0x16);
    packet[0x14] = (char)(size & 0xFF);
    packet[0x15] = (char)((size >> 8) & 0xFF);
    sendPacket(true);
}

void ICQClient::sendMessageRequest()
{
    serverRequest(ICQ_SRVxREQ_OFFLINE_MSG);
    m_offlineMessagesRequestId = m_nMsgSequence;
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
    QString unpack_list(Buffer &b);
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

void FullInfoRequest::fail(unsigned short)
{
    Contact *contact = NULL;
    if (m_nParts){
        if (m_client->data.owner.Uin.toULong() == m_uin){
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
    m_client->removeFullInfoRequest(m_uin);
}

QString FullInfoRequest::unpack_list(Buffer &b)
{
    QString res;
    char n;
    b >> n;
    for (; n > 0; n--){
        unsigned short c;
        b.unpack(c);
        QCString s;
        b >> s;
        if (c == 0) continue;
        if (res.length())
            res += ";";
        res += QString::number(c);
        res += ",";
        res += quoteChars(s, ";");
    }
    return res;
}

bool FullInfoRequest::answer(Buffer &b, unsigned short nSubtype)
{
    Contact *contact = NULL;
    ICQUserData *data;
    if (m_client->data.owner.Uin.toULong() == m_uin){
        data = &m_client->data.owner;
    }else{
        data = m_client->findContact(m_uin, NULL, false, contact);
        if (data == NULL){
            log(L_DEBUG, "Info request %lu not found", m_uin);
            m_client->removeFullInfoRequest(m_uin);
            return true;
        }
    }
    switch (nSubtype){
    case ICQ_SRVxGENERAL_INFO:{
            unsigned short n;
            char TimeZone;
            char authFlag;  /* ??? */
            char webAware;
            char allowDC;
            char hideEmail;
            QCString Nick, FirstName, LastName, EMail, City, State;
            QCString HomePhone, HomeFax, Address, PrivateCellular, Zip;
            b
            >> Nick
            >> FirstName
            >> LastName
            >> EMail
            >> City
            >> State
            >> HomePhone
            >> HomeFax
            >> Address
            >> PrivateCellular
            >> Zip;

            data->Nick.str() = getContacts()->fromUnicode(contact, Nick);
            data->FirstName.str() = getContacts()->fromUnicode(contact, FirstName);
            data->LastName.str() = getContacts()->fromUnicode(contact, LastName);
            data->EMail.str() = getContacts()->fromUnicode(contact, EMail);
            data->City.str() = getContacts()->fromUnicode(contact, City);
            data->State.str() = getContacts()->fromUnicode(contact, State);
            data->HomePhone.str() = getContacts()->fromUnicode(contact, HomePhone);
            data->HomeFax.str() = getContacts()->fromUnicode(contact, HomeFax);
            data->Address.str() = getContacts()->fromUnicode(contact, Address);
            data->PrivateCellular.str() = getContacts()->fromUnicode(contact, PrivateCellular);
            data->Zip.str() = QString::fromUtf8(Zip);;
            b.unpack(n);
            data->Country.asULong() = n;
            b
            >> TimeZone
            >> authFlag
            >> webAware
            >> allowDC
            >> hideEmail;
            data->TimeZone.asULong()   = TimeZone;
            data->WebAware.asBool()    = (webAware != 0);
            data->bNoDirect.asBool()   = (allowDC == 0);   // negate!
            data->HiddenEMail.asBool() = (hideEmail != 0);
            break;
        }
    case ICQ_SRVxMORE_INFO:{
            char c;
            QCString Homepage;
            b >> c;
            data->Age.asULong() = c;
            b >> c;
            b >> c;
            data->Gender.asULong() = c;
            b >> Homepage;
            data->Homepage.str() = getContacts()->fromUnicode(contact, Homepage);
            unsigned short year;
            b.unpack(year);
            data->BirthYear.asULong() = year;
            b >> c;
            data->BirthMonth.asULong() = c;
            b >> c;
            data->BirthDay.asULong() = c;
            unsigned char lang[3];
            b
            >> lang[0]
            >> lang[1]
            >> lang[2];
            data->Language.asULong() = (lang[2] << 16) + (lang[1] << 8) + lang[0];
            break;
        }
    case ICQ_SRVxEMAIL_INFO:{
            QString mail;
            char c;
            b >> c;
            for (;c > 0;c--){
                char d;
                b >> d;
                QCString s;
                b >> s;
                s = quoteChars(s, ";");
                if (mail.length())
                    mail += ";";
                mail += getContacts()->fromUnicode(contact, s);
                mail += '/';
                if (d)
                    mail += '-';
            }
            data->EMails.str() = mail;
            break;
        }
    case ICQ_SRVxWORK_INFO:{
            unsigned short n;
            QCString WorkCity, WorkState, WorkPhone, WorkFax, WorkAddress, WorkZip;
            QCString WorkName, WorkDepartment, WorkPosition, WorkHomepage;
            b
            >> WorkCity
            >> WorkState
            >> WorkPhone
            >> WorkFax
            >> WorkAddress
            >> WorkZip;
            data->WorkCity.str() = getContacts()->fromUnicode(contact, WorkCity);
            data->WorkState.str() = getContacts()->fromUnicode(contact, WorkState);
            data->WorkPhone.str() = getContacts()->fromUnicode(contact, WorkPhone);
            data->WorkFax.str() = getContacts()->fromUnicode(contact, WorkFax);
            data->WorkAddress.str() = getContacts()->fromUnicode(contact, WorkAddress);
            data->WorkZip.str() = getContacts()->fromUnicode(contact, WorkZip);

            b.unpack(n);
            data->WorkCountry.asULong() = n;
            b
            >> WorkName
            >> WorkDepartment
            >> WorkPosition;
            data->WorkName.str() = getContacts()->fromUnicode(contact, WorkName);
            data->WorkDepartment.str() = getContacts()->fromUnicode(contact, WorkDepartment);
            data->WorkPosition.str() = getContacts()->fromUnicode(contact, WorkPosition);

            b.unpack(n);
            data->Occupation.asULong() = n;
            b >> WorkHomepage;
            data->WorkHomepage.str() = getContacts()->fromUnicode(contact, WorkHomepage);
            break;
        }
    case ICQ_SRVxABOUT_INFO: {
            QCString About;
            b >> About;
            data->About.str() = getContacts()->fromUnicode(contact, About);
            break;
        }
    case ICQ_SRVxINTERESTS_INFO:
        data->Interests.str() = unpack_list(b);
        break;
    case ICQ_SRVxBACKGROUND_INFO:
        data->Backgrounds.str() = unpack_list(b);
        data->Affilations.str() = unpack_list(b);
        break;
    case ICQ_SRVxUNKNOWN_INFO:
        break;
    default:
        log(L_WARN, "Unknwon info type %04X for %lu", nSubtype, m_uin);
    }
    m_nParts++;
    if (m_nParts >= 8){
        data->InfoFetchTime.asULong() = data->InfoUpdateTime.toULong() ? data->InfoUpdateTime.toULong() : 1;
        if (contact != NULL){
            m_client->setupContact(contact, data);
            Event e(EventContactChanged, contact);
            e.process();
        }else{
            int tz;
#ifndef HAVE_TM_GMTOFF
            tz = - timezone;
#else
            time_t now;
            time(&now);
            struct tm *tm = localtime(&now);
            tz = tm->tm_gmtoff;
            if (tm->tm_isdst) tz -= (60 * 60);
#endif
            tz = - tz / (30 * 60);
            m_client->setupContact(getContacts()->owner(), data);
            if (data->TimeZone.toULong() != (unsigned)tz){
                data->TimeZone.asULong() = tz;
                m_client->setMainInfo(data);
            }
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

unsigned ICQClient::processInfoRequest()
{
    if ((getState() != Connected) || infoRequests.empty())
        return 0;
    for (list<InfoRequest>::iterator it = infoRequests.begin(); it != infoRequests.end(); ++it){
        if ((*it).request_id)
            continue;
        unsigned delay = delayTime(SNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxVAR_REQxSRV));
        if (delay)
            return delay;
        unsigned long uin = (*it).uin;
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer << ((uin == data.owner.Uin.toULong()) ? ICQ_SRVxREQ_OWN_INFO : ICQ_SRVxREQ_FULL_INFO);
        m_socket->writeBuffer.pack(uin);
        sendServerRequest();
        (*it).request_id = m_nMsgSequence;
        time_t now;
        time(&now);
        (*it).start_time = now;
        varRequests.push_back(new FullInfoRequest(this, m_nMsgSequence, uin));
    }
    return 0;
}

void ICQClient::checkInfoRequest()
{
    time_t now;
    time(&now);
    for (list<InfoRequest>::iterator it = infoRequests.begin(); it != infoRequests.end(); ){
        if (((*it).request_id == 0) || ((time_t)((*it).start_time + INFO_REQUEST_TIMEOUT) < now)){
            ++it;
            continue;
        }
        ServerRequest *req = findServerRequest((*it).request_id);
        if (req){
            req->fail();
        }else{
            infoRequests.erase(it);
        }
        it = infoRequests.begin();
    }
}

void ICQClient::addFullInfoRequest(unsigned long uin)
{
    for (list<InfoRequest>::iterator it = infoRequests.begin(); it != infoRequests.end(); ++it){
        if ((*it).uin == uin)
            return;
    }
    InfoRequest r;
    r.uin = uin;
    r.request_id = 0;
    infoRequests.push_back(r);
    processSendQueue();
}

void ICQClient::removeFullInfoRequest(unsigned long uin)
{
    list<InfoRequest>::iterator it;
    for (it = infoRequests.begin(); it != infoRequests.end(); ++it){
        if ((*it).uin == uin){
            infoRequests.erase(it);
            break;
        }
    }
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
    load_data(ICQProtocol::icqUserData, &res.data);
    Event e(EventSearchDone, &res);
    e.process();
    free_data(ICQProtocol::icqUserData, &res.data);
}

bool SearchWPRequest::answer(Buffer &b, unsigned short nSubType)
{
    QCString Nick, FirstName, LastName, EMail;
    SearchResult res;
    res.id = m_id;
    res.client = m_client;
    load_data(ICQProtocol::icqUserData, &res.data);

    unsigned short n;
    b >> n;
    b.unpack(res.data.Uin.asULong());
    char waitAuth;
    unsigned short state;
    char gender;
    unsigned short age;
    b
    >> Nick
    >> FirstName
    >> LastName
    >> EMail
    >> waitAuth;
    res.data.Nick.str() = QString::fromUtf8(Nick);
    res.data.FirstName.str() = QString::fromUtf8(FirstName);
    res.data.LastName.str() = QString::fromUtf8(LastName);
    res.data.EMail.str() = QString::fromUtf8(EMail);

    b.unpack(state);
    b >> gender;
    b.unpack(age);

    if (waitAuth)
        res.data.WaitAuth.asBool() = true;
    switch (state){
    case SEARCH_STATE_OFFLINE:
        res.data.Status.asULong() = STATUS_OFFLINE;
        break;
    case SEARCH_STATE_ONLINE:
        res.data.Status.asULong() = STATUS_ONLINE;
        break;
    case SEARCH_STATE_DISABLED:
        res.data.Status.asULong() = STATUS_UNKNOWN;
        break;
    }
    res.data.Gender.asULong() = gender;
    res.data.Age.asULong()    = age;

    if (res.data.Uin.toULong() != m_client->data.owner.Uin.toULong()){
        Event e(EventSearch, &res);
        e.process();
    }
    free_data(ICQProtocol::icqUserData, &res.data);

    if (nSubType == 0xAE01){
        unsigned long all;
        b >> all;
        load_data(ICQProtocol::icqUserData, &res.data);
        res.data.Uin.asULong() = all;
        Event e(EventSearchDone, &res);
        e.process();
        free_data(ICQProtocol::icqUserData, &res.data);
        return true;
    }
    return false;
}

unsigned short ICQClient::findByUin(unsigned long uin)
{
    if (getState() != Connected)
        return (unsigned short)(-1);
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer
    << ICQ_SRVxREQ_WP_UIN;
    m_socket->writeBuffer.tlvLE(TLV_UIN, uin);
    sendServerRequest();
    varRequests.push_back(new SearchWPRequest(this, m_nMsgSequence));
    return m_nMsgSequence;
}

unsigned short ICQClient::findByMail(const char *mail)
{
    if (getState() != Connected)
        return (unsigned short)(-1);
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer
    << ICQ_SRVxREQ_WP_MAIL;
    m_socket->writeBuffer.tlvLE(TLV_EMAIL, mail);
    sendServerRequest();
    varRequests.push_back(new SearchWPRequest(this, m_nMsgSequence));
    return m_nMsgSequence;
}

void ICQClient::packTlv(unsigned short tlv, unsigned short code, const char *keywords)
{
    string k;
    if (keywords)
        k = keywords;
    if ((code == 0) && k.empty())
        return;
    Buffer b;
    b.pack(code);
    b << k;
    m_socket->writeBuffer.tlvLE(tlv, b);
}

unsigned short ICQClient::findWP(const char *szFirst, const char *szLast, const char *szNick,
                                 const char *szEmail, char age, char nGender,
                                 unsigned short nLanguage, const char *szCity, const char *szState,
                                 unsigned short nCountryCode,
                                 const char *szCoName, const char *szCoDept, const char *szCoPos,
                                 unsigned short nOccupation,
                                 unsigned short nPast, const char *szPast,
                                 unsigned short nInterests, const char *szInterests,
                                 unsigned short nAffilation, const char *szAffilation,
                                 unsigned short nHomePage, const char *szHomePage,
                                 const char *szKeyWords, bool bOnlineOnly)
{
    if (getState() != Connected)
        return (unsigned short)(-1);
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << ICQ_SRVxREQ_WP_FULL;

    unsigned long nMinAge = 0;
    unsigned long nMaxAge = 0;
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

    if (szCity && *szCity)
        m_socket->writeBuffer.tlvLE(TLV_CITY, szCity);
    if (szState && *szState)
        m_socket->writeBuffer.tlvLE(TLV_STATE, szState);
    if (szCoName && *szCoName)
        m_socket->writeBuffer.tlvLE(TLV_WORK_COMPANY, szCoName);
    if (szCoDept && *szCoDept)
        m_socket->writeBuffer.tlvLE(TLV_WORK_DEPARTMENT, szCoDept);
    if (szCoPos && *szCoPos)
        m_socket->writeBuffer.tlvLE(TLV_WORK_POSITION, szCoPos);
    if (nMinAge || nMaxAge)
        m_socket->writeBuffer.tlvLE(TLV_AGE_RANGE, (nMaxAge << 16) + nMinAge);
    if (nGender)
        m_socket->writeBuffer.tlvLE(TLV_GENDER, nGender);
    if (nLanguage)
        m_socket->writeBuffer.tlvLE(TLV_LANGUAGE, nLanguage);
    if (nCountryCode)
        m_socket->writeBuffer.tlvLE(TLV_COUNTRY, nCountryCode);
    if (nOccupation)
        m_socket->writeBuffer.tlvLE(TLV_WORK_OCCUPATION, nOccupation);
    packTlv(TLV_PAST, nPast, szPast);
    packTlv(TLV_INTERESTS, nInterests, szInterests);
    packTlv(TLV_AFFILATIONS, nAffilation, szAffilation);
    packTlv(TLV_HOMEPAGE, nHomePage, szHomePage);
    if (szFirst && *szFirst)
        m_socket->writeBuffer.tlvLE(TLV_FIRST_NAME, szFirst);
    if (szLast && *szLast)
        m_socket->writeBuffer.tlvLE(TLV_LAST_NAME, szLast);
    if (szNick && *szNick)
        m_socket->writeBuffer.tlvLE(TLV_NICK, szNick);
    if (szKeyWords && *szKeyWords)
        m_socket->writeBuffer.tlvLE(TLV_KEYWORDS, szKeyWords);
    if (szEmail && *szEmail)
        m_socket->writeBuffer.tlvLE(TLV_EMAIL, szEmail);
    if (bOnlineOnly)
        m_socket->writeBuffer.tlvLE(TLV_SEARCH_ONLINE, (char)1);
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
    QString m_nick;
    QString m_firstName;
    QString m_lastName;
    QString m_city;
    QString m_state;
    QString m_address;
    QString m_zip;
    QString m_email;
    QString m_homePhone;
    QString m_homeFax;
    QString m_privateCellular;
    bool   m_hiddenEMail;
    unsigned m_country;
    unsigned m_tz;
    ICQClient *m_client;
};

SetMainInfoRequest::SetMainInfoRequest(ICQClient *client, unsigned short id, ICQUserData *data)
        : ServerRequest(id)
{
    m_client = client;
    m_nick = data->Nick.str();
    m_firstName = data->FirstName.str();
    m_lastName = data->LastName.str();
    m_city = data->City.str();
    m_state = data->State.str();
    m_address = data->Address.str();
    m_zip = data->Zip.str();
    m_email = data->EMail.str();
    m_homePhone = data->HomePhone.str();
    m_homeFax = data->HomeFax.str();
    m_privateCellular = data->PrivateCellular.str();
    m_country = data->Country.toULong();
    m_tz = data->TimeZone.toULong();
    m_hiddenEMail = data->HiddenEMail.toBool();
}

bool SetMainInfoRequest::answer(Buffer&, unsigned short)
{
    m_client->data.owner.Nick.str() = m_nick;
    m_client->data.owner.FirstName.str() = m_firstName;
    m_client->data.owner.LastName.str() = m_lastName;
    m_client->data.owner.City.str() = m_city;
    m_client->data.owner.State.str() = m_state;
    m_client->data.owner.Address.str() = m_address;
    m_client->data.owner.Zip.str() = m_zip;
    m_client->data.owner.EMail.str() = m_email;
    m_client->data.owner.HomePhone.str() = m_homePhone;
    m_client->data.owner.HomeFax.str() = m_homeFax;
    m_client->data.owner.PrivateCellular.str() = m_privateCellular;
    m_client->data.owner.Country.asULong() = m_country;
    m_client->data.owner.TimeZone.asULong() = m_tz;
    m_client->data.owner.HiddenEMail.asBool() = m_hiddenEMail;
    Event e(EventClientChanged, m_client);
    e.process();
    m_client->sendUpdate();
    return true;
}

// ******************************************
//  static helper functions
// ******************************************
static Tlv makeSString(unsigned id, const QString &str)
{
    unsigned len = str.utf8().length() + 1; // including '\0'
    QByteArray ba( len + 2 );
    ba[0] = (char)((len     ) & 0xff);
    ba[1] = (char)((len >> 8) & 0xff);
    memcpy( ba.data() + 2, str.utf8(), len );
    return Tlv( id, ba.size(), ba.data() );
}

static Tlv makeBCombo(unsigned id, unsigned long y, unsigned long m, unsigned long d)
{
    unsigned short buf[4];
    buf[0] = (unsigned short)(y);
    buf[1] = (unsigned short)(m);
    buf[2] = (unsigned short)(d);
    buf[3] = '\0';

    return Tlv( id, 6, (const char*)buf );
}

static Tlv makeECombo(unsigned id, const QString &str)
{
    unsigned len = str.utf8().length() + 1; // including '\0'
    QByteArray ba( len + 3 );
    ba[0] = (char)((len     ) & 0xff);
    ba[1] = (char)((len >> 8) & 0xff);
    memcpy( ba.data() + 2, str.utf8(), len  );
    ba[ (int)len + 2 ] = '\0';  // permission (don't use in icq directories)
    return Tlv( id, ba.size(), ba.data() );
}

static QValueList<Tlv> makeICombo(unsigned id, const QString &str)
{
    QValueList<Tlv> list;
    if ( str.isEmpty() )
        return list;

    QCString cstr = str.utf8();
    int cur = 0;
    int idx = 0;
    do {
        idx = cstr.find( ',', cur );
        if( idx == -1 )
            break;

        int cat = cstr.mid( cur, idx - cur ).toULong();
        cur = idx + 1;

        idx = cstr.find( ';', cur );
        if( idx == -1 )
            idx = cstr.length();

        QCString data = cstr.mid( cur, idx - cur);
        cur = idx + 1;

        int len = data.length();

        QByteArray ba( len + 4 );
        ba[0] = (char)((cat     ) & 0xff);
        ba[1] = (char)((cat >> 8) & 0xff);
        ba[2] = (char)((len     ) & 0xff);
        ba[3] = (char)((len >> 8) & 0xff);
        memcpy( ba.data() + 4, data.data(), len  );

        list.append( Tlv( id, ba.size(), ba.data() ) );
    } while( idx != (int)cstr.length() );
    return list;
}

static Tlv makeUInt32(unsigned id, unsigned long d)
{
    char data[4];

    data[0] = (char)((d >>  0) & 0xff);
    data[1] = (char)((d >>  8) & 0xff);
    data[2] = (char)((d >> 16) & 0xff);
    data[3] = (char)((d >> 24) & 0xff);
    return Tlv( id, 2, data );
}

static Tlv makeUInt16(unsigned id, unsigned short d)
{
    char data[2];

    data[0] = (char)((d >> 0) & 0xff);
    data[1] = (char)((d >> 8) & 0xff);
    return Tlv( id, 2, data );
}

static Tlv makeUInt8(unsigned id, unsigned char d)
{
    char data[1];

    data[0] = (char)((d >> 0) & 0xff);
    return Tlv( id, 2, data );
}

static QString getSString(const char *tlvData)
{
    unsigned len;
    len = tlvData[0] | ( tlvData[1] << 8 );
    QString ret = QString::fromUtf8( &tlvData[2], len);
    return ret;
}

static void getBCombo(const char *tlvData, unsigned long &y, unsigned long &m, unsigned long &d)
{
    unsigned short *buf = (unsigned short*)tlvData;
    y = buf[0];
    m = buf[1];
    d = buf[2];
}

static QString getECombo(const char *tlvData)
{
    unsigned len;
    len = tlvData[0] | ( tlvData[1] << 8 );
    QString ret = QString::fromUtf8( &tlvData[2], len);
    return ret;
}

static QString getICombo(const char *tlvData, const QString &o)
{
    QString ret;
    QString others = o;

    int cat = tlvData[0] | ( tlvData[1] >> 8 );
    ret = QString::number( cat ) + "," + getSString( &tlvData[2] );
    if( others.isEmpty() )
        return ret;
    return others + ';' + ret;
}

static unsigned long getUInt32(const char *tlvData)
{
    unsigned long ret;
    ret = tlvData[0] | ( tlvData[1] << 8 ) | ( tlvData[1] << 16 ) |  ( tlvData[1] << 24 );
    return ret;
}

static unsigned short getUInt16(const char *tlvData)
{
    unsigned short ret;
    ret = tlvData[0] | ( tlvData[1] << 8 );
    return ret;
}

static char getUInt8(const char *tlvData)
{
    unsigned char ret;
    ret = tlvData[0];
    return ret;
}

class ChangeInfoRequest : public ServerRequest
{
public:
    ChangeInfoRequest(ICQClient *client, unsigned short id, const QValueList<Tlv> &clientInfoTLVs);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    ICQClient *m_client;
    QValueList<Tlv> m_clientInfoTLVs;
};

ChangeInfoRequest::ChangeInfoRequest(ICQClient *client, unsigned short id, const QValueList<Tlv> &clientInfoTLVs)
        : ServerRequest(id), m_client(client), m_clientInfoTLVs(clientInfoTLVs)
{
}

bool ChangeInfoRequest::answer(Buffer&, unsigned short)
{
    bool bFirstAffilation = true;
    bool bFirstInterest = true;
    bool bFirstBackground = true;
    for( unsigned i = 0; i < m_clientInfoTLVs.count(); i++ ) {
        Tlv *tlv = &m_clientInfoTLVs[i];
        switch(tlv->Num()) {
            case TLV_FIRST_NAME:
                m_client->data.owner.FirstName.str() = getSString(tlv->Data());
                break;
            case TLV_LAST_NAME:
                m_client->data.owner.LastName.str() = getSString(tlv->Data());
                break;
            case TLV_NICK:
                m_client->data.owner.Nick.str() = getSString(tlv->Data());
                break;
            case TLV_EMAIL:
                m_client->data.owner.EMail.str() = getECombo(tlv->Data());
                break;
            case TLV_AGE:
                m_client->data.owner.Age.asULong() = getUInt16(tlv->Data());
                break;
            case TLV_GENDER:
                m_client->data.owner.Gender.asULong() = getUInt8(tlv->Data());
                break;
            case TLV_LANGUAGE:
                // getUInt16((tlv->Data()));
                break;
            case TLV_CITY:
                m_client->data.owner.City.str() = getSString(tlv->Data());
                break;
            case TLV_STATE:
                m_client->data.owner.State.str() = getSString(tlv->Data());
                break;
            case TLV_COUNTRY:
                m_client->data.owner.Country.asULong() = getUInt16(tlv->Data());
                break;
            case TLV_WORK_COMPANY:
                m_client->data.owner.WorkName.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_DEPARTMENT:
                m_client->data.owner.WorkDepartment.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_POSITION:
                m_client->data.owner.WorkPosition.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_OCCUPATION:
                m_client->data.owner.Occupation.asULong() = getUInt16(tlv->Data());
                break;
            case TLV_AFFILATIONS: {
                if( bFirstAffilation ) {
                    m_client->data.owner.Affilations.clear();
                    bFirstAffilation = false;
                }
                m_client->data.owner.Affilations.str() = getICombo(tlv->Data(), m_client->data.owner.Affilations.str());
                break;
            }
            case TLV_INTERESTS:
                if( bFirstInterest ) {
                    m_client->data.owner.Interests.clear();
                    bFirstInterest = false;
                }
                m_client->data.owner.Interests.str() = getICombo(tlv->Data(), m_client->data.owner.Interests.str());
                break;
            case TLV_PAST: {
                if( bFirstBackground ) {
                    m_client->data.owner.Backgrounds.clear();
                    bFirstBackground = false;
                }
                m_client->data.owner.Backgrounds.str() = getICombo(tlv->Data(), m_client->data.owner.Backgrounds.str());
                break;
            }
//  530       0x0212      icombo     User homepage category/keywords
            case TLV_HOMEPAGE:
                m_client->data.owner.Homepage.str() = getSString(tlv->Data());
                break;
            case TLV_BIRTHDAY: {
                getBCombo(tlv->Data(), m_client->data.owner.BirthYear.asULong(),
                                       m_client->data.owner.BirthMonth.asULong(),
                                       m_client->data.owner.BirthDay.asULong());
                break;
            }
            case TLV_NOTES:
                m_client->data.owner.About.str() = getSString(tlv->Data());
                break;
            case TLV_STREET:
                m_client->data.owner.Address.str() = getSString(tlv->Data());
                break;
            case TLV_ZIP: {
                QString str;
                str.sprintf("%lu", getUInt32(tlv->Data()));
                m_client->data.owner.Zip.str() = str;
                break;
            }
            case TLV_PHONE:
                m_client->data.owner.HomePhone.str() = getSString(tlv->Data());
                break;
            case TLV_FAX:
                m_client->data.owner.HomeFax.str() = getSString(tlv->Data());
                break;
            case TLV_CELLULAR:
                m_client->data.owner.PrivateCellular.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_STREET:
                m_client->data.owner.WorkAddress.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_CITY:
                m_client->data.owner.WorkCity.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_STATE:
                m_client->data.owner.WorkState.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_COUNTRY:
                m_client->data.owner.WorkCountry.asULong() = getUInt16((tlv->Data()));
                break;
            case TLV_WORK_ZIP: {
                QString str;
                str.sprintf("%lu", getUInt32(tlv->Data()));
                m_client->data.owner.WorkZip.str() = str;
                break;
            }
            case TLV_WORK_PHONE:
                m_client->data.owner.WorkPhone.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_FAX:
                m_client->data.owner.WorkFax.str() = getSString(tlv->Data());
                break;
            case TLV_WORK_HOMEPAGE:
                m_client->data.owner.WorkHomepage.str() = getSString(tlv->Data());
                break;
            case TLV_SHOW_WEB:
                m_client->data.owner.WebAware.asBool() = !getUInt8(tlv->Data());
                break;
            case TLV_NEED_AUTH:
                m_client->data.owner.WaitAuth.asBool() = getUInt8(tlv->Data());
                break;
            case TLV_TIMEZONE:
                m_client->data.owner.TimeZone.asBool() = getUInt8(tlv->Data());
                break;
  /*
  800         0x0320      sstring   User originally from city
  810         0x032A      sstring   User originally from state
  820         0x0334      uint16    User originally from country (code)
    */
            default:
                break;
        }
    }
    m_client->sendStatus();
    Event e(EventClientChanged, m_client);
    e.process();
    return true;
}

void ICQClient::setMainInfo(ICQUserData *d)
{
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << ICQ_SRVxREQ_MODIFY_MAIN
    << d->Nick.str()
    << d->FirstName.str()
    << d->LastName.str()
    << d->EMail.str()
    << d->City.str()
    << d->State.str()
    << d->HomePhone.str()
    << d->HomeFax.str()
    << d->Address.str()
    << d->PrivateCellular.str()
    << d->Zip.str();
    m_socket->writeBuffer.pack((unsigned short)(d->Country.toULong()));
    m_socket->writeBuffer.pack((char)(d->TimeZone.toULong()));
    m_socket->writeBuffer.pack((char)(d->HiddenEMail.toBool()));
    sendServerRequest();

    varRequests.push_back(new SetMainInfoRequest(this, m_nMsgSequence, d));
}

void ICQClient::setClientInfo(void *_data)
{
    if (getState() != Connected)
        return;

    ICQUserData *d = (ICQUserData*)_data;

    if (m_bAIM){
        d->ProfileFetch.asBool() = true;
        data.owner.About.str() = d->About.str();
        setAIMInfo(d);
        setProfile(d);
        return;
    }

    QValueList<Tlv> clientInfoTLVs;

    if (d->FirstName.str() == data.owner.FirstName.str())
        clientInfoTLVs.append(makeSString(TLV_FIRST_NAME, d->FirstName.str()));

    if (d->LastName.str() == data.owner.LastName.str())
        clientInfoTLVs.append(makeSString(TLV_LAST_NAME, d->LastName.str()));

    if (d->Nick.str() == data.owner.Nick.str())
        clientInfoTLVs.append(makeSString(TLV_NICK, d->Nick.str()));

    if (d->EMail.str() == data.owner.EMail.str())
        clientInfoTLVs.append(makeECombo(TLV_EMAIL, d->EMail.str()));

    if (d->Age.toULong() != data.owner.Age.toULong())
        clientInfoTLVs.append(makeUInt16(TLV_AGE, d->Age.toULong()));

    if (d->Gender.toULong() != data.owner.Gender.toULong())
        clientInfoTLVs.append(makeUInt8(TLV_GENDER, d->Gender.toULong()));

    // TLV_LANGUAGE (uint16) missing

    if (d->City.str() == data.owner.City.str())
        clientInfoTLVs.append(makeSString(TLV_CITY, d->City.str()));

    if (d->State.str() == data.owner.State.str())
        clientInfoTLVs.append(makeSString(TLV_STATE, d->State.str()));

    if (d->Country.toULong() != data.owner.Country.toULong())
        clientInfoTLVs.append(makeUInt16(TLV_COUNTRY, d->Country.toULong()));

    if (d->WorkName.str() == data.owner.WorkName.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_COMPANY, d->WorkName.str()));

    if (d->WorkDepartment.str() == data.owner.WorkDepartment.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_DEPARTMENT, d->WorkDepartment.str()));

    if (d->WorkPosition.str() == data.owner.WorkPosition.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_POSITION, d->WorkPosition.str()));

    if (d->Occupation.toULong() != data.owner.Occupation.toULong())
        clientInfoTLVs.append(makeUInt16(TLV_WORK_OCCUPATION, d->Occupation.toULong()));

    if (d->Affilations.str() == data.owner.Affilations.str())
        clientInfoTLVs += makeICombo(TLV_AFFILATIONS, d->Affilations.str());

    if (d->Interests.str() == data.owner.Interests.str())
        clientInfoTLVs += makeICombo(TLV_INTERESTS, d->Interests.str());

    if (d->Backgrounds.str() == data.owner.Backgrounds.str())
        clientInfoTLVs += makeICombo(TLV_PAST, d->Backgrounds.str());

//  530       0x0212      icombo     User homepage category/keywords

    if (d->Homepage.str() == data.owner.Homepage.str())
        clientInfoTLVs.append(makeSString(TLV_HOMEPAGE, d->Homepage.str()));

    if (d->BirthDay.toULong() != data.owner.BirthDay.toULong() ||
        d->BirthMonth.toULong() != data.owner.BirthMonth.toULong() ||
        d->BirthYear.toULong() != data.owner.BirthYear.toULong()) {
        clientInfoTLVs.append(makeBCombo(TLV_BIRTHDAY, d->BirthYear.toULong(), d->BirthMonth.toULong(), d->BirthDay.toULong()));
    }

    if (d->About.str() == data.owner.About.str())
        clientInfoTLVs.append(makeSString(TLV_NOTES, d->About.str()));

    if (d->Address.str() == data.owner.Address.str())
        clientInfoTLVs.append(makeSString(TLV_STREET, d->Address.str()));

    if (d->Zip.str() == data.owner.Zip.str())
        clientInfoTLVs.append(makeUInt32(TLV_ZIP, QString(d->Zip.str()).toULong()));

    if (d->HomePhone.str() == data.owner.HomePhone.str())
        clientInfoTLVs.append(makeSString(TLV_PHONE, d->HomePhone.str()));

    if (d->HomeFax.str() == data.owner.HomeFax.str())
        clientInfoTLVs.append(makeSString(TLV_FAX, d->HomeFax.str()));

    if (d->PrivateCellular.str() == data.owner.PrivateCellular.str())
        clientInfoTLVs.append(makeSString(TLV_CELLULAR, d->PrivateCellular.str()));

    if (d->WorkAddress.str() == data.owner.WorkAddress.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_STREET, d->WorkAddress.str()));

    if (d->WorkCity.str() == data.owner.WorkCity.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_CITY, d->WorkCity.str()));

    if (d->WorkState.str() == data.owner.WorkState.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_STATE, d->WorkState.str()));

     if (d->WorkCountry.toULong() != data.owner.WorkCountry.toULong())
        clientInfoTLVs.append(makeUInt16(TLV_WORK_COUNTRY, d->WorkCountry.toULong()));

    if (d->WorkZip.str() != data.owner.WorkZip.str())
        clientInfoTLVs.append(makeUInt32(TLV_WORK_ZIP, QString(d->WorkZip.str()).toULong()));

    if (d->WorkPhone.str() == data.owner.WorkPhone.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_PHONE, d->WorkPhone.str()));

    if (d->WorkFax.str() == data.owner.WorkFax.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_FAX, d->WorkFax.str()));

    if (d->WorkHomepage.str() == data.owner.WorkHomepage.str())
        clientInfoTLVs.append(makeSString(TLV_WORK_HOMEPAGE, d->WorkHomepage.str()));

    if (d->WebAware.toBool() != data.owner.WebAware.toBool())
        clientInfoTLVs.append(makeUInt8(TLV_SHOW_WEB, !d->WebAware.toBool()));

    if (d->WaitAuth.toBool() != data.owner.WaitAuth.toBool())
        clientInfoTLVs.append(makeUInt8(TLV_NEED_AUTH, d->WaitAuth.toBool()));

    if (d->TimeZone.toULong() != data.owner.TimeZone.toULong())
        clientInfoTLVs.append(makeUInt8(TLV_TIMEZONE, d->TimeZone.toULong()));
  /*
  800         0x0320      sstring   User originally from city
  810         0x032A      sstring   User originally from state
  820         0x0334      uint16    User originally from country (code)
  */

    if (!clientInfoTLVs.isEmpty()) {
        serverRequest(ICQ_SRVxREQ_MORE);
        m_socket->writeBuffer << ICQ_SRVxCLI_SET_FULLINFO;
        for( unsigned i =0; i < clientInfoTLVs.count(); i++ ) {
            Tlv *tlv = &clientInfoTLVs[i];
            m_socket->writeBuffer.tlvLE( tlv->Num(), *tlv, tlv->Size() );
        }
        sendServerRequest();
        varRequests.push_back(new ChangeInfoRequest(this, m_nMsgSequence, clientInfoTLVs));
    }

    setChatGroup();
    sendStatus();
}

class SetPasswordRequest : public ServerRequest
{
public:
    SetPasswordRequest(ICQClient *client, unsigned short id, const QString &pwd);
protected:
    bool answer(Buffer &b, unsigned short nSubtype);
    virtual void fail(unsigned short error_code);
    QString m_pwd;
    ICQClient *m_client;
};

SetPasswordRequest::SetPasswordRequest(ICQClient *client, unsigned short id, const QString &pwd)
        : ServerRequest(id), m_pwd(pwd), m_client(client)
{}

bool SetPasswordRequest::answer(Buffer&, unsigned short)
{
    m_client->setPassword(m_pwd);
    return true;
}

void SetPasswordRequest::fail(unsigned short error_code)
{
    log(L_DEBUG, "Password change fail: %X", error_code);
    clientErrorData d;
    d.client  = m_client;
    d.code    = 0;
    d.err_str = I18N_NOOP("Change password fail");
    d.args    = NULL;
    d.flags   = ERR_ERROR;
    d.options = NULL;
    d.id      = CmdPasswordFail;
    Event e(EventClientError, &d);
    e.process();
}

void ICQClient::changePassword(const QString &new_pswd)
{
    QString pwd = new_pswd;
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer
    << ICQ_SRVxREQ_CHANGE_PASSWD
    << (const char*)getContacts()->fromUnicode(NULL, pwd).data();
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
        QCString errStr = b.data(b.readPos());
        SendMsg &s = m_client->smsQueue.front();
        SMSMessage *sms = static_cast<SMSMessage*>(s.msg);
        m_client->smsQueue.erase(m_client->smsQueue.begin());
        sms->setError(errStr.data());
        Event e(EventMessageSent, sms);
        e.process();
        delete sms;
    }else{
        b.incReadPos(6);
        QCString provider;
        QCString answer_QCString;
        b.unpackStr(provider);
        b.unpackStr(answer_QCString);
// FIXME
        std::string answer = (const char *)answer_QCString;
        string::iterator s = answer.begin();
        auto_ptr<XmlNode> top(XmlNode::parse(s, answer.end()));
        QString error = I18N_NOOP("SMS send fail");
        QString network;
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

        if (error.isEmpty()){
            if (!m_client->smsQueue.empty()){
                SendMsg &s = m_client->smsQueue.front();
                SMSMessage *sms = static_cast<SMSMessage*>(s.msg);
                sms->setNetwork(network);
                if ((sms->getFlags() & MESSAGE_NOHISTORY) == 0){
                    SMSMessage m;
                    m.setContact(sms->contact());
                    m.setText(s.part);
                    m.setPhone(sms->getPhone());
                    m.setNetwork(network);
                    Event e(EventSent, &m);
                    e.process();
                }
            }
        }else{
            if (!m_client->smsQueue.empty()){
                SendMsg &s = m_client->smsQueue.front();
                s.msg->setError(error);
                Event e(EventMessageSent, s.msg);
                e.process();
                delete s.msg;
                m_client->smsQueue.erase(m_client->smsQueue.begin());
            }
        }
    }
    m_client->processSendQueue();
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
    m_client->processSendQueue();
}

const unsigned MAX_SMS_LEN_LATIN1   = 160;
const unsigned MAX_SMS_LEN_UNICODE  = 70;

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
        I18N_NOOP("Jan"),
        I18N_NOOP("Feb"),
        I18N_NOOP("Mar"),
        I18N_NOOP("Apr"),
        I18N_NOOP("May"),
        I18N_NOOP("Jun"),
        I18N_NOOP("Jul"),
        I18N_NOOP("Aug"),
        I18N_NOOP("Sep"),
        I18N_NOOP("Oct"),
        I18N_NOOP("Nov"),
        I18N_NOOP("Dec")
    };

unsigned ICQClient::processSMSQueue()
{
    if (m_sendSmsId)
        return 0;
    for (;;){
        if (smsQueue.empty())
            break;
        unsigned delay = delayTime(SNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxVAR_REQxSRV));
        if (delay)
            return delay;
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

        string  nmb = "+";
        QString phone = sms->getPhone();
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
        xmltree.pushnode(new XmlLeaf("senders_UIN",QString::number(data.owner.Uin.toULong()).latin1()));
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
        break;
    }
    return 0;
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
    Event e(EventRandomChat, (void*)uin);
    e.process();
    return true;
}

void RandomChatRequest::fail(unsigned short)
{
    Event e(EventRandomChat, NULL);
    e.process();
}

void ICQClient::searchChat(unsigned short group)
{
    if (getState() != Connected){
        Event e(EventRandomChat, NULL);
        e.process();
        return;
    }
    serverRequest(ICQ_SRVxREQ_MORE);
    m_socket->writeBuffer << (unsigned short)ICQ_SRVxREQ_RANDOM_CHAT;
    m_socket->writeBuffer.pack(group);
    sendServerRequest();
    varRequests.push_back(new RandomChatRequest(this, m_nMsgSequence));
}
