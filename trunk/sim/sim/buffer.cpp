/***************************************************************************
                          buffer.cpp  -  description
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

#include "buffer.h"

#include <stdio.h>

#include <qcstring.h>
#include <qptrlist.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <string>   // fixme!
using namespace SIM;
using namespace std;

#ifdef WORDS_BIGENDIAN

#define SWAP_S(s)   s = ((s&0xFF)<<8) + ((s&0xFF00)>>8);  
#define SWAP_L(s)   s = ((s&0xFF)<<24) + ((s&0xFF00)<<8) + ((s&0xFF0000)>>8) + ((s&0xFF000000)>>24); 

#else

#define SWAP_S(s)
#define SWAP_L(s)

#endif

Tlv::Tlv(unsigned short num, unsigned short size, const char *data)
        : m_nNum(num), m_nSize(size)
{
    m_data = new char[size + 1];
    memcpy(m_data, data, size);
    m_data[size] = 0;
}

Tlv::~Tlv()
{
    delete[] m_data;
}

Tlv::operator unsigned short ()
{
    return (unsigned short)((m_nSize >= 2) ? htons(*((unsigned short*)m_data)) : 0);
}

Tlv::operator unsigned long ()
{
    return (m_nSize >= 4) ? htonl(*((unsigned long*)m_data)) : 0;
}

TlvList::TlvList()
{
    setAutoDelete(true);
}

TlvList::TlvList(Buffer &b, unsigned nTlvs)
{
    setAutoDelete(true);
    for (unsigned n = 0; (b.readPos() < b.size()) && (n < nTlvs); n++){
        unsigned short num, size;
        b >> num >> size;
        if (b.readPos() + size > b.size())
            break;
        append(new Tlv(num, size, b.data(b.readPos())));
        b.incReadPos(size);
    }
}

Tlv *TlvList::operator()(unsigned short num)
{
    for(uint i = 0; i < count(); i++) {
        if (at(i)->Num() == num)
            return at(i);
    }
    return NULL;
}

Tlv *TlvList::operator[](unsigned n)
{
    return at(n);
}

Buffer::Buffer(unsigned size)
        : QByteArray(size)
{
    init(size);
}

Buffer::Buffer(Tlv &tlv)
        : QByteArray(tlv.Size())
{
    init(tlv.Size());
    pack((char*)tlv, tlv.Size());
}

Buffer::~Buffer()
{
}

void Buffer::init(unsigned size)
{
    resize(size);   
    m_posRead = 0;
    m_posWrite = 0;
    m_packetStartPos = 0;
}

void Buffer::incReadPos(int n)
{
    m_posRead += n;
    if (m_posRead > m_posWrite)
        m_posRead = m_posWrite;
}

bool Buffer::add(uint addSize)
{
    return(resize(size()+addSize));
}

bool Buffer::resize(uint size)
{
    bool bRet = QByteArray::resize(size);
    if (m_posWrite > size)
        m_posWrite = size;
    if (m_posRead > size)
        m_posRead = size;
    return bRet;
}

void Buffer::setWritePos(unsigned n)
{
    m_posWrite = n;
    if (m_posRead > m_posWrite) m_posRead = m_posWrite;
    if (m_posWrite > size()){
        resize(m_posWrite);
    }
}

void Buffer::setReadPos(unsigned n)
{
    if (n > m_posWrite)
        n = m_posWrite;
    m_posRead = n;
}

void Buffer::pack(const char *d, unsigned s)
{
    if(s == 0)
		return;
    if(m_posWrite+s > size())
        resize(m_posWrite+s);
    memcpy(data() + m_posWrite, d, s);
    m_posWrite += s;
}

unsigned Buffer::unpack(char *d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    memcpy(d, data() + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

unsigned Buffer::unpack(QString &d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    d = QString::fromAscii(data() + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

QString Buffer::unpackScreen()
{
    char len;
    QString res;

    *this >> len;
    /* 13 isn't right, AIM allows 16. But when we get a longer
    name, we *must* unpack them if we won't lose the TLVs
    behind the Screenname ... */
    if (len > 16)
        log(L_DEBUG,"Too long Screenname! Length: %d",len);
    res.fill('\x00',len);
    unpack(res, len);
    return res;
}

