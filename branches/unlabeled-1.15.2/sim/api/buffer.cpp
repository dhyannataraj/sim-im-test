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

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif
#include "buffer.h"

#include <stdio.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(push)
#pragma warning(disable: 4018)  
#pragma warning(disable: 4663)  
#endif
#endif

#include <vector>
using namespace std;

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(pop)
#endif
#endif


#ifdef WORDS_BIGENDIAN

#define SWAP_S(s)	s = ((s&0xFF)<<8) + ((s&0xFF00)>>8);  
#define SWAP_L(s)	s = ((s&0xFF)<<24) + ((s&0xFF00)<<8) + ((s&0xFF0000)>>8) + ((s&0xFF000000)>>24); 

#else

#define SWAP_S(s)
#define SWAP_L(s)

#endif

Tlv::operator unsigned short ()
{
    return (unsigned short)((m_nSize >= 2) ? htons(*((unsigned short*)m_data)) : 0);
}

Tlv::operator unsigned long ()
{
    return (m_nSize >= 4) ? htonl(*((unsigned long*)m_data)) : 0;
}

Buffer::Buffer(unsigned size)
        : m_alloc_size(0), m_data(NULL)
{
    init(size);
}

Buffer::Buffer(Tlv &tlv)
        : m_alloc_size(0), m_data(NULL)
{
    init(tlv.Size());
    pack((char*)tlv, tlv.Size());
}

Buffer::~Buffer()
{
    if (m_data) free(m_data);
}

void Buffer::init(unsigned size)
{
    allocate(size, 0);
    m_size = size;
    m_posRead = 0;
    m_posWrite = 0;
    m_packetStartPos = 0;
}

void Buffer::add(unsigned size)
{
    allocate(m_size + size, 0);
    m_size += size;
}

void Buffer::allocate(unsigned size, unsigned add_size)
{
    if (size <= m_alloc_size) return;
    m_alloc_size = size + add_size;
    if (m_data){
        m_data = (char*)realloc(m_data, m_alloc_size);
    }else{
        m_data = (char*)malloc(m_alloc_size);
    }
}

void Buffer::insert(unsigned size)
{
	if (size == 0)
		return;
	allocate(size, size);
	if (m_size)
		memmove(data(size), data(), m_size);
	m_size += size;
}

void Buffer::incReadPos(int n)
{
    m_posRead += n;
    if (m_posRead > m_posWrite) m_posRead = m_posWrite;
}

void Buffer::setSize(unsigned size)
{
    if (size >= m_size)
        return;
    m_size = size;
    if (m_posWrite > m_size)
        m_posWrite = m_size;
    if (m_posRead > m_size)
        m_posRead = m_size;
}

void Buffer::setWritePos(unsigned n)
{
    m_posWrite = n;
    if (m_posRead > m_posWrite) m_posRead = m_posWrite;
    if (m_posWrite > m_size){
        m_size = m_posWrite;
        allocate(m_size, 0);
    }
}

void Buffer::setReadPos(unsigned n)
{
    if (n > m_posWrite) n = m_posWrite;
    m_posRead = n;
}

void Buffer::pack(const char *d, unsigned size)
{
    allocate(m_posWrite + size, 1024);
    memcpy(m_data + m_posWrite, d, size);
    m_posWrite += size;
    if (m_posWrite > m_size) m_size = m_posWrite;
}

