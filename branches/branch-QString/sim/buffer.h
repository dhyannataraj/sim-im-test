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
#include <qcstring.h>
#include <qptrlist.h>

class Buffer;

class EXPORT Tlv
{
public:
    Tlv(unsigned short num = 0, unsigned short size = 0, const char *data = NULL);
    unsigned short Num() { return (unsigned short)m_nNum; }
    unsigned short Size() { return (unsigned short)m_nSize; }
    const char *Data() { return m_data.data(); }
    operator char *() { return m_data.data(); }
    operator unsigned short ();
    operator unsigned long ();
protected:
    int m_nNum;
    int m_nSize;
    QByteArray m_data;
};

class EXPORT TlvList : public QPtrList<Tlv>
{
public:
    TlvList();
    TlvList(Buffer&, unsigned nTlvs = ~0U);
    Tlv *operator() (unsigned short num);
    Tlv *operator[] (unsigned n) {  return at(n); }
    TlvList &operator+= (const Tlv *tlv) { append(tlv); return *this; }
};

class EXPORT Buffer : public QByteArray
{
public:
    Buffer(unsigned size=0);
    Buffer(Tlv&);
    ~Buffer();
    bool add(uint size);
    bool resize(uint size);
    unsigned readPos() const { return m_posRead; }
    void incReadPos(int size);
    void decReadPos(int size) { incReadPos(-size); }
    unsigned writePos() const { return m_posWrite; }
    void setWritePos(unsigned size);
    void setReadPos(unsigned size);

    char* data(unsigned pos=0) const { return QByteArray::data() + pos; }

    void packetStart();
    unsigned long packetStartPos();

    void pack(char c) { pack(&c, 1); }
    void pack(long c) { pack((unsigned long)c); }
    void pack(unsigned char c) { pack((char)c); }
    void pack(unsigned short c);
    void pack(unsigned long c);
    void pack(const char *d, unsigned size);
    void pack(const QString &s);    // utf8
    void packScreen(const QString &s);
    void packStr32(const char *s);
    void pack32(const Buffer &b);

    void unpack(char &c);
    void unpack(unsigned char &c) { unpack((char&)c); }
    void unpack(unsigned short &c);
    void unpack(unsigned long &c);
    unsigned unpack(char *d, unsigned size);
    unsigned unpack(QString &d, unsigned size); // utf8
    unsigned unpack(QCString &d, unsigned size);
    unsigned unpack(QByteArray &d, unsigned size);
    void unpack(QCString &s);
    void unpackStr(QString &s);     // utf8
    void unpackStr(QCString &s);
    void unpackStr32(QByteArray &s);
    void unpackStr32(QCString &s);

    QString unpackScreen();

    Buffer &operator >> (char &c);
    Buffer &operator >> (unsigned char &c) { return operator >> ((char&)c); }
    Buffer &operator >> (unsigned short &c);
    Buffer &operator >> (unsigned long &c);
    Buffer &operator >> (int &c);
    Buffer &operator >> (QCString &s);

    Buffer &operator << (const QString &s);
    Buffer &operator << (const QCString &s);
    Buffer &operator << (const char *str);
    Buffer &operator << (char c);
    Buffer &operator << (unsigned char c) { return operator << ((char)c); }
    Buffer &operator << (unsigned short c);
    Buffer &operator << (int c) { return operator << ((unsigned short)c); }
    Buffer &operator << (unsigned long c);
    Buffer &operator << (long c) { return operator << ((unsigned long)c); }
    Buffer &operator << (const Buffer &b);
    Buffer &operator << (const bool b);
    Buffer &operator << (TlvList&);

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

    bool scan(const char *substr, QCString &res);

    void init(unsigned size);

    void fromBase64(Buffer &from);
    void toBase64(Buffer &from);

    QString getSection(bool bSkip=false);
    unsigned    startSection() { return m_startSection; }

    Buffer &operator = (const QByteArray &ba);

protected:
    unsigned m_packetStartPos;
    unsigned m_posRead;
    unsigned m_posWrite;
    unsigned m_startSection;
};

class EXPORT ConfigBuffer : public QString
{
public:
	ConfigBuffer(const QString &str, unsigned posStart = 0);
	ConfigBuffer(QIODevice *io, unsigned posStart = 0);
    QString getSection(bool bSkip=false);
    QString getLine();
    unsigned    startSection() { return m_startSection; }
    bool dataAvailable() { return (m_posRead < m_posNextSection); }
    ConfigBuffer *getData() { return new ConfigBuffer(mid( m_posRead, m_posNextSection - m_posRead)); }
    void savePos() { m_posReadSave = m_posRead; }
    void restorePos() { m_posRead = m_posReadSave; }

    unsigned readPos() const { return m_posRead; }
protected:
    int findStartSection(unsigned start);
    int findEndSection(unsigned start);
protected:
    unsigned m_posRead;
    unsigned m_startSection;
    unsigned m_posNextSection;
    unsigned m_posReadSave;
};

EXPORT void log_packet(Buffer &buf, bool bOut, unsigned packet_id, const char *add_info=NULL);

#endif

