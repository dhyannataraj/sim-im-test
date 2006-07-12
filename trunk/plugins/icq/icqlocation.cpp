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

#include <qtextcodec.h>
#ifdef Q_OS_WIN32
# include <winsock.h>
#else
# include <netinet/in.h>
#endif

using std::string;
using namespace SIM;

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
            log(L_WARN, "Unknown encoding %s", charset.c_str());
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
                break;	/* Because we won't find tlv(0x03) which is
                           "since online" instead of encoding... */                            
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
    sendPacket(true);
}

#define cap_id   0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define cap_none 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define cap_str  0xbc, 0xd2, 0x00, 0x04, 0xac, 0x96, 0xdd, 0x96

#define cap_mid  0x4c, 0x7f, 0x11, 0xd1
#define cap_mstr 0x4f, 0xe9, 0xd3, 0x11
#define cap_aim  0x09, 0x46, 0x13

// must be synced with cap_id_t enum in icqclient.h
const capability arrCapabilities[] =
    {
        // CAP_AIM_VOICE
        { cap_aim, 0x41, cap_mid, cap_id },
        // CAP_AIM_SENDFILE
        { cap_aim, 0x43, cap_mid, cap_id },
        // CAP_DIRECT
        { cap_aim, 0x44, cap_mid, cap_id },
        // CAP_AIM_IMIMAGE
        { cap_aim, 0x45, cap_mid, cap_id },
        // CAP_AIM_BUDDYCON
        { cap_aim, 0x46, cap_mid, cap_id },
        // CAP_AIM_STOCKS
        { cap_aim, 0x47, cap_mid, cap_id },
        // CAP_AIM_GETFILE
        { cap_aim, 0x48, cap_mid, cap_id },
        // CAP_SRV_RELAY
        { cap_aim, 0x49, cap_mid, cap_id },
        // CAP_AIM_GAMES
        { cap_aim, 0x4a, cap_mid, cap_id },
        // CAP_AIM_BUDDYLIST
        { cap_aim, 0x4b, cap_mid, cap_id },
        // CAP_AVATAR
        { cap_aim, 0x4c, cap_mid, cap_id },
        // CAP_AIM_SUPPORT
        { cap_aim, 0x4d, cap_mid, cap_id },
        // CAP_UTF
        { cap_aim, 0x4e, cap_mid, cap_id },
        // CAP_RTF
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92 },
        // CAP_TYPING
        { 0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd,
          0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3 },
        // CAP_SIM
        { 'S', 'I', 'M', ' ', 'c', 'l', 'i', 'e',
          'n', 't', ' ', ' ', 0, 0, 0, 0 },
        // CAP_STR_2001
        { 0xa0, 0xe9, 0x3f, 0x37, cap_mstr, cap_str },
        // CAP_STR_2002
        { 0x10, 0xcf, 0x40, 0xd1, cap_mstr, cap_str },
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
        // CAP_MICQ
        { 'm', 'I', 'C', 'Q', ' ', (unsigned char)'©', 'R', '.',
          'K', ' ', '.', ' ', 0, 0, 0, 0 },
        // CAP_LICQ
        { 'L', 'i', 'c', 'q', ' ', 'c', 'l', 'i',
          'e', 'n', 't', ' ', 0, 0, 0, 0 },
        // CAP_SIMOLD
        { 0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
          0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x00 },
        // CAP_KOPETE
        { 'K', 'o', 'p', 'e', 't', 'e', ' ', 'I',
          'C', 'Q', ' ', ' ',   0,   0,   0,  0 },
        // CAP_XTRAZ
        { 0x1A, 0x09, 0x3C, 0x6C, 0xD7, 0xFD, 0x4E, 0xC5,
          0x9D, 0x51, 0xA6, 0x47, 0x4E, 0x34, 0xF5, 0xA0  },
        // CAP_IS_2002
        { 0x10, 0xcf, 0x40, 0xd1, cap_mid, cap_id },
        // CAP_MIRANDA
        { 'M', 'i', 'r', 'a', 'n', 'd', 'a', 'M', cap_none },
        // CAP_ANDRQ
        { '&', 'R', 'Q', 'i', 'n', 's', 'i', 'd',
          'e', 0, 0, 0, 0, 0, 0, 0 },
        // CAP_QIP
        { 0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 'Q',
          'I', 'P', ' ', '2', '0', '0', '5', 'a' },
        // CAP_IMSECURE
        { 'I', 'M', 's', 'e', 'c', 'u', 'r', 'e',
          'C', 'p', 'h', 'r', 0, 0, 0, 0 },
        // CAP_KXICQ
        { 0x09, 0x49, 0x13, 0x44, cap_mid, cap_id },
		// CAP_ICQ5_1
		{ 0xe3, 0x62, 0xc1, 0xe9, 0x12, 0x1a, 0x4b, 0x94,
		  0xa6, 0x26, 0x7a, 0x74, 0xde, 0x24, 0x27, 0x0d },
		// CAP_UNKNOWN - used by Trillian and some ICQ 5 clients
		{ 0x17, 0x8c, 0x2d, 0x9b, 0xda, 0xa5, 0x45, 0xbb,
		  0x8d, 0xdb, 0xf3, 0xbd, 0xbd, 0x53, 0xa1, 0x0a },
		// CAP_ICQ5_3
		{ 0x67, 0x36, 0x15, 0x15, 0x61, 0x2d, 0x4c, 0x07,
		  0x8f, 0x3d, 0xbd, 0xe6, 0x40, 0x8e, 0xa0, 0x41 },
		// CAP_ICQ5_4
		{ 0xb9, 0x97, 0x08, 0xb5, 0x3a, 0x92, 0x42, 0x02,
		  0xb0, 0x69, 0xf1, 0xe7, 0x57, 0xbb, 0x2e, 0x17 },
        // CAP_ICQ51
        { 0xb2, 0xec, 0x8f, 0x16, 0x7c, 0x6f, 0x45, 0x1b,
          0xbd, 0x79, 0xdc, 0x58, 0x49, 0x78, 0x88, 0xb9 },
        // CAP_JIMM
		{ 'J', 'i', 'm', 'm', ' ', 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0 },