void Buffer::unpack(std::string &str)
{
    QCString cstr;
    unsigned short s;
    unpack(s);
    str.clear();
    if (s == 0)
        return;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    cstr.fill('\0',s);
    unpack(cstr.data(), s);
    str = cstr;
}

void Buffer::unpackStr(std::string &str)
{
    QCString cstr;
    unsigned short s;
    *this >> s;
    str.clear();
    if (s == 0)
        return;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    cstr.fill('\0',s);
    unpack(cstr.data(), s);
    str = cstr;
}

void Buffer::unpack(QString &str)
{
    unsigned short s;
    str = "";
    unpack(s);
    if (s == 0)
        return;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    unpack(str, s);
}

void Buffer::unpackStr(QString &str)
{
    unsigned short s;
    str = "";
    *this >> s;
    if (s == 0)
        return;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    unpack(str, s);
}

void Buffer::unpackStr32(std::string &str)
{
    QCString cstr;
    unsigned long s;
    *this >> s;
    s = htonl(s);
    str.clear();
    if (s == 0)
        return;
    if (s > size() - m_posRead)
        s = size() - m_posRead;
    cstr.fill('\0',s);
    unpack(cstr.data(), s);
    str = cstr;
}

Buffer &Buffer::operator >> (string &str)
{
    unsigned short s;
    *this >> s;
    s = htons(s);
    str.erase();
    if (s){
        if (s > size() - m_posRead)
            s = (unsigned short)(size() - m_posRead);
        str.append((unsigned)s, '\x00');
        unpack((char*)str.c_str(), s);
    }
    return *this;
}

Buffer &Buffer::operator >> (QString &str)
{
    unsigned short s;
    *this >> s;
    s = htons(s);
    str = "";
    if (s){
        if (s > size() - m_posRead)
            s = (unsigned short)(size() - m_posRead);
        char *buf = new char[s+1];
        unpack(buf, s);
        str = QString::fromAscii(buf,s);
        delete[] buf;
    }
    return *this;
}

Buffer &Buffer::operator >> (char &c)
{
    if (unpack(&c, 1) != 1) c = 0;
    return *this;
}

Buffer &Buffer::operator >> (unsigned short &c)
{
    if (unpack((char*)&c, 2) != 2) c = 0;
    c = htons(c);
    return *this;
}

Buffer &Buffer::operator >> (unsigned long &c)
{
    if (unpack((char*)&c, 4) != 4) c = 4;
    c = htonl(c);
    return *this;
}

Buffer &Buffer::operator >> (int &c)
{
    if (unpack((char*)&c, 4) != 4) c = 4;
    c = htonl(c);
    return *this;
}

Buffer &Buffer::operator >> (char **s)
{
    unsigned short size;
    *this >> size;
    size = htons(size);
    if (size){
        string str;
        str.append(size, '\x00');
        unpack((char*)str.c_str(), size);
        set_str(s, str.c_str());
    }else{
        set_str(s, NULL);
    }
    return *this;
}

void Buffer::unpack(char &c)
{
    *this >> c;
}

void Buffer::unpack(unsigned char &c)
{
    *this >> c;
}

void Buffer::unpack(unsigned short &c)
{
    if (unpack((char*)&c, 2) != 2) c = 0;
    SWAP_S(c)
}

void Buffer::unpack(unsigned long &c)
{
    // FIXME: This needs to be rewritten for 64-bit machines.
    // Kludge for now.
    c = 0;
    if (unpack((char*)&c, 4) != 4) c = 0;
    SWAP_L(c);
}

void Buffer::pack(const string &s)
{
    unsigned short size = (unsigned short)(s.size());
    *this << size;
    pack(s.c_str(), size);
}

void Buffer::pack(unsigned short s)
{
    SWAP_S(s)
    pack((char*)&s, 2);
}

void Buffer::pack(unsigned long s)
{
    SWAP_L(s)
    pack((char*)&s, 4);
}

void Buffer::packStr32(const char *s)
{
    if (s) {
        unsigned long size = strlen(s);
        pack(size);
        pack(s, strlen(s));
    } else {
        pack((unsigned long)0);
        pack("", 0);
    }
}

