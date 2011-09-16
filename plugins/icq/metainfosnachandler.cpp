/*
 * metainfosnachandler.cpp
 *
 *  Created on: Sep 15, 2011
 */

#include "metainfosnachandler.h"
#include "bytearraybuilder.h"
#include "tlvlist.h"
#include "icqclient.h"
#include "events/eventhub.h"
#include "events/icqcontactupdate.h"

#include <algorithm>


MetaInfoSnacHandler::MetaInfoSnacHandler(ICQClient* client) : SnacHandler(client, 0x15), // FIXME hardcoded
    m_sqnum(1)
{
}

MetaInfoSnacHandler::~MetaInfoSnacHandler()
{
}

bool MetaInfoSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    switch(subtype)
    {
    case SnacMetaInfoData:
        return processMetaInfoData(data);
    }
    return false;
}

void MetaInfoSnacHandler::requestFullInfo(const ICQContactPtr& contact)
{
    addMetaInfoRequest(m_sqnum, contact);
    TlvList tlvs;
    ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
    builder.appendWord(14);
    builder.appendDword(client()->ownerIcqContact()->getUin());
    builder.appendWord(MetaInfoRequest);
    builder.appendWord(m_sqnum++);
    builder.appendWord(0x4b2);
    builder.appendDword(contact->getUin());
    tlvs.append(Tlv(0x01, builder.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

bool MetaInfoSnacHandler::processMetaInfoData(const QByteArray& arr)
{
    TlvList list = TlvList::fromByteArray(arr);
    Tlv tlv = list.firstTlv(0x01);
    ByteArrayParser parser(tlv.data(), ByteArrayParser::LittleEndian);
    parser.readWord(); // Data chunk size
    parser.readDword(); // Owner uin
    int dataType = parser.readWord();
    if(dataType != MetaInfoData)
        return false;
    int sqnum = parser.readWord();

    ICQContactPtr contact = getMetaInfoRequestContact(sqnum);
    if(!contact)
        return false;


    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_basic_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_basic_info_updated")->triggered(data);
    return true;
}

void MetaInfoSnacHandler::addMetaInfoRequest(int sqnum, const ICQContactPtr& contact)
{
    MetaInfoRequestDescriptor desc = {m_sqnum, QDateTime::currentDateTime(), contact};
    m_requests.append(desc);
}

ICQContactPtr MetaInfoSnacHandler::getMetaInfoRequestContact(int sqnum)
{
    auto it = std::find_if(m_requests.begin(), m_requests.end(),
            [=](const MetaInfoRequestDescriptor& desc) -> bool { return desc.sqnum == sqnum; } );
    if(it == m_requests.end())
        return ICQContactPtr();
    return (*it).contact;
}
