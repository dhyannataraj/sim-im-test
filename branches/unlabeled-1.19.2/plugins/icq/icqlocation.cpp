/***************************************************************************
                          icqlocation.cpp  -  description
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

#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <qtextcodec.h>

const unsigned short ICQ_SNACxLOC_ERROR             = 0x0001;
const unsigned short ICQ_SNACxLOC_REQUESTxRIGHTS    = 0x0002;
const unsigned short ICQ_SNAXxLOC_RIGHTSxGRANTED    = 0x0003;
const unsigned short ICQ_SNACxLOC_SETxUSERxINFO     = 0x0004;
const unsigned short ICQ_SNACxLOC_REQUESTxUSERxINFO = 0x0005;
const unsigned short ICQ_SNACxLOC_LOCATIONxINFO     = 0x0006;
const unsigned short ICQ_SNACxLOC_SETxDIRxINFO		= 0x0009;
const unsigned short ICQ_SNACxLOC_RESPONSExSETxINFO	= 0x000A;
const unsigned short ICQ_SNACxLOC_REQUESTxDIRxINFO  = 0x000B;
const unsigned short ICQ_SNACxLOC_DIRxINFO          = 0x000C;

static bool extractInfo(TlvList &tlvs, unsigned short id, char **data)
{
    const char *info = NULL;
    Tlv *tlv = tlvs(id);
    if (tlv)
        info = *tlv;
    return set_str(data, info);
}

QString ICQClient::convert(Tlv *tlvInfo, TlvList &tlvs, unsigned n)
{
    if (tlvInfo == NULL)
        return "";
    return convert(*tlvInfo, tlvInfo->Size(), tlvs, n);
}

QString ICQClient::convert(const char *text, unsigned size, TlvList &tlvs, unsigned n)
{
    string charset = "us-ascii";
    Tlv *tlvCharset = NULL;
    for (unsigned i = 0;; i++){
        Tlv *tlv = tlvs[i];
        if (tlv == NULL)
            break;
        if (tlv->Num() != n)
            continue;
        if (tlvCharset && (tlv->Size() < tlvCharset->Size()))
            continue;
        tlvCharset = tlv;
    }
    if (tlvCharset){
        const char *type = *tlvCharset;
        const char *p = strchr(type, '\"');
        if (p){
            p++;
            char *e = strchr((char*)p, '\"');
            if (e)
                *e = 0;
            charset = p;
        }else{
            charset = type;
        }
    }
    QString res;
    if (strstr(charset.c_str(), "us-ascii") || strstr(charset.c_str(), "utf")){
        res = QString::fromUtf8(text, size);
    }else if (strstr(charset.c_str(), "unicode")){
        unsigned short *p = (unsigned short*)text;
        for (unsigned i = 0; i < size - 1; i += 2, p++)
            res += QChar((unsigned short)htons(*p));
    }else{
        QTextCodec *codec = QTextCodec::codecForName(charset.c_str());
        if (codec){
            res = codec->toUnicode(text, size);
        }else{
            res = QString::fromUtf8(text, size);
            log(L_WARN, "Unknown encdoing %s", charset.c_str());
        }
    }
    return res;
}

void ICQClient::snac_location(unsigned short type, unsigned short seq)
{
    Contact *contact = NULL;
    ICQUserData *data;
    string screen;
    switch (type){
    case ICQ_SNAXxLOC_RIGHTSxGRANTED:
        log(L_DEBUG, "Location rights granted");
        break;
    case ICQ_SNACxLOC_ERROR:
        break;
    case ICQ_SNACxLOC_LOCATIONxINFO:
        screen = m_socket->readBuffer.unpackScreen();
        if (isOwnData(screen.c_str())){
            data = &this->data.owner;
        }else{
            data = findContact(screen.c_str(), NULL, false, contact);
        }
        if (data){
            string charset = "us-ascii";
            m_socket->readBuffer.incReadPos(4);
            TlvList tlvs(m_socket->readBuffer);
            Tlv *tlvInfo = tlvs(0x02);
            if (tlvInfo){
                QString info = convert(tlvInfo, tlvs, 0x01);
                if (info.left(6).upper() == "<HTML>")
                    info = info.mid(6);
                if (info.right(7).upper() == "</HTML>")
                    info = info.left(info.length() - 7);
                if (set_str(&data->About.ptr, info.utf8())){
                    data->ProfileFetch.bValue = true;
                    if (contact){
                        Event e(EventContactChanged, contact);
                        e.process();
                    }else{
                        Event e(EventClientChanged, this);
                        e.process();
                    }
                }
            }
            Tlv *tlvAway = tlvs(0x04);
            if (tlvAway){
                QString info = convert(tlvAway, tlvs, 0x03);
                set_str(&data->AutoReply.ptr, info.utf8());
                Event e(EventClientChanged, contact);
                e.process();
            }
        }
        break;
    case ICQ_SNACxLOC_DIRxINFO:
        if (isOwnData(screen.c_str())){
            data = &this->data.owner;
        }else{
            data = findInfoRequest(seq, contact);
        }
        if (data){
            bool bChanged = false;
            unsigned country = 0;
            m_socket->readBuffer.incReadPos(4);
            TlvList tlvs(m_socket->readBuffer);
            bChanged |= extractInfo(tlvs, 0x01, &data->FirstName.ptr);
            bChanged |= extractInfo(tlvs, 0x02, &data->LastName.ptr);
            bChanged |= extractInfo(tlvs, 0x03, &data->MiddleName.ptr);
            bChanged |= extractInfo(tlvs, 0x04, &data->Maiden.ptr);
            bChanged |= extractInfo(tlvs, 0x07, &data->State.ptr);
            bChanged |= extractInfo(tlvs, 0x08, &data->City.ptr);
            bChanged |= extractInfo(tlvs, 0x0C, &data->Nick.ptr);
            bChanged |= extractInfo(tlvs, 0x0D, &data->Zip.ptr);
            bChanged |= extractInfo(tlvs, 0x21, &data->Address.ptr);
            Tlv *tlvCountry = tlvs(0x06);
            if (tlvCountry){
                const char *code = *tlvCountry;
                for (const ext_info *c = getCountryCodes(); c->nCode; c++){
                    QString name(c->szName);
                    if (name.upper() == code){
                        country = c->nCode;
                        break;
                    }
                }
            }
            if (country != data->Country.value){
                data->Country.value = country;
                bChanged = true;
            }
            data->ProfileFetch.bValue = true;
            if (bChanged){
                if (contact){
                    Event e(EventContactChanged, contact);
                    e.process();
                }else{
                    Event e(EventClientChanged, this);
                    e.process();
                }
            }
        }
        break;
    case ICQ_SNACxLOC_RESPONSExSETxINFO:
        break;
    default:
        log(L_WARN, "Unknown location family type %04X", type);
    }
}

void ICQClient::locationRequest()
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxRIGHTS);
    sendPacket();
}

#define cap_id   0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define cap_str  0xbc, 0xd2, 0x00, 0x04, 0xac, 0x96, 0xdd, 0x96

#define cap_mid  0x4c, 0x7f, 0x11, 0xd1
#define cap_mstr 0x4f, 0xe9, 0xd3, 0x11

const capability arrCapabilities[] =
    {
        // CAP_DIRECT
        { 0x09, 0x46, 0x13, 0x44, cap_mid, cap_id },
        // CAP_RTF
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92 },
        // CAP_SRV_RELAY
        { 0x09, 0x46, 0x13, 0x49, cap_mid, cap_id },
        // CAP_UTF
        { 0x09, 0x46, 0x13, 0x4e, cap_mid, cap_id },
        // CAP_TYPING
        { 0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd,
          0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3 },
        // CAP_AIM_SUPPORT
        { 0x09, 0x46, 0x13, 0x4d, cap_mid, cap_id },
        // CAP_SIM
        { 'S', 'I', 'M', ' ', 'c', 'l', 'i', 'e',
          'n', 't', ' ', ' ', 0, 0, 0, 0 },
        // CAP_STR_2001
        { 0xa0, 0xe9, 0x3f, 0x37, cap_mstr, cap_str },
        // CAP_IS_2001
        { 0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8,
          0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf },
        // CAP_TRILLIAN
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x09 },
        // CAP_TRIL_CRYPT
        { 0xf2, 0xe7, 0xc7, 0xf4, 0xfe, 0xad, 0x4d, 0xfb,
          0xb2, 0x35, 0x36, 0x79, 0x8b, 0xdf, 0x00, 0x00 },
        // CAP_MACICQ
        { 0xdd, 0x16, 0xf2, 0x02, 0x84, 0xe6, 0x11, 0xd4,
          0x90, 0xdb, 0x00, 0x10, 0x4b, 0x9b, 0x4b, 0x7d },
        // CAP_AIM_CHAT
        { 0x74, 0x8f, 0x24, 0x20, 0x62, 0x87, 0x11, 0xd1, cap_id },
        // CAP_AIM_BUDDYCON
        { 0x09, 0x46, 0x13, 0x46, cap_mid, cap_id },
        // CAP_AIM_IMIMAGE
        { 0x09, 0x46, 0x13, 0x45, cap_mid, cap_id },
        // CAP_AIM_SENDFILE
        { 0x09, 0x46, 0x13, 0x43, cap_mid, cap_id },
        // CAP_AIM_BUDDY_LIST
        { 0x09, 0x46, 0x13, 0x4b, cap_mid, cap_id },
        // CAP_MICQ
        { 'm', 'I', 'C', 'Q', ' ', (unsigned char)'©', 'R', '.',
          'K', ' ', '.', ' ', 0, 0, 0, 0 },
        // CAP_LICQ
        { 'L', 'i', 'c', 'q', ' ', 'c', 'l', 'i',
          'e', 'n', 't', ' ', 0, 0, 0, 0 },
        // CAP_SIMOLD
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x00 },
        // CAP_NULL
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };

const capability *ICQClient::capabilities = arrCapabilities;

#ifndef VERSION
#define VERSION  "0.1"
#endif

static unsigned char get_ver(const char *&v)
{
    if (v == NULL)
        return 0;
    char c = (char)atol(v);
    v = strchr(v, '.');
    if (v)
        v++;
    return c;
}

static bool isWide(const QString &text)
{
    QString m = QString::fromUtf8(text);
    for (int i = 0; i < (int)(m.length()); i++)
        if (m[i].unicode() > 0x7F)
            return true;
    return true;
}

static bool isWide(const char *text)
{
    if ((text == NULL) || (*text == 0))
        return false;
    return isWide(QString::fromUtf8(text));
}

void ICQClient::encodeString(const char *str, unsigned short nTlv, bool bWide)
{
    if ((str == NULL) || (*str == 0)){
        m_socket->writeBuffer.tlv(nTlv, "");
        return;
    }
    QString m;
    if (str)
        m = QString::fromUtf8(str);
    if (bWide){
        unsigned short *unicode = new unsigned short[m.length()];
        unsigned short *t = unicode;
        for (int i = 0; i < (int)(m.length()); i++)
            *(t++) = htons(m[i].unicode());
        m_socket->writeBuffer.tlv(nTlv, (char*)unicode, (unsigned short)(m.length() * sizeof(unsigned short)));
        delete[] unicode;
    }else{
        m_socket->writeBuffer.tlv(nTlv, m.latin1());
    }
}

void ICQClient::encodeString(const QString &m, const char *type, unsigned short charsetTlv, unsigned short infoTlv)
{
    bool bWide = isWide(m);
    string content_type = type;
    content_type += "; charset=\"";
    if (bWide){
        unsigned short *unicode = new unsigned short[m.length()];
        unsigned short *t = unicode;
        for (int i = 0; i < (int)(m.length()); i++)
            *(t++) = htons(m[i].unicode());
        content_type += "unicode-2\"";
        m_socket->writeBuffer.tlv(charsetTlv, content_type.c_str());
        m_socket->writeBuffer.tlv(infoTlv, (char*)unicode, (unsigned short)(m.length() * sizeof(unsigned short)));
        delete[] unicode;
    }else{
        content_type += "us-ascii\"";
        m_socket->writeBuffer.tlv(charsetTlv, content_type.c_str());
        m_socket->writeBuffer.tlv(infoTlv, m.latin1());
    }
}

void ICQClient::sendCapability(const char *away_msg)
{
    Buffer cap;
    capability c;
    memcpy(c, capabilities[CAP_SIM], sizeof(c));
    const char *ver = VERSION;
    unsigned char *pack_ver = c + sizeof(capability) - 4;
    *(pack_ver++) = get_ver(ver);
    *(pack_ver++) = get_ver(ver);
    *(pack_ver++) = get_ver(ver);
    unsigned char os_ver;
#ifdef WIN32
    os_ver = 0x80;
#else
#ifdef QT_MACOSX_VERSION
os_ver = 0x40;
#else
os_ver = 0;
#endif
#endif
    *(pack_ver++) = os_ver;
    if (m_bAIM){
        cap.pack((char*)capabilities[CAP_AIM_CHAT], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_BUDDYCON], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_IMIMAGE], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_SENDFILE], sizeof(capability));
        cap.pack((char*)capabilities[CAP_AIM_BUDDYLIST], sizeof(capability));
    }else{
        cap.pack((char*)capabilities[CAP_DIRECT], sizeof(capability));
        cap.pack((char*)capabilities[CAP_SRV_RELAY], sizeof(capability));
        if (getSendFormat() <= 1)
            cap.pack((char*)capabilities[CAP_UTF], sizeof(capability));
        if (getSendFormat() == 0)
            cap.pack((char*)capabilities[CAP_RTF], sizeof(capability));
    }
    if (getTypingNotification())
        cap.pack((char*)capabilities[CAP_TYPING], sizeof(capability));
    cap.pack((char*)capabilities[CAP_AIM_SUPPORT], sizeof(capability));
    cap.pack((char*)c, sizeof(c));
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    if (m_bAIM){
        if (data.owner.ProfileFetch.bValue){
            QString profile;
            if (data.owner.About.ptr)
                profile = QString::fromUtf8(data.owner.About.ptr);
            profile = QString("<HTML>") + profile + "</HTML>";
            encodeString(profile, "text/aolrtf", 1, 2);
        }
        if (away_msg)
            encodeString(QString::fromUtf8(away_msg), "text/plain", 3, 4);
    }
    m_socket->writeBuffer.tlv(0x0005, cap);
    if (m_bAIM)
        m_socket->writeBuffer.tlv(0x0006, "\x00\x04\x00\x02\x00\x02", 6);
    sendPacket();
}

void ICQClient::setAwayMessage(const char *msg)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    if (msg){
        encodeString(QString::fromUtf8(msg), "text/plain", 3, 4);
    }else{
        m_socket->writeBuffer.tlv(0x0004);
    }
    sendPacket();
}

void ICQClient::fetchProfile(ICQUserData *data)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxUSERxINFO, true);
    m_socket->writeBuffer << (unsigned short)0x0001;
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    sendPacket();
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxDIRxINFO, true);
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    sendPacket();
    m_info_req.insert(INFO_REQ_MAP::value_type(m_nMsgSequence, screen(data)));
}

void ICQClient::fetchAwayMessage(ICQUserData *data)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxUSERxINFO, true);
    m_socket->writeBuffer << (unsigned short)0x0003;
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    sendPacket();
}

void ICQClient::fetchProfiles()
{
    if (data.owner.ProfileFetch.bValue == 0)
        fetchProfile(&data.owner);
    Contact *contact;
    ContactList::ContactIterator itc;
    while ((contact = ++itc) != NULL){
        ICQUserData *data;
        ClientDataIterator itd(contact->clientData, this);
        while ((data = (ICQUserData*)(++itd)) != NULL){
            if (data->Uin.value || data->ProfileFetch.bValue)
                continue;
            fetchProfile(data);
        }
    }
}

ICQUserData *ICQClient::findInfoRequest(unsigned short seq, Contact *&contact)
{
    INFO_REQ_MAP::iterator it = m_info_req.find(seq);
    if (it == m_info_req.end()){
        log(L_WARN, "Info req %u not found", seq);
        return NULL;
    }
    string screen = (*it).second;
    m_info_req.erase(it);
    return findContact(screen.c_str(), NULL, false, contact);
}

void ICQClient::setAIMInfo(ICQUserData *data)
{
    if (getState() != Connected)
        return;
    bool bWide = isWide(data->FirstName.ptr) ||
                 isWide(data->LastName.ptr) ||
                 isWide(data->MiddleName.ptr) ||
                 isWide(data->Maiden.ptr) ||
                 isWide(data->Nick.ptr) ||
                 isWide(data->Zip.ptr) ||
                 isWide(data->Address.ptr) ||
                 isWide(data->City.ptr);
    string country;
    for (const ext_info *e = getCountryCodes(); e->szName; e++){
        if (e->nCode == data->Country.value){
            country = e->szName;
            break;
        }
    }
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxDIRxINFO);
    string encoding = bWide ? "unicode-2-0" : "us-ascii";
    m_socket->writeBuffer.tlv(0x1C, encoding.c_str());
    m_socket->writeBuffer.tlv(0x0A, (unsigned short)0x01);
    encodeString(data->FirstName.ptr, 0x01, bWide);
    encodeString(data->LastName.ptr, 0x02, bWide);
    encodeString(data->MiddleName.ptr, 0x03, bWide);
    encodeString(data->Maiden.ptr, 0x04, bWide);
    encodeString(country.c_str(), 0x06, bWide);
    encodeString(data->Address.ptr, 0x07, bWide);
    encodeString(data->City.ptr, 0x08, bWide);
    encodeString(data->Nick.ptr, 0x0C, bWide);
    encodeString(data->Zip.ptr, 0x0D, bWide);
    encodeString(data->State.ptr, 0x21, bWide);
    sendPacket();

    ICQUserData *ownerData = &this->data.owner;
    set_str(&ownerData->FirstName.ptr, data->FirstName.ptr);
    set_str(&ownerData->LastName.ptr, data->LastName.ptr);
    set_str(&ownerData->MiddleName.ptr, data->MiddleName.ptr);
    set_str(&ownerData->Maiden.ptr, data->Maiden.ptr);
    set_str(&ownerData->Address.ptr, data->Address.ptr);
    set_str(&ownerData->City.ptr, data->City.ptr);
    set_str(&ownerData->Nick.ptr, data->Nick.ptr);
    set_str(&ownerData->Zip.ptr, data->Zip.ptr);
    set_str(&ownerData->State.ptr, data->State.ptr);
}

void ICQClient::setProfile(ICQUserData *data)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    QString profile;
    if (data->About.ptr)
        profile = QString::fromUtf8(data->About.ptr);
    profile = QString("<HTML>") + profile + "</HTML>";
    encodeString(profile, "text/aolrtf", 1, 2);
    sendPacket();
}