Buffer &Buffer::operator << (const Buffer &b)
{
    unsigned short size = (unsigned short)(b.size() - b.readPos());
    *this << (unsigned short)htons(size);
    pack(b.data(b.readPos()), size);
    return *this;
}

void Buffer::pack32(const Buffer &b)
{
    unsigned long size = b.size() - b.readPos();
    *this << (unsigned long)htonl(size);
    pack(b.data(b.readPos()), size);
}

Buffer &Buffer::operator << (const QString &s)
{
    QCString utf8 = s.utf8();
	unsigned short size = (unsigned short)(utf8.length() + 1);
    *this << (unsigned short)htons(size);
    pack(utf8, size);
    return *this;
}

Buffer &Buffer::operator << (char **str)
{
    QString s;
    if (*str)
        s = *str;
    return *this << s;
}

Buffer &Buffer::operator << (const char *str)
{
    if(!str)
		return *this;
    pack(str, strlen(str));
    return *this;
}

Buffer &Buffer::operator << (char c)
{
    pack(&c, 1);
    return *this;
}

Buffer &Buffer::operator << (bool b)
{
    char c = b ? (char)1 : (char)0;
    pack(&c, 1);
    return *this;
}

Buffer &Buffer::operator << (unsigned short c)
{
    c = htons(c);
    pack((char*)&c, 2);
    return *this;
}

Buffer &Buffer::operator << (unsigned long c)
{
    c = htonl(c);
    pack((char*)&c, 4);
    return *this;
}

void Buffer::packScreen(const QString &screen)
{
    char len = (char)(strlen(screen.latin1()));
    pack(&len, 1);
    pack(screen.latin1(), len);
}

bool Buffer::scan(const char *substr, string &res)
{
    char c = *substr;
    for (unsigned pos = readPos(); pos < writePos(); pos++){
        if (*data(pos) != c)
            continue;
        const char *sp = substr;
        for (unsigned pos1 = pos; *sp; pos1++, sp++){
            if (pos1 >= writePos())
                break;
            if (*data(pos1) != *sp)
                break;
        }
        if (*sp == 0){
            res = "";
            if (pos - readPos()){
                res.append(pos - readPos(), '\x00');
                unpack((char*)res.c_str(), pos - readPos());
            }
            incReadPos(pos + strlen(substr) - readPos());
            return true;
        }
    }
    return false;
}

bool Buffer::scan(const char *substr, QString &res)
{
    char c = *substr;
    for (unsigned pos = readPos(); pos < writePos(); pos++){
        if (*data(pos) != c)
            continue;
        const char *sp = substr;
        for (unsigned pos1 = pos; *sp; pos1++, sp++){
            if (pos1 >= writePos())
                break;
            if (*data(pos1) != *sp)
                break;
        }
        if (*sp == 0){
            res = "";
            if (pos - readPos()){
                int size = pos - readPos();
                unpack(res, size);
            }
            incReadPos(pos + strlen(substr) - readPos());
            return true;
        }
    }
    return false;
}

void Buffer::tlv(unsigned short n, const char *data, unsigned short len)
{
    *this << n << len;
    pack(data, len);
}

void Buffer::tlvLE(unsigned short n, const char *data, unsigned short len)
{
    pack(n);
    pack(len);
    pack(data, len);
}

void Buffer::tlv(unsigned short n, const char *data)
{
    if (data == NULL)
        data = "";
    tlv(n, data, (unsigned short)strlen(data));
}

void Buffer::tlvLE(unsigned short n, const char *data)
{
    if (data == NULL)
        data = "";
    unsigned short len = strlen(data) + 1;
    pack(n);
    pack((unsigned short)(len + 2));
    pack(len);
    pack(data, len);
}

void Buffer::tlv(unsigned short n, unsigned short c)
{
    c = htons(c);
    tlv(n, (char*)&c, 2);
}

void Buffer::tlvLE(unsigned short n, unsigned short c)
{
    pack(n);
    pack((unsigned short)2);
    pack(c);
}

void Buffer::tlv(unsigned short n, unsigned long c)
{
    c = htonl(c);
    tlv(n, (char*)&c, 4);
}