unsigned Buffer::unpack(char *d, unsigned size)
{
    unsigned readn = m_size - m_posRead;
    if (size < readn) readn = size;
    memcpy(d, m_data + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

string Buffer::unpackScreen()
{
    char len;
    string res;

    *this >> len;
    /* 13 isn't right, AIM allows 16. But when we get a longer
    name, we *must* unpack them if we won't lose the TLVs
    behind the Screenname ... */
    if (len > 16)
        log(L_DEBUG,"Too long Screenname! Length: %d",len);
    res.append((unsigned)len, '\x00');
    unpack((char*)res.c_str(), len);
    return res;
}

void Buffer::unpack(string &s)
{
    unsigned short size;
    unpack(size);
    s.erase();
    if (size == 0) return;
    if (size > m_size - m_posRead)
        size = (unsigned short)(m_size - m_posRead);
    s.append(size, '\x00');
    unpack((char*)s.c_str(), size);
}

void Buffer::unpackStr(string &s)
{
    unsigned short size;
    *this >> size;
    s.erase();
    if (size == 0) return;
    if (size > m_size - m_posRead)
        size = (unsigned short)(m_size - m_posRead);
    s.append(size, '\x00');
    unpack((char*)s.c_str(), size);
}

void Buffer::unpackStr32(string &s)
{
    unsigned long size;
    *this >> size;
    size = htonl(size);
    s.erase();
    if (size == 0) return;
    if (size > m_size - m_posRead)
        size = m_size - m_posRead;
    s.append(size, '\x00');
    unpack((char*)s.c_str(), size);
}

Buffer &Buffer::operator >> (string &s)
{
    unsigned short size;
    *this >> size;
    size = htons(size);
    s.erase();
    if (size){
        if (size > m_size - m_posRead)
            size = (unsigned short)(m_size - m_posRead);
        s.append((unsigned)size, '\x00');
        unpack((char*)s.c_str(), size);
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
    unsigned long size = strlen(s);
    pack(size);
    pack(s, strlen(s));
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

Buffer &Buffer::operator << (const string &s)
{
    unsigned short size = (unsigned short)(s.size() + 1);
    *this << (unsigned short)htons(size);
    pack(s.c_str(), size);
    return *this;
}

Buffer &Buffer::operator << (char **str)
{
    string s;
    if (*str)
        s = *str;
    return *this << s;
}

Buffer &Buffer::operator << (const char *str)
{
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

void Buffer::packScreen(const char *screen)
{
    char len = (char)(strlen(screen));
    pack(&len, 1);
    pack(screen, len);
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

void Buffer::tlv(unsigned short n, const char *data, unsigned short len)
{
    *this << n << len;
    pack(data, len);
}

void Buffer::tlv(unsigned short n, const char *data)
{
    tlv(n, data, (unsigned short)strlen(data));
}

void Buffer::tlv(unsigned short n, unsigned short c)
{
    c = htons(c);
    tlv(n, (char*)&c, 2);
}

void Buffer::tlv(unsigned short n, unsigned long c)
{
    c = htonl(c);
    tlv(n, (char*)&c, 4);
}

void Buffer::packetStart()
{
    m_packetStartPos = writePos();
}

unsigned long Buffer::packetStartPos()
{
    return m_packetStartPos;
}

class listTlv : public vector<Tlv*>
{
public:
    listTlv();
    ~listTlv();
};

listTlv::listTlv()
{
}

listTlv::~listTlv()
{
    vector<Tlv*>::iterator it;
    for (it = begin(); it != end(); it++)
        delete *it;
}

TlvList::TlvList()
{
    m_tlv = new listTlv;
}

TlvList::TlvList(Buffer &b)
{
    m_tlv = new listTlv;
    for (; b.readPos() < b.size(); ){
        unsigned short num, size;
        b >> num >> size;
        *this + new Tlv(num, size, b.data(b.readPos()));
        b.incReadPos(size);
    }
}

TlvList::~TlvList()
{
    delete static_cast<listTlv*>(m_tlv);
}

TlvList &TlvList::operator +(Tlv *tlv)
{
    static_cast<listTlv*>(m_tlv)->push_back(tlv);
    return *this;
}

Tlv *TlvList::operator()(unsigned short num)
{
    vector<Tlv*>::iterator it;
    for (it = static_cast<listTlv*>(m_tlv)->begin(); it != static_cast<listTlv*>(m_tlv)->end(); it++)
        if ((*it)->Num() == num) return *it;
    return NULL;
}

Tlv *TlvList::operator[](unsigned n)
{
    listTlv *l = static_cast<listTlv*>(m_tlv);
    if (n >= l->size())
        return NULL;
    return (*l)[n];
}

Buffer &Buffer::operator << (TlvList &tlvList)
{
    listTlv *l = static_cast<listTlv*>(tlvList.m_tlv);
    unsigned size = 0;
    vector<Tlv*>::iterator it;
    for (it = l->begin(); it != l->end(); it++)
        size += (*it)->Size() + 4;
    *this << (unsigned short)size;
    for (it = l->begin(); it != l->end(); it++){
        Tlv *tlv = *it;
        *this << tlv->Num() << tlv->Size();
        pack(*tlv, tlv->Size());
    }
    return *this;
}

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

string Buffer::getSection(bool bSkip)
{
	m_posRead = m_posWrite;
	unsigned posRead = m_posRead;
	char *p = data(m_posRead);
	if (bSkip){
		for (;;){
			for (; m_posRead < m_size; p++)
				if ((*p == '\n') || (*p == 0))
					break;
			if (m_posRead >= m_size){
				m_posRead = posRead;
				return "";
			}
			m_posRead++;
			p++;
			if (*p == '[')
				break;
		}
	}
	for (;;){
		if (m_posRead >= m_size){
			m_posRead = posRead;
			return "";
		}
		if (*p == '[')
			break;
		for (; m_posRead < m_size; p++)
			if ((*p == '\n') || (*p == 0))
				break;
		if (m_posRead >= m_size){
			m_posRead = posRead;
			return "";
		}
		m_posRead++;
		p++;
	}
	m_startSection = m_posRead;
	m_posRead++;
	p++;
	string section;
	char *s = p;
	for (; m_posRead < m_size; p++, m_posRead++){
		if (*p == ']'){
			*p = 0;
			section = s;
			*p = ']';
		}
		if ((*p == '\n') || (*p == 0))
			break;
	}
	if (m_posRead >= m_size){
		m_posRead = posRead;
		return "";
	}
	for (;m_posRead < m_size; p++, m_posRead++){
		if ((*p != '\n') || (*p == 0))
			break;
	}
	m_posWrite = m_posRead;
	for (; m_posWrite < m_size; p++, m_posWrite++){
		if ((*p == '\r') || (*p == '\n') || (*p == 0)){
			*p = 0;
			if ((m_posWrite + 1 < m_size) && (p[1] == '[')){
				m_posWrite++;
				return section;
			}
		}
	}
	return section;
}

char *Buffer::getLine()
{
	if (readPos() >= writePos())
		return NULL;
	char *res = data(m_posRead);
	char *p;
	for (p = res; *p && (m_posRead < m_posWrite); p++)
		m_posRead++;
	for (; (*p == 0) && (m_posRead < m_posWrite); p++)
		m_posRead++;
	return res;
}


