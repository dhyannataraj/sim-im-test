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

using namespace SIM;

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

void MetaInfoSnacHandler::uploadBasicInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadBasicInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList basicInfo(TlvList::LittleEndian);
    basicInfo.append(Tlv(MetaInfoSnacHandler::TlvFirstName, nullTerminatedStringWLength(client()->ownerIcqContact()->getFirstName())));
    basicInfo.append(Tlv(MetaInfoSnacHandler::TlvLastName, nullTerminatedStringWLength(client()->ownerIcqContact()->getLastName())));
    basicInfo.append(Tlv(MetaInfoSnacHandler::TlvNickname, nullTerminatedStringWLength(client()->ownerIcqContact()->getNick())));

    QByteArray data = basicInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

void MetaInfoSnacHandler::uploadHomeInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadHomeInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList homeInfo(TlvList::LittleEndian);
    homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeCity, nullTerminatedStringWLength(contact->getCity())));
    homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeState, nullTerminatedStringWLength(contact->getState())));
    homeInfo.append(Tlv::fromUint16(MetaInfoSnacHandler::TlvHomeCountry, contact->getCountry()));
    homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeAddress, nullTerminatedStringWLength(contact->getAddress())));
    homeInfo.append(Tlv::fromUint16(MetaInfoSnacHandler::TlvHomeZip, contact->getZip().toUInt()));
    homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomePhone, nullTerminatedStringWLength(contact->getHomePhone())));
    homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeFax, nullTerminatedStringWLength(contact->getHomeFax())));
    homeInfo.append(Tlv(MetaInfoSnacHandler::TlvCellular, nullTerminatedStringWLength(contact->getCellular())));


    QByteArray data = homeInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

void MetaInfoSnacHandler::uploadWorkInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadWorkInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList workInfo(TlvList::LittleEndian);
    workInfo.append(Tlv(TlvWorkCompany, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkName())));
    workInfo.append(Tlv(TlvWorkDepartment, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkDepartment())));
    workInfo.append(Tlv(TlvWorkPosition, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkPosition())));
    workInfo.append(Tlv::fromUint16(TlvWorkOccupation, client()->ownerIcqContact()->getOccupation(), Tlv::LittleEndian));
    workInfo.append(Tlv(TlvWorkAddress, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkAddress())));
    workInfo.append(Tlv(TlvWorkCity, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkCity())));
    //workInfo.append(Tlv(TlvWorkState, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkState())));
    workInfo.append(Tlv::fromUint16(TlvWorkCountry, client()->ownerIcqContact()->getCountry(), Tlv::LittleEndian));
    workInfo.append(Tlv::fromUint32(TlvWorkZip, client()->ownerIcqContact()->getWorkZip().toUInt(), Tlv::LittleEndian));
    workInfo.append(Tlv(TlvWorkPhone, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkPhone())));
    workInfo.append(Tlv(TlvWorkFax, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkFax())));
    workInfo.append(Tlv(TlvWorkHomepage, nullTerminatedStringWLength(client()->ownerIcqContact()->getWorkHomepage())));


    QByteArray data = workInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

void MetaInfoSnacHandler::uploadMoreInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadMoreInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList moreInfo(TlvList::LittleEndian);
    moreInfo.append(Tlv::fromUint16(TlvAge, contact->getAge(), Tlv::LittleEndian));

    ByteArrayBuilder birthdayData(ByteArrayBuilder::LittleEndian);
    birthdayData.appendWord(contact->getBirthday().year());
    birthdayData.appendWord(contact->getBirthday().month());
    birthdayData.appendWord(contact->getBirthday().day());
    moreInfo.append(Tlv(TlvBirthday, birthdayData.getArray()));
    moreInfo.append(Tlv::fromUint16(TlvLanguage, contact->getPrimaryLanguage(), Tlv::LittleEndian));
    moreInfo.append(Tlv::fromUint16(TlvLanguage, contact->getSecondaryLanguage(), Tlv::LittleEndian));
    moreInfo.append(Tlv::fromUint16(TlvLanguage, contact->getTertiaryLanguage(), Tlv::LittleEndian));

    QByteArray data = moreInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