/*
        // from Gaim:
        // CAP_AIM_HIPTOP
        { cap_aim, 0x23, cap_mid, cap_id },
        // CAP_AIM_ICHAT
        { 0x09, 0x46, 0x00, 0x00, cap_mid, cap_id },
        // CAP_AIM_SECUREIM
        { 0x09, 0x46, 0x00, 0x01, cap_mid, cap_id },
        // CAP_AIM_VIDEO
        { 0x09, 0x46, 0x01, 0x00, cap_mid, cap_id },
        // CAP_AIM_LIVEVIDEO
        { 0x09, 0x46, 0x01, 0x01, cap_mid, cap_id },
        // CAP_AIM_CAMERA
        { 0x09, 0x46, 0x01, 0x02, cap_mid, cap_id },
        // CAP_AIM_ICHATAV
        { 0x09, 0x46, 0x01, 0x05, cap_mid, cap_id },
        // CAP_AIM_SMS
        { 0x09, 0x46, 0x01, 0xff, cap_mid, cap_id },
        // unknown
        { 0x09, 0x46, 0xf0, 0x03, cap_mid, cap_id },
        { 0x09, 0x46, 0xf0, 0x05, cap_mid, cap_id },

        // from http://community.livejournal.com/oscardoc/12366.html:
        // HasMicrophone
        { 0x09, 0x46, 0x01, 0x03, cap_mid, cap_id },
        // RtcAudio
        { 0x09, 0x46, 0x01, 0x04, cap_mid, cap_id },
        // Aca
        { 0x09, 0x46, 0x01, 0x06, cap_mid, cap_id },
        // MultiAudio
        { 0x09, 0x46, 0x01, 0x07, cap_mid, cap_id },
        // MultiVideo
        { 0x09, 0x46, 0x01, 0x08, cap_mid, cap_id },
        // Viceroy
        { 0x09, 0x46, 0xf0, 0x04, cap_mid, cap_id },

        // unknown QIP caps:
        { 0xd3, 0xd4, 0x53, 0x19, 0x8b, 0x32, 0x40, 0x3b,
          0xac, 0xc7, 0xd1, 0xa9, 0xe2, 0xb5, 0x81, 0x3e },
        { 0x78, 0x5e, 0x8c, 0x48, 0x40, 0xd3, 0x4c, 0x65,
          0x88, 0x6f, 0x04, 0xcf, 0x3f, 0x3f, 0x43, 0xdf },
        { 0xe6, 0x01, 0xe4, 0x1c, 0x33, 0x73, 0x4b, 0xd1,
          0xbc, 0x06, 0x81, 0x1d, 0x6c, 0x32, 0x3d, 0x81 },
        { 0x61, 0xbe, 0xe0, 0xdd, 0x8b, 0xdd, 0x47, 0x5d,
          0x8d, 0xee, 0x5f, 0x4b, 0xaa, 0xcf, 0x19, 0xa7 },

        // from mICQ:
        { 0x17, 0x8c, 0x2d, 0x9b, 0xda, 0xa5, 0x45, 0xbb, 
          0x8d, 0xdb, 0xf3, 0xbd, 0xbd, 0x53, 0xa1, 0x0a },
        { 0x67, 0x36, 0x15, 0x15, 0x61, 0x2d, 0x4c, 0x07,
          0x8f, 0x3d, 0xbd, 0xe6, 0x40, 0x8e, 0xa0, 0x41 },
        { 0xe3, 0x62, 0xc1, 0xe9, 0x12, 0x1a, 0x4b, 0x94,
          0xa6, 0x26, 0x7a, 0x74, 0xde, 0x24, 0x27, 0x0d },
        { 0xb9, 0x97, 0x08, 0xb5, 0x3a, 0x92, 0x42, 0x02,
          0xb0, 0x69, 0xf1, 0xe7, 0x57, 0xbb, 0x2e, 0x17 },
        { 0xb6, 0x07, 0x43, 0x78, 0xf5, 0x0c, 0x4a, 0xc7,
          0x90, 0x92, 0x59, 0x38, 0x50, 0x2d, 0x05, 0x91 },
        { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x19, 0x04, 
          0x4a, 0x16, 0xed, 0x79, 0x2c, 0xb1, 0x71, 0x01 },
        { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xb3, 0xf8, 
          0x53, 0x44, 0x7f, 0x0d, 0x2d, 0x83, 0xbd, 0x76 },
*/
		// CAP_NULL
        { cap_none, cap_none },
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
        cap.pack((char*)capabilities[CAP_XTRAZ], sizeof(capability));
        if (getSendFormat() <= 1)
            cap.pack((char*)capabilities[CAP_UTF], sizeof(capability));
        if (getSendFormat() == 0)
            cap.pack((char*)capabilities[CAP_RTF], sizeof(capability));
    }
    if (!getDisableTypingNotification())
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
    sendPacket(true);
}