void Buffer::tlvLE(unsigned short n, unsigned long c)
{
    pack(n);
    pack((unsigned short)4);
    pack(c);
}

void Buffer::packetStart()
{
    m_packetStartPos = writePos();
}

unsigned long Buffer::packetStartPos()
{
    return m_packetStartPos;
}

Buffer &Buffer::operator << (TlvList &tlvList)
{
    unsigned size = 0;
    for (uint i = 0; i < tlvList.count(); i++)
        size += tlvList.at(i)->Size() + 4;
    *this << (unsigned short)size;
    for (uint i = 0; i < tlvList.count(); i++) {
        Tlv *tlv = tlvList.at(i);
        *this << tlv->Num() << (int)tlv->Size();
        pack(*tlv, tlv->Size());
    }
    return *this;
}

void Buffer::fromBase64(Buffer &from)
{
    unsigned n = 0;
    unsigned tmp2 = 0;
    for (;;) {
        char res[3];
        char c;
        from >> c;
        if (c == 0)
            break;
        char tmp = 0;
        if ((c >= 'A') && (c <= 'Z')) {
            tmp = (char)(c - 'A');
        } else if ((c >= 'a') && (c <= 'z')) {
            tmp = (char)(26 + (c - 'a'));
        } else if ((c >= '0') && (c <= 57)) {
            tmp = (char)(52 + (c - '0'));
        } else if (c == '+') {
            tmp = 62;
        } else if (c == '/') {
            tmp = 63;
        } else if ((c == '\r') || (c == '\n')) {
            continue;
        } else if (c == '=') {
            if (n == 3) {
                res[0] = (char)((tmp2 >> 10) & 0xff);
                res[1] = (char)((tmp2 >> 2) & 0xff);
                pack(res, 2);
            } else if (n == 2) {
                res[0] = (char)((tmp2 >> 4) & 0xff);
                pack(res, 1);
            }
            break;
        }
        tmp2 = ((tmp2 << 6) | (tmp & 0xff));
        n++;
        if (n == 4) {
            res[0] = (char)((tmp2 >> 16) & 0xff);
            res[1] = (char)((tmp2 >> 8) & 0xff);
            res[2] = (char)(tmp2 & 0xff);
            pack(res, 3);
            tmp2 = 0;
            n = 0;
        }
    }
}

static const char alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

void Buffer::toBase64(Buffer &from)
{
    unsigned char b[3];
    char res[4];
    unsigned tmp;

    while (from.readPos() + 3 < from.size()){
        from.unpack((char*)b, 3);
        tmp = (b[0] << 16) | (b[1] << 8) | b[2];
        res[0] = alphabet[(tmp >> 18) & 0x3F];
        res[1] = alphabet[(tmp >> 12) & 0x3F];
        res[2] = alphabet[(tmp >> 6) & 0x3F];
        res[3] = alphabet[tmp & 0x3F];
        pack(res, 4);
    }

    switch(from.size() - from.readPos()){
    case 2:
        from.unpack((char*)b, 2);
        tmp = (b[0] << 16) | (b[1] << 8);
        res[0] = alphabet[(tmp >> 18) & 0x3F];
        res[1] = alphabet[(tmp >> 12) & 0x3F];
        res[2] = alphabet[(tmp >> 6) & 0x3F];
        res[3] = '=';
        pack(res, 4);
        break;
    case 1:
        from.unpack((char*)b, 1);
        tmp = b[0] << 16;
        res[0] = alphabet[(tmp >> 18) & 0x3F];
        res[1] = alphabet[(tmp >> 12) & 0x3F];
        res[2] = res[3] = '=';
        pack(res, 4);
        break;
    }
}

static int findStartSection(const Buffer *pBuf, unsigned start)
{
    int idx = start;

    do {
        idx = pBuf->find( '[', idx);
        if(idx == -1)
            return -1;
        if( idx == 0 || pBuf->at( idx - 1 ) == '\n' )
            return idx;
        idx++;
    } while(true);
}

static int findEndSection(const Buffer *pBuf, unsigned start)
{
    int idx = start;

    do {
        idx = pBuf->find( ']', idx);
        if(idx == -1)
            return -1;
        if( idx == (int)pBuf->size() - 1 || pBuf->at( idx + 1 ) == '\n' )
            return idx;
        idx++;
    } while(true);
}