void MetaInfoSnacHandler::uploadAboutInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadAboutInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList aboutInfo(TlvList::LittleEndian);
    aboutInfo.append(Tlv(TlvAbout, nullTerminatedStringWLength(contact->getAbout())));

    QByteArray data = aboutInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

void MetaInfoSnacHandler::uploadInterestsInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadInterestsInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList interestsInfo(TlvList::LittleEndian);
    interestsInfo.append(Tlv(TlvInterest, makeIcombo(contact->getInterest(0), contact->getInterestText(0))));
    interestsInfo.append(Tlv(TlvInterest, makeIcombo(contact->getInterest(1), contact->getInterestText(1))));
    interestsInfo.append(Tlv(TlvInterest, makeIcombo(contact->getInterest(2), contact->getInterestText(2))));
    interestsInfo.append(Tlv(TlvInterest, makeIcombo(contact->getInterest(3), contact->getInterestText(3))));

    QByteArray data = interestsInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

    client()->oscarSocket()->snac(SnacId, SnacMetaInfoRequest, 0, tlvs.toByteArray());
}

void MetaInfoSnacHandler::uploadPastInfo()
{
    log(L_DEBUG, "MetaInfoSnacHandler::uploadPastInfo");
    ICQContactPtr contact = m_client->ownerIcqContact();
    addMetaInfoRequest(m_sqnum, contact);

    TlvList backgroundInfo(TlvList::LittleEndian);
    backgroundInfo.append(Tlv(TlvBackground, makeIcombo(contact->getBackgroundCode(0), contact->getBackgroundText(0))));
    backgroundInfo.append(Tlv(TlvBackground, makeIcombo(contact->getBackgroundCode(1), contact->getBackgroundText(1))));
    backgroundInfo.append(Tlv(TlvBackground, makeIcombo(contact->getBackgroundCode(2), contact->getBackgroundText(2))));

    backgroundInfo.append(Tlv(TlvAffiliation, makeIcombo(contact->getAffiliationCode(0), contact->getAffiliationText(0))));
    backgroundInfo.append(Tlv(TlvAffiliation, makeIcombo(contact->getAffiliationCode(1), contact->getAffiliationText(1))));
    backgroundInfo.append(Tlv(TlvAffiliation, makeIcombo(contact->getAffiliationCode(2), contact->getAffiliationText(2))));

    QByteArray data = backgroundInfo.toByteArray();

    ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
    metaPacket.appendWord(data.size() + 10);
    metaPacket.appendDword(contact->getUin());
    metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
    metaPacket.appendWord(m_sqnum++);
    metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
    metaPacket.appendBytes(data);

    TlvList tlvs;
    tlvs.append(Tlv(0x01, metaPacket.getArray()));

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

    int dataSubtype = parser.readWord();

    switch(dataSubtype)
    {
    case MetaBasicUserInfo:
        return parseBasicUserInfo(parser, contact);
    case MetaWorkUserInfo:
        return parseWorkUserInfo(parser, contact);
    case MetaMoreUserInfo:
        return parseMoreUserInfo(parser, contact);
    case MetaAboutUserInfo:
        return parseAboutUserInfo(parser, contact);
    case MetaInterestsUserInfo:
        return parseInterestsUserInfo(parser, contact);
    case MetaPastUserInfo:
        return parsePastUserInfo(parser, contact);
    default:
        return false;
    }

    return false;
}

bool MetaInfoSnacHandler::parseBasicUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact)
{
    int successByte = parser.readByte();
    if(successByte != 0x0a)
        return false;

    contact->setNick(readString(parser));
    contact->setFirstName(readString(parser));
    contact->setLastName(readString(parser));
    contact->setEmail(readString(parser));
    contact->setCity(readString(parser));
    contact->setState(readString(parser));
    contact->setHomePhone(readString(parser));
    contact->setHomeFax(readString(parser));
    contact->setAddress(readString(parser));
    contact->setCellular(readString(parser));
    contact->setZip(readString(parser));

    contact->setCountry(parser.readWord());
    contact->setTimeZone(parser.readByte());

    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_basic_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_basic_info_updated")->triggered(data);
    return true;
}

