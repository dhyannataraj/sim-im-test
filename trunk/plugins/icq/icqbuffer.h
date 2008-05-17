/***************************************************************************
                          icqbuffer.h  -  description
                             -------------------
    begin                : Fri Jan 05 2007
    copyright            : (C) 2007 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _ICQBUFFER_H
#define _ICQBUFFER_H

#include "buffer.h"
#include "socket.h"

class ICQBuffer;
class Tlv
{
public:
    Tlv(unsigned short num = 0, unsigned short size = 0, const char *data = NULL);
    unsigned short Num() const { return m_nNum; }
    unsigned short Size() const { return m_nSize; }
    const char *Data() const { return m_data.data(); }
    operator char *() const { return m_data.data(); }
    const QByteArray &byteArray() const { return m_data; }
    operator uint16_t () const;
    operator uint32_t () const;
protected:
    unsigned int m_nNum;
    unsigned int m_nSize;
    QByteArray m_data;
};

class TlvList : public QValueList<Tlv*>
{
public:
    TlvList();
    TlvList(ICQBuffer&, unsigned nTlvs = ~0U);
    ~TlvList();
    // return Tlv<num>, since there can be more than one Tlv<num> inside
    // a package, you can specificy how much Tlv<num> should be skipped
    Tlv *operator() (unsigned short num, int skip = 0);
};

class ICQBuffer : public Buffer
{
public:
    ICQBuffer(unsigned size = 0);
    ICQBuffer(const QByteArray &ba);
    ICQBuffer(Tlv&);
    virtual ~ICQBuffer();

    void tlv(unsigned short n, const char *data, unsigned short len);
    void tlv(unsigned short n) { tlv(n, NULL, 0); }
    void tlv(unsigned short n, const char *data);
    void tlv(unsigned short n, unsigned short c);
    void tlv(unsigned short n, int c) { tlv(n, (unsigned short)c); }
    void tlv(unsigned short n, unsigned long c);
    void tlv(unsigned short n, long c) { tlv(n, (unsigned long)c); }
    void tlv(unsigned short n, Buffer &b) { tlv(n, b.data(), (unsigned short)(b.size())); }

    void tlvLE(unsigned short n, const char *data, unsigned short len);
    void tlvLE(unsigned short n, const char *data);
    void tlvLE(unsigned short n, char c) { tlvLE(n, &c, 1); }
    void tlvLE(unsigned short n, unsigned short c);
    void tlvLE(unsigned short n, unsigned long c);
    void tlvLE(unsigned short n, Buffer &b) { tlvLE(n, b.data(), (unsigned short)(b.size())); }

    ICQBuffer &operator << (const TlvList&);
    ICQBuffer &operator << (const QString &s);     // utf8
    ICQBuffer &operator << (const QCString &s);
    ICQBuffer &operator << (const QByteArray &s);
    ICQBuffer &operator << (const Buffer &b);
    ICQBuffer &operator << (char c);
    ICQBuffer &operator << (unsigned char c) { return operator << ((char)c); }
    ICQBuffer &operator << (const char *str);
    ICQBuffer &operator << (unsigned short c);
    ICQBuffer &operator << (int c) { return operator << ((unsigned short)c); }
    ICQBuffer &operator << (unsigned long c);
    ICQBuffer &operator << (long c) { return operator << ((unsigned long)c); }
    ICQBuffer &operator << (const bool b);

    ICQBuffer &operator >> (QCString &s);  // size is 2 byte & little endian!
    ICQBuffer &operator >> (char &c);
    ICQBuffer &operator >> (unsigned char &c) { return operator >> ((char&)c); }
    ICQBuffer &operator >> (unsigned short &c);
    ICQBuffer &operator >> (unsigned long &c);
    ICQBuffer &operator >> (int &c);

    void pack(const QString &s);
    void pack(const QCString &s);
    void pack(const char *d, unsigned size) { Buffer::pack(d, size); }
    void pack(char c)          { *this << c; }
    void pack(unsigned char c) { *this << c; }
    void pack(unsigned short c);
    void pack(unsigned long c);
    void pack(long c) { pack((unsigned long)c); }

    void packScreen(const QString &);
    void packStr32(const QCString &);
    void pack32(const Buffer &b);

    // 2 byte size + string
    bool unpackStr(QString &s);     // utf8
    bool unpackStr(QCString &s);
    // 4 byte size  + string
    bool unpackStr32(QCString &s);
    bool unpackStr32(QByteArray &s);
    QString unpackScreen();

    void unpack(char &c)          { *this >> c; }
    void unpack(unsigned char &c) { *this >> c; }
    unsigned unpack(char *d, unsigned size);
    unsigned unpack(QString &d, unsigned size); // utf8
    unsigned unpack(QCString &d, unsigned size);
    unsigned unpack(QByteArray &d, unsigned size);
    void unpack(unsigned short &c);
    void unpack(unsigned long &c);
};

class ICQClientSocket : public SIM::ClientSocket
{
public:
    ICQClientSocket(SIM::ClientSocketNotify *notify, SIM::Socket *sock = NULL)
        : ClientSocket(notify, sock) {};
    ~ICQClientSocket() {};

    virtual ICQBuffer &readBuffer() { return m_readICQBuffer; }
    virtual ICQBuffer &writeBuffer() { return m_writeICQBuffer; }
protected:
    ICQBuffer m_readICQBuffer;
    ICQBuffer m_writeICQBuffer;
};

#endif  // _ICQBUFFER_H
