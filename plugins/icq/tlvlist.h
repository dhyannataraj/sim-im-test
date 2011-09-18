#ifndef TLVLIST_H
#define TLVLIST_H

#include "tlv.h"
#include "icq_defines.h"
#include <QList>

class ICQ_EXPORT TlvList
{
public:
    enum Endianness
    {
        BigEndian,
        LittleEndian
    };
    TlvList(Endianness endianness = BigEndian);

    void append(const Tlv& tlv);
    Tlv at(int index) const;
    Tlv firstTlv(int id) const;

    int size() const;
    int tlvCount() const;

    bool contains(int id) const;

    static TlvList fromByteArray(const QByteArray& data, Endianness endianness = BigEndian);
    QByteArray toByteArray();

private:
    QList<Tlv> m_tlvs;
    Endianness m_endianness;
};

#endif // TLVLIST_H