bool MetaInfoSnacHandler::parseWorkUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact)
{
    int successByte = parser.readByte();
    if(successByte != 0x0a)
        return false;

    contact->setWorkCity(readString(parser));
    contact->setWorkState(readString(parser));
    contact->setWorkPhone(readString(parser));
    contact->setWorkFax(readString(parser));
    contact->setWorkAddress(readString(parser));
    contact->setWorkZip(readString(parser));
    contact->setWorkCountry(parser.readWord());
    contact->setWorkName(readString(parser));
    contact->setWorkDepartment(readString(parser));
    contact->setWorkPosition(readString(parser));
    contact->setOccupation(parser.readWord());
    contact->setWorkHomepage(readString(parser));

    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_work_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_work_info_updated")->triggered(data);
    return true;
}

bool MetaInfoSnacHandler::parseMoreUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact)
{
    int successByte = parser.readByte();
    if(successByte != 0x0a)
        return false;

    contact->setAge(parser.readWord());
    contact->setGender(parser.readByte());
    contact->setHomepage(readString(parser));

    int year = parser.readWord();
    int month = parser.readByte();
    int day = parser.readByte();
    contact->setBirthday(QDate(year, month, day));

    contact->setPrimaryLanguage(parser.readByte());
    contact->setSecondaryLanguage(parser.readByte());
    contact->setTertiaryLanguage(parser.readByte());

    // Ignore the rest

    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_more_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_more_info_updated")->triggered(data);
    return true;
}

bool MetaInfoSnacHandler::parseAboutUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact)
{
    int successByte = parser.readByte();
    if(successByte != 0x0a)
        return false;

    contact->setAbout(readString(parser));

    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_about_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_about_info_updated")->triggered(data);
    return true;
}

bool MetaInfoSnacHandler::parseInterestsUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact)
{
    int successByte = parser.readByte();
    if(successByte != 0x0a)
        return false;

    int interestsCount = parser.readByte();
    if(interestsCount != 4)
    {
        SIM::log(SIM::L_WARN, "MetaInfoSnacHandler::parseInterestsUserInfo: Invalid interests count: %d", interestsCount);
    }

    for(int i = 0; i < interestsCount; i++)
    {
        int interestCode = parser.readWord();
        QString interestText = readString(parser);
        contact->setInterest(i, interestCode, interestText);
    }

    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_interests_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_interests_info_updated")->triggered(data);
    return true;
}

bool MetaInfoSnacHandler::parsePastUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact)
{
    int successByte = parser.readByte();
    if(successByte != 0x0a)
        return false;

    int affiliationCount = parser.readByte();
    for(int i = 0; i < 3; i++)
    {
        int affiliationCode = parser.readWord();
        QString affiliationText = readString(parser);
        contact->setAffiliation(i, affiliationCode, affiliationText);
    }

    int backgroundsCount = parser.readByte();
    for(int i = 0; i < 3; i++)
    {
        int backgroundCode = parser.readWord();
        QString backgroundText = readString(parser);
        contact->setBackground(i, backgroundCode, backgroundText);
    }

    IcqContactUpdateDataPtr data = IcqContactUpdateData::create("icq_contact_past_info_updated", contact->getScreen());
    SIM::getEventHub()->getEvent("icq_contact_past_info_updated")->triggered(data);
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

QString MetaInfoSnacHandler::readString(ByteArrayParser& parser)
{
    int length = parser.readWord();
    QByteArray arr = parser.readBytes(length);
    return QString::fromAscii(arr.data());
}

void MetaInfoSnacHandler::appendString(ByteArrayBuilder& builder, const QString& str)
{
    builder.appendWord(str.length() + 1);
    builder.appendBytes(str.toAscii());
    builder.appendByte(0);
}

QByteArray MetaInfoSnacHandler::nullTerminatedStringWLength(const QString& str)
{
    ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
    builder.appendWord(str.length() + 1);
    builder.appendBytes(str.toAscii());
    builder.appendByte(0);
    return builder.getArray();
}

QByteArray MetaInfoSnacHandler::makeIcombo(int code, const QString& str)
{
    ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
    builder.appendWord(code);
    builder.appendWord(str.length() + 1);
    builder.appendBytes(str.toAscii());
    builder.appendByte(0);
    return builder.getArray();
}