void ICQClient::setAwayMessage(const char *msg)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    if (msg){
        encodeString(QString::fromUtf8(msg), "text/plain", 3, 4);
    }else{
        m_socket->writeBuffer.tlv(0x0004);
    }
    sendPacket(true);
}

void ICQClient::fetchProfile(ICQUserData *data)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxUSERxINFO, true);
    m_socket->writeBuffer << (unsigned short)0x0001;
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    sendPacket(false);
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxDIRxINFO, true);
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    sendPacket(false);
    m_info_req.insert(INFO_REQ_MAP::value_type(m_nMsgSequence, screen(data)));
}

void ICQClient::fetchAwayMessage(ICQUserData *data)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_REQUESTxUSERxINFO, true);
    m_socket->writeBuffer << (unsigned short)0x0003;
    m_socket->writeBuffer.packScreen(screen(data).c_str());
    sendPacket(false);
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
    sendPacket(false);

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
    ownerData->Country.value = data->Country.value;
}

void ICQClient::setProfile(ICQUserData *data)
{
    snac(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_SETxUSERxINFO);
    QString profile;
    if (data->About.ptr)
        profile = QString::fromUtf8(data->About.ptr);
    profile = QString("<HTML>") + profile + "</HTML>";
    encodeString(profile, "text/aolrtf", 1, 2);
    sendPacket(false);
}

