/***************************************************************************
                          buffer.h  -  description
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

#ifndef _BUFFER_H
#define _BUFFER_H

#include "simapi.h"

#include <list>

using namespace std;

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

class Buffer;

class EXPORT Tlv
{
public:
    Tlv(unsigned short num, unsigned short size, const char *data);
    ~Tlv();
    unsigned short Num() { return m_nNum; }
    unsigned short Size() { return m_nSize; }
    operator char *() { return m_data; }
    operator unsigned short ();
    operator unsigned long ();
protected:
    unsigned m_nNum;
    unsigned m_nSize;
    char *m_data;
};

class EXPORT TlvList
{
public:
    TlvList();
    TlvList(Buffer&);
    ~TlvList();
    Tlv *operator() (unsigned short num);
    TlvList &operator + (Tlv *tlv);
	Tlv *operator[](unsigned n);
protected:
    void *m_tlv;
    friend class Buffer;
};

class EXPORT Buffer
{
public:
    Buffer(unsigned size=0);
    Buffer(Tlv&);
    ~Buffer();
    void add(unsigned size);
unsigned size() const { return m_size; }
    unsigned readPos() const { return m_posRead; }
    void incReadPos(int size);
    unsigned writePos() const { return m_posWrite; }
    void setWritePos(unsigned size);
    void setReadPos(unsigned size);

    char* data(unsigned pos=0) const { return m_data + pos; }

    void packetStart();
    unsigned long packetStartPos();

    void pack(const char *d, unsigned size);
    unsigned unpack(char *d, unsigned size);

    void pack(char c) { pack(&c, 1); }
    void pack(unsigned short c);
    void pack(unsigned long c);
    void pack(long c) { pack((unsigned long)c); }

    Buffer &operator >> (char &c);
    Buffer &operator >> (unsigned char &c) { return operator >> ((char&)c); }
    Buffer &operator >> (unsigned short &c);
    Buffer &operator >> (unsigned long &c);
    Buffer &operator >> (string &s);
    Buffer &operator >> (char**);

    void unpack(char &c);
    void unpack(unsigned short &c);
    void unpack(unsigned long &c);
    string unpackScreen();
    void unpack(string &s);
    void unpackStr(string &s);
    void unpackStr32(string &s);

    void pack(const string &s);

    Buffer &operator << (const string &s);
    Buffer &operator << (const char *str);
    Buffer &operator << (char c);
    Buffer &operator << (unsigned char c) { return operator << ((char)c); }
    Buffer &operator << (unsigned short c);
    Buffer &operator << (int c) { return operator << ((unsigned short)c); }
    Buffer &operator << (unsigned long c);
    Buffer &operator << (long c) { return operator << ((unsigned long)c); }
    Buffer &operator << (const Buffer &b);
    Buffer &operator << (const bool b);
    Buffer &operator << (char**);
    Buffer &operator << (TlvList&);

    void packScreen(const char *);
    void packStr32(const char *);
    void pack32(const Buffer &b);

    void tlv(unsigned short n, const char *data, unsigned short len);
    void tlv(unsigned short n) { tlv(n, NULL, 0); }
    void tlv(unsigned short n, const char *data);
    void tlv(unsigned short n, unsigned short c);
    void tlv(unsigned short n, int c) { tlv(n, (unsigned short)c); }
    void tlv(unsigned short n, unsigned long c);
    void tlv(unsigned short n, long c) { tlv(n, (unsigned long)c); }
    void tlv(unsigned short n, Buffer &b) { tlv(n, b.data(), b.size()); }

    bool scan(const char *substr, string &res);

    void init(unsigned size);
    unsigned allocSize() { return m_alloc_size; }
    void allocate(unsigned size, unsigned add_size);
protected:
    unsigned m_packetStartPos;
    unsigned m_size;
    unsigned m_alloc_size;
    unsigned m_posRead;
    unsigned m_posWrite;
    char *m_data;
};

EXPORT void log_packet(Buffer &buf, bool bOut, unsigned packet_id, const char *add_info=NULL);

#endif

