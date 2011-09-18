#ifndef TLV_H
#define TLV_H

#include "icq_defines.h"
#include <QByteArray>

class ICQ_EXPORT Tlv
{
public:
    enum Endianness
    {
        BigEndian,
        LittleEndian
    };

    Tlv();
    Tlv(int id, const QByteArray& data);

    int id() const;
    QByteArray data() const;

    quint16 toUint16() const;
    quint32 toUint32() const;

    bool isValid() const;

    static Tlv fromUint16(int id, int value, Endianness end = BigEndian);
    static Tlv fromUint32(int id, int value, Endianness end = BigEndian);

private:
    int m_id;
    QByteArray m_data;
    bool m_valid;
};

#endif // TLV_H
