/***************************************************************************
                          buffer.cpp  -  description
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

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "buffer.h"
#include "log.h"

#include <stdio.h>

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

void Buffer::incReadPos(int n)
{
    m_posRead += n;
    if (m_posRead > m_posWrite) m_posRead = m_posWrite;
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

unsigned long Buffer::unpackUin()
{
    char len;
    *this >> len;
    char uin[14];
    if (len > 13) len = 13;
    unpack(uin, len);
    uin[len] = 0;
    return atol(uin);
}

void Buffer::unpack(string &s)
{
    unsigned short size;
    unpack(size);
    s.erase();
    if (size == 0) return;
    if (size > m_size - m_posRead) size = m_size - m_posRead;
    s.append(size, '\x00');
    unpack((char*)s.c_str(), size);
}

void Buffer::unpackStr(string &s)
{
    unsigned short size;
    *this >> size;
    s.erase();
    if (size == 0) return;
    if (size > m_size - m_posRead) size = m_size - m_posRead;
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
    if (size > m_size - m_posRead) size = m_size - m_posRead;
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
        s.append(size, '\x00');
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

void Buffer::unpack(char &c)
{
    *this >> c;
}

void Buffer::unpack(unsigned short &c)
{
    if (unpack((char*)&c, 2) != 2) c = 0;
}

void Buffer::unpack(unsigned long &c)
{
    if (unpack((char*)&c, 4) != 4) c = 0;
}

void Buffer::unpackBE(unsigned short &c)
{
    unpack(c);
    c = htons(c);
}

void Buffer::unpackBE(unsigned long &c)
{
    unpack(c);
    c = htonl(c);
}

void Buffer::pack(const string &s)
{
    unsigned short size = s.size();
    *this << size;
    pack(s.c_str(), size);
}

void Buffer::packStr32(const char *s)
{
    unsigned long size = strlen(s);
    pack((char*)&size, sizeof(size));
    pack(s, strlen(s));
}

Buffer &Buffer::operator << (const Buffer &b)
{
    unsigned short size = b.size() - b.readPos();
    *this << (unsigned short)htons(size);
    pack(b.Data(b.readPos()), size);
    return *this;
}

void Buffer::pack32(const Buffer &b)
{
    unsigned long size = b.size() - b.readPos();
    *this << (unsigned long)htonl(size);
    pack(b.Data(b.readPos()), size);
}

Buffer &Buffer::operator << (const string &s)
{
    unsigned short size = s.size() + 1;
    *this << (unsigned short)htons(size);
    pack(s.c_str(), size);
    return *this;
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
    char c = b ? 1 : 0;
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

void Buffer::packUin(unsigned long uin)
{
    char u[13];
    char len = snprintf(u, sizeof(u), "%lu", uin);
    pack(&len, 1);
    pack(u, len);
}

void Buffer::tlv(unsigned short n, const char *data, unsigned short len)
{
    *this << n << len;
    pack(data, len);
}

void Buffer::tlv(unsigned short n, const char *data)
{
    tlv(n, data, strlen(data));
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

TlvList::TlvList(Buffer &b)
{
    for (; b.readPos() < b.size(); ){
        unsigned short num, size;
        b >> num >> size;
        m_tlv.push_back(new Tlv(num, size, b.Data(b.readPos())));
        b.incReadPos(size);
    }
}

TlvList::~TlvList()
{
    list<Tlv*>::iterator it;
    for (it = m_tlv.begin(); it != m_tlv.end(); it++)
        delete *it;
}

Tlv *TlvList::operator()(unsigned short num)
{
    list<Tlv*>::iterator it;
    for (it = m_tlv.begin(); it != m_tlv.end(); it++)
        if ((*it)->Num() == num) return *it;
    return NULL;
}

Tlv::Tlv(unsigned short num, unsigned short size, char *data)
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