QString Buffer::getSection(bool bSkip)
{
    QString str;
    unsigned start = m_posRead;
    unsigned end = m_posRead;

    if( bSkip )
        start = findStartSection(this, m_posRead + 1);
    if( start == -1 )
        return str;
    start = findStartSection( this, start );
    end   = findEndSection( this, start );
    if( start == -1 || end == -1 )
        return str;
    m_startSection = m_posRead = start;

    str = QString::fromLocal8Bit( data() + start + 1, end - start - 1 );

    m_posRead = end + 1;
    if ( m_posRead < size() )
        if ( at(m_posRead) == '\n' )
            m_posRead++;
    if ( m_posRead >= size() )
        m_posRead = size() - 1;
    m_posWrite = findStartSection( this, end );
    if( m_posWrite == -1 )
        m_posWrite = size() - 1;

    return str;
}

Buffer &Buffer::operator = (const QByteArray &ba)
{
    int size = ba.size();
    assign(ba);
    resize(size+1);
    at(size) = '\0';
    m_posWrite = size;
    return *this;
}

// *********************************************
// ConfigBuffer
// *********************************************
ConfigBuffer::ConfigBuffer(const QString &str, unsigned pos)
: m_posRead(pos), m_startSection(0), m_posNextSection(~0U), m_posReadSave(0)
{
	QString *that = static_cast<QString*>(this);
	*that = str;
}

ConfigBuffer::ConfigBuffer(QIODevice *io, unsigned pos)
: m_posRead(pos), m_startSection(0), m_posNextSection(~0U), m_posReadSave(0)
{
	QString *that = static_cast<QString*>(this);
	if( io ) {
		QTextStream ts( io );
        ts.setEncoding( QTextStream::UnicodeUTF8 );
		*that = ts.read();
	}
}

QString ConfigBuffer::getSection(bool bSkip)
{
    QString str;
    unsigned start = m_posRead;
    unsigned end = m_posRead;

    if( bSkip )
        start = findStartSection( m_posRead + 1 );
    if( start == -1 )
        return str;
    start = findStartSection( start );
    end   = findEndSection( start );
    if( start == -1 || end == -1 )
        return str;
    m_startSection = m_posRead = start;

    str = mid( start + 1, end - start - 1 );

    m_posRead = end + 1;
    if ( m_posRead < length() )
        if ( at(m_posRead) == '\n' )
            m_posRead++;
    if ( m_posRead >= length() )
        m_posRead = length() - 1;

	m_posNextSection = findStartSection( end );
    if( m_posNextSection == -1 )
        m_posNextSection = length() - 1;

    return str;
}

QString ConfigBuffer::getLine()
{
    QString str;
    if(m_posNextSection==~0U)
		getSection();
	if (m_posRead >= m_posNextSection)
        return str;
    int idx = find( '\n', m_posRead );
    if( idx==-1 )
        idx = find( '\0', m_posRead );
    if( idx==-1 )
        idx = length();
    str = mid( m_posRead, idx - m_posRead );
    m_posRead += str.length();
    if ( at(m_posRead) == '\n' )
        m_posRead++;
    if ( m_posRead >= length() )
        m_posRead = length();
    return str;
}

int ConfigBuffer::findStartSection(unsigned start)
{
    int idx = start;

    do {
        idx = find( '[', idx);
        if(idx == -1)
            return -1;
        if( idx == 0 || at( idx - 1 ) == '\n' )
            return idx;
        idx++;
    } while(true);
}

int ConfigBuffer::findEndSection(unsigned start)
{
    int idx = start;

    do {
        idx = find( ']', idx);
        if(idx == -1)
            return -1;
        if( idx == (int)length() - 1 || at( idx + 1 ) == '\n' )
            return idx;
        idx++;
    } while(true);
}


EXPORT void log_packet(Buffer &buf, bool bOut, unsigned packet_id, const char *add_info)
{
    LogInfo li;
    li.log_level = bOut ? L_PACKET_OUT : L_PACKET_IN;
    li.log_info  = &buf;
    li.packet_id = packet_id;
    li.add_info  = add_info;
    Event e(EventLog, &li);
    e.process();
}
