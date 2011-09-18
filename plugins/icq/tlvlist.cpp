#include "tlvlist.h"
#include "bytearrayparser.h"
#include "bytearraybuilder.h"

TlvList::TlvList(Endianness endianness) : m_endianness(endianness)
{
}

void TlvList::append(const Tlv& tlv)
{
    m_tlvs.append(tlv);
}

Tlv TlvList::at(int index) const
{
    return m_tlvs.at(index);
}

Tlv TlvList::firstTlv(int id) const
{
    foreach(const Tlv& tlv, m_tlvs)
    {
        if(tlv.id() == id)
            return tlv;
    }
    return Tlv();
}

int TlvList::size() const
{
    return m_tlvs.size();
}

bool TlvList::contains(int id) const
{
    foreach(const Tlv& tlv, m_tlvs)
    {
        if(tlv.id() == id)
            return true;
    }
    return false;
}

TlvList TlvList::fromByteArray(const QByteArray& data, Endianness endianness)
{
    ByteArrayParser parser(data, endianness == LittleEndian ? ByteArrayParser::LittleEndian : ByteArrayParser::BigEndian);
    TlvList list(endianness);

    while(!parser.atEnd())
    {
        quint16 id = parser.readWord();
        int length = parser.readWord();
        QByteArray parserdata = parser.readBytes(length);
        if(parserdata.size() < length)
            break;
        list.append(Tlv(id, parserdata));
    }

    return list;
}

QByteArray TlvList::toByteArray()
{
    ByteArrayBuilder builder(m_endianness == LittleEndian ? ByteArrayBuilder::LittleEndian : ByteArrayBuilder::BigEndian);
    foreach(const Tlv& tlv, m_tlvs)
    {
        builder.appendWord(tlv.id());
        builder.appendWord(tlv.data().length());
        builder.appendBytes(tlv.data());
    }
    return builder.getArray();
}

int TlvList::tlvCount() const
{
    return m_tlvs.count();
}
