/***************************************************************************
                          services.cpp  -  description
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

#include <map>

using namespace std;
using namespace SIM;

ServiceSocket::ServiceSocket(ICQClient *client, unsigned short id)
{
    m_client = client;
    m_id     = id;
    m_client->m_services.push_back(this);
    m_socket = NULL;

    m_bConnected = false;
}

ServiceSocket::~ServiceSocket()
{
    for (list<ServiceSocket*>::iterator it = m_client->m_services.begin(); it != m_client->m_services.end(); ++it){
        if ((*it) == this){
            m_client->m_services.erase(it);
            break;
        }
    }
    delete m_socket;
}

ClientSocket *ServiceSocket::socket()
{
    return m_socket;
}

void ServiceSocket::connect(const char *addr, unsigned short port, const char *cookie, unsigned cookie_size)
{
    m_cookie.init(0);
    m_cookie.pack(cookie, cookie_size);
    m_socket = new ClientSocket(this);
    m_socket->connect(addr, port, m_client);
}

void ServiceSocket::close()
{
    m_socket->close();
}

bool ServiceSocket::error_state(const char *err, unsigned)
{
    log(L_DEBUG, "Service error %s", err);
    return true;
}

void ServiceSocket::connect_ready()
{
    OscarSocket::connect_ready();
}

void ServiceSocket::packet_ready()
{
    OscarSocket::packet_ready();
}

void ServiceSocket::packet()
{
    log_packet(m_socket->readBuffer, false,ICQPlugin::icq_plugin->OscarPacket);
    switch (m_nChannel){
    case ICQ_CHNxNEW:
        flap(ICQ_CHNxNEW);
        m_socket->writeBuffer << 0x00000001L;
        m_socket->writeBuffer.tlv(6, m_cookie.data(), (unsigned short)(m_cookie.size()));
        m_cookie.init(0);
        sendPacket();
        break;
    case ICQ_CHNxDATA:
        unsigned short fam, type;
        unsigned short flags, seq, cmd;
        m_socket->readBuffer >> fam >> type >> flags >> seq >> cmd;
        if ((flags & 0x8000)) {	// some unknown data before real snac data
            // just read the length and forget it ;-)
            unsigned short unknown_length = 0;
            m_socket->readBuffer >> unknown_length;
            m_socket->readBuffer.incReadPos(unknown_length);
        }
        // now just take a look at the type because 0x0001 == error
        // in all families
        if (type == 0x0001) {
            unsigned short err_code;
            m_socket->readBuffer >> err_code;
            log(L_DEBUG,"Error! family: %04X reason",fam);
            // now decrease for icqicmb & icqvarious
            m_socket->readBuffer.decReadPos(sizeof(unsigned short));
        }
        data(fam, type, seq);
        break;
    default:
        log(L_ERROR, "Unknown channel %u", m_nChannel & 0xFF);
    }
    m_socket->readBuffer.init(6);
    m_socket->readBuffer.packetStart();
    m_bHeader = true;
}

const unsigned short USER_DIRECTORY_SERVICE		= 0x000F;

const unsigned short USER_DIRECTORY_SEARCH	     = 0x0002;
const unsigned short USER_DIRECTORY_RESULT	     = 0x0003;

const unsigned short SNACxSRV_CLIENTxREADY		= 0x0002;
const unsigned short SNACxSRV_READYxSERVER		= 0x0003;
const unsigned short SNACxSRV_REQxRATExINFO		= 0x0006;
const unsigned short SNACxSRV_RATExINFO			= 0x0007;
const unsigned short SNACxSRV_RATExACK			= 0x0008;
const unsigned short SNACxSRV_I_AM_ICQ			= 0x0017;
const unsigned short SNACxSRV_ACK_ICQ			= 0x0018;

typedef QMap<unsigned short, QStringList> REQUEST_MAP;
typedef QMap<unsigned short, unsigned short> SEQ_MAP;

class SearchSocket : public ServiceSocket
{
public:
    SearchSocket(ICQClient*);
    unsigned short add(const QStringList &str);
protected:
    void data(unsigned short fam, unsigned short type, unsigned short seq);
    void snac_service(unsigned short type);
    void snac_search(unsigned short type, unsigned short seq);
    void process();
    void addTlv(unsigned short n, const QString&, bool);
    REQUEST_MAP m_requests;
    SEQ_MAP		m_seq;
    unsigned short m_id;
};

SearchSocket::SearchSocket(ICQClient *client)
        : ServiceSocket(client, USER_DIRECTORY_SERVICE)
{
    m_id = 0;
}

static bool bLatin1(const QString &s)
{
    for (int i = 0; i < (int)(s.length()); i++){
        if (s[i].unicode() > 0x7F)
            return false;
    }
    return true;
}

void SearchSocket::addTlv(unsigned short n, const QString &s, bool bLatin)
{
    QCString str;
    if (bLatin){
        str = s.latin1();
    }else{
        str = s.utf8();
    }
    m_socket->writeBuffer.tlv(n, str.data());
}

void SearchSocket::process()
{
    if (!connected())
        return;
    for (REQUEST_MAP::iterator it = m_requests.begin(); it != m_requests.end(); ++it){
        snac(USER_DIRECTORY_SERVICE, USER_DIRECTORY_SEARCH, true);
        bool bLatin;
        if (!(*it).count() == 0)
            continue;
        if (!(*it).count() == 1){
            QStringList sl = (*it);
            QString mail = sl[0];
            bLatin = bLatin1(mail);
            m_socket->writeBuffer.tlv(0x1C, bLatin ? "us-ascii" : "utf8");
            m_socket->writeBuffer.tlv(0x0A, (unsigned short)1);
            addTlv(0x05, mail, bLatin);
        }else{
            QStringList sl = (*it);
            bLatin = bLatin1(sl[0]) &&
                     bLatin1(sl[1]) &&
                     bLatin1(sl[2]) &&
                     bLatin1(sl[3]) &&
                     bLatin1(sl[4]) &&
                     bLatin1(sl[5]) &&
                     bLatin1(sl[6]) &&
                     bLatin1(sl[7]) &&
                     bLatin1(sl[8]) &&
                     bLatin1(sl[9]);
            m_socket->writeBuffer.tlv(0x1C, bLatin ? "us-ascii" : "utf8");
            m_socket->writeBuffer.tlv(0x0A, (unsigned short)0);
            if (!sl[0].isEmpty())
                addTlv(0x01, sl[0], bLatin);
            if (!sl[1].isEmpty())
                addTlv(0x02, sl[1], bLatin);
            if (!sl[2].isEmpty())
                addTlv(0x03, sl[2], bLatin);
            if (!sl[3].isEmpty())
                addTlv(0x04, sl[3], bLatin);
            if (!sl[4].isEmpty())
                addTlv(0x06, sl[4], bLatin);
            if (!sl[5].isEmpty())
                addTlv(0x07, sl[5], bLatin);
            if (!sl[6].isEmpty())
                addTlv(0x08, sl[6], bLatin);
            if (!sl[7].isEmpty())
                addTlv(0x0C, sl[7], bLatin);
            if (!sl[8].isEmpty())
                addTlv(0x0D, sl[8], bLatin);
            if (!sl[9].isEmpty())
                addTlv(0x21, sl[9], bLatin);
        }
        sendPacket();
        m_seq.insert(SEQ_MAP::value_type(m_nMsgSequence, it.key()));
    }
    m_requests.clear();
}

unsigned short SearchSocket::add(const QStringList &name)
{
    m_requests.insert(REQUEST_MAP::value_type(++m_id, name));
    process();
    return m_id;
}

void SearchSocket::data(unsigned short fam, unsigned short type, unsigned short seq)
{
    switch (fam){
    case ICQ_SNACxFAM_SERVICE:
        snac_service(type);
        break;
    case USER_DIRECTORY_SERVICE:
        snac_search(type, seq);
        break;
    default:
        log(L_WARN, "Unknown family %04X", fam);
    }
}

void SearchSocket::snac_service(unsigned short type)
{
    switch (type){
    case SNACxSRV_READYxSERVER:
        snac(ICQ_SNACxFAM_SERVICE, SNACxSRV_I_AM_ICQ);
        m_socket->writeBuffer << 0x00010004L << 0x000F0001L;
        sendPacket();
        break;
    case SNACxSRV_ACK_ICQ:
        snac(ICQ_SNACxFAM_SERVICE, SNACxSRV_REQxRATExINFO);
        sendPacket();
        break;
    case SNACxSRV_RATExINFO:
        snac(ICQ_SNACxFAM_SERVICE, SNACxSRV_RATExACK);
        m_socket->writeBuffer << 0x00010002L << 0x00030004L << 0x0005;
        sendPacket();
        snac(ICQ_SNACxFAM_SERVICE, SNACxSRV_CLIENTxREADY);
        m_socket->writeBuffer << 0x00010003L << 0x00100739L << 0x000F0001L << 0x00100739L;
        sendPacket();
        m_bConnected = true;
        process();
        break;
    default:
        log(L_DEBUG, "Unknown service type %u", type);
    }
}

void SearchSocket::snac_search(unsigned short type, unsigned short seq)
{
    SEQ_MAP::iterator it;
    switch (type){
    case USER_DIRECTORY_RESULT:
        it = m_seq.find(seq);
        if (it == m_seq.end()){
            log(L_WARN, "Bad sequence in search answer");
        }else{
            unsigned short r;
            unsigned long nSearch;
            m_socket->readBuffer >> r >> nSearch;

            SearchResult res;
            res.id = (*it);
            res.client = m_client;
            for (unsigned n = 0; n < nSearch; n++){
                unsigned short nTlvs;
                m_socket->readBuffer >> nTlvs;
                TlvList tlvs(m_socket->readBuffer, nTlvs);
                Tlv *tlv = tlvs(0x09);
                if (tlv){
                    load_data(ICQProtocol::icqUserData, &res.data, NULL);
                    res.data.Screen.str() = tlv->Data();    // utf8 ?
                    tlv = tlvs(0x01);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.FirstName.str() = str;
                    }
                    tlv = tlvs(0x02);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.LastName.str() = str;
                    }
                    tlv = tlvs(0x03);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.MiddleName.str() = str;
                    }
                    tlv = tlvs(0x07);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.Address.str() = str;
                    }
                    tlv = tlvs(0x08);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.City.str() = str;
                    }
                    tlv = tlvs(0x0C);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.Nick.str() = str;
                    }
                    tlv = tlvs(0x07);
                    if (tlv){
                        QString str = ICQClient::convert(tlv, tlvs, 0x1C);
                        res.data.State.str() = str;
                    }
                    tlv = tlvs(0x06);
                    if (tlv){
                        QString country_text;
                        country_text.setLatin1(*tlv, tlv->Size());
                        country_text = country_text.lower();
                        for (const ext_info *info = getCountryCodes(); info->szName; ++info){
                            if (country_text == info->szName){
                                res.data.Country.asULong() = info->nCode;
                                break;
                            }
                        }
                    }
                    Event e(EventSearch, &res);
                    e.process();
                    free_data(ICQProtocol::icqUserData, &res.data);
                }
            }
            if (r != 6){
                load_data(ICQProtocol::icqUserData, &res.data, NULL);
                Event e(EventSearchDone, &res);
                e.process();
                free_data(ICQProtocol::icqUserData, &res.data);
                m_seq.erase(it);
            }
        }
        break;
    default:
        log(L_WARN, "Unknown search family type %04X", type);
    }
}

unsigned short ICQClient::aimEMailSearch(const QString &name)
{
    SearchSocket *s = NULL;
    for (list<ServiceSocket*>::iterator it = m_services.begin(); it != m_services.end(); ++it){
        if ((*it)->id() == USER_DIRECTORY_SERVICE){
            s = static_cast<SearchSocket*>(*it);
            break;
        }
    }
    if (s == NULL){
        s = new SearchSocket(this);
        requestService(s);
    }
    QStringList sl;
    sl.append(name);
    return s->add(sl);
}

unsigned short ICQClient::aimInfoSearch(const QString &first, const QString &last, const QString &middle,
                                        const QString &maiden, const QString &country, const QString &street,
                                        const QString &city, const QString &nick, const QString &zip,
                                        const QString &state)
{
    SearchSocket *s = NULL;
    for (list<ServiceSocket*>::iterator it = m_services.begin(); it != m_services.end(); ++it){
        if ((*it)->id() == USER_DIRECTORY_SERVICE){
            s = static_cast<SearchSocket*>(*it);
            break;
        }
    }
    if (s == NULL){
        s = new SearchSocket(this);
        requestService(s);
    }
    QStringList info;

    info.append(first);
    info.append(last);
    info.append(middle);
    info.append(maiden);
    info.append(country);
    info.append(street);
    info.append(city);
    info.append(nick);
    info.append(zip);
    info.append(state);
    return s->add(info);
}

