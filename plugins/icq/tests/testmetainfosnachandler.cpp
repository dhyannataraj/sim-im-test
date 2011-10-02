/*
 * testmetainfosnachandler.cpp
 *
 *  Created on: Sep 15, 2011
 */

#include "../metainfosnachandler.h"
#include "icqclient.h"

#include <QSignalSpy>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "events/eventhub.h"

#include "mocks/mockoscarsocket.h"
#include "bytearraybuilder.h"
#include "tlvlist.h"

namespace
{
    static const int OwnerUin = 666666;
    static const int TargetUin = 123456789;

    static const QString Nickname = "Test nickname";
    static const QString FirstName = "Test firstname";
    static const QString LastName = "Test lastname";
    static const QString Email = "test@email.com";
    static const QString HomeCity = "Test homecity";
    static const QString HomeState = "Test homestate";
    static const QString HomePhone = "Test homephone";
    static const QString HomeFax = "Test homefax";
    static const QString HomeAddress = "Test address";
    static const QString CellPhone = "Test cellphone";
    static const QString HomeZip = "Test homezip";
    static const int HomeCountry = 1;
    static const int GmtOffset = 6;

    static const QString WorkCity = "Test Work City";
    static const QString WorkState = "Test Work State";
    static const QString WorkPhone = "Test Work Phone";
    static const QString WorkFax = "Test Work Fax";
    static const QString WorkAddress = "Test Work Address";
    static const QString WorkZip = "Test Work Zip";
    static const int WorkCountry = 355;
    static const QString WorkCompany = "Test Work Company";
    static const QString WorkDepartment = "Test Work Department";
    static const QString WorkPosition = "Test Work Position";
    static const int WorkOccupation = 1;
    static const QString WorkHomePage = "Test Work Homepage";

    static const int Age = 42;
    static const int Gender = 1;
    static const QString HomePage = "http://test.com";
    static const QDate Birthday = QDate(2000, 1, 1);
    static const int PrimaryLanguage = 4;
    static const int SecondaryLanguage = 5;
    static const int TertiaryLanguage = 6;
    static const QString OriginalCity = "Test original city";
    static const QString OriginalState = "Test original state";
    static const int OriginalCountry = 1;
    static const int OriginalTimezone = 1;

    static const QString AboutInfo = "Test test about info\nBlahblah";

    static const int InterestCode[] = {100, 101, 103, 107};
    static const QString InterestText[] = {"Interest1", "Interest2", "Interest3", "Interest4"};

    static const int AffiliationCode[] = {200, 201, 202};
    static const QString AffiliationText[] = {"Test affiliation1", "Test affiliation2", "Test affiliation3" };

    static const int BackgroundCode[] = {300, 301, 302};
    static const QString BackgroundText[] = { "Test background1", "Test background2", "Test background3" };

    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Truly;

    class TestMetaInfoSnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.666666", false);
            client->ownerIcqContact()->setUin(OwnerUin);
            client->ownerIcqContact()->setNick(Nickname);
            client->ownerIcqContact()->setFirstName(FirstName);
            client->ownerIcqContact()->setLastName(LastName);
            client->ownerIcqContact()->setEmail(Email);
            client->ownerIcqContact()->setCity(HomeCity);
            client->ownerIcqContact()->setState(HomeState);
            client->ownerIcqContact()->setHomePhone(HomePhone);
            client->ownerIcqContact()->setHomeFax(HomeFax);
            client->ownerIcqContact()->setAddress(HomeAddress);
            client->ownerIcqContact()->setCellular(CellPhone);
            client->ownerIcqContact()->setZip(HomeZip);
            client->ownerIcqContact()->setCountry(HomeCountry);
            client->ownerIcqContact()->setTimeZone(GmtOffset);

            handler = client->metaInfoSnacHandler();
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client->setOscarSocket(socket);

            contact = ICQContactPtr(new ICQContact(client));
            contact->setUin(TargetUin);

        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeFullinfoRequestPacket(int sqnum, unsigned int targetUin)
        {
            TlvList tlvs;
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendWord(14);
            builder.appendDword(client->ownerIcqContact()->getUin());
            builder.appendWord(0x7d0);
            builder.appendWord(sqnum);
            builder.appendWord(0x4b2);
            builder.appendDword(targetUin);
            tlvs.append(Tlv(0x01, builder.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makeBasicInfoSetPacket(int sqnum)
        {
            TlvList basicInfo(TlvList::LittleEndian);
            basicInfo.append(Tlv(MetaInfoSnacHandler::TlvFirstName, nullTerminatedStringWLength(client->ownerIcqContact()->getFirstName())));
            basicInfo.append(Tlv(MetaInfoSnacHandler::TlvLastName, nullTerminatedStringWLength(client->ownerIcqContact()->getLastName())));
            basicInfo.append(Tlv(MetaInfoSnacHandler::TlvNickname, nullTerminatedStringWLength(client->ownerIcqContact()->getNick())));

            QByteArray data = basicInfo.toByteArray();

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(data.size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
            metaPacket.appendBytes(data);

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makeHomeInfoSetPacket(int sqnum)
        {
            TlvList homeInfo(TlvList::LittleEndian);
            homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeCity, nullTerminatedStringWLength(client->ownerIcqContact()->getCity())));
            homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeState, nullTerminatedStringWLength(client->ownerIcqContact()->getState())));
            homeInfo.append(Tlv::fromUint16(MetaInfoSnacHandler::TlvHomeCountry, client->ownerIcqContact()->getCountry()));
            homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeAddress, nullTerminatedStringWLength(client->ownerIcqContact()->getAddress())));
            homeInfo.append(Tlv::fromUint16(MetaInfoSnacHandler::TlvHomeZip, client->ownerIcqContact()->getZip().toUInt()));
            homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomePhone, nullTerminatedStringWLength(client->ownerIcqContact()->getHomePhone())));
            homeInfo.append(Tlv(MetaInfoSnacHandler::TlvHomeFax, nullTerminatedStringWLength(client->ownerIcqContact()->getHomeFax())));
            homeInfo.append(Tlv(MetaInfoSnacHandler::TlvCellular, nullTerminatedStringWLength(client->ownerIcqContact()->getCellular())));

            QByteArray data = homeInfo.toByteArray();

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(data.size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoRequest);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaSetFullUserInfo);
            metaPacket.appendBytes(data);

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray nullTerminatedStringWLength(const QString& str)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendWord(str.length() + 1);
            builder.appendBytes(str.toAscii());
            builder.appendByte(0);
            return builder.getArray();
        }

        void appendString(ByteArrayBuilder& builder, const QString str)
        {
            builder.appendWord(str.size() + 1);
            builder.appendBytes(str.toAscii());
            builder.appendByte(0);
        }

        QByteArray makeBasicInfoPacket(int sqnum, unsigned int targetUin)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendByte(0x0a); //Success byte
            appendString(builder, Nickname);
            appendString(builder, FirstName);
            appendString(builder, LastName);
            appendString(builder, Email);
            appendString(builder, HomeCity);
            appendString(builder, HomeState);
            appendString(builder, HomePhone);
            appendString(builder, HomeFax);
            appendString(builder, HomeAddress);
            appendString(builder, CellPhone);
            appendString(builder, HomeZip);
            builder.appendWord(HomeCountry);
            builder.appendByte(GmtOffset);
            builder.appendByte(0); // Auth flag
            builder.appendByte(0); // Webaware flag
            builder.appendByte(0); // DC flag
            builder.appendByte(0); // publish primary mail flag

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(builder.getArray().size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoData);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaBasicUserInfo);
            metaPacket.appendBytes(builder.getArray());

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makeWorkInfoPacket(int sqnum, unsigned int targetUin)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendByte(0x0a); //Success byte
            appendString(builder, WorkCity);
            appendString(builder, WorkState);
            appendString(builder, WorkPhone);
            appendString(builder, WorkFax);
            appendString(builder, WorkAddress);
            appendString(builder, WorkZip);
            builder.appendWord(WorkCountry);
            appendString(builder, WorkCompany);
            appendString(builder, WorkDepartment);
            appendString(builder, WorkPosition);
            builder.appendWord(WorkOccupation);
            appendString(builder, WorkHomePage);

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(builder.getArray().size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoData);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaWorkUserInfo);
            metaPacket.appendBytes(builder.getArray());

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makeMoreInfoPacket(int sqnum, unsigned int targetUin)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendByte(0x0a); //Success byte
            builder.appendWord(Age);
            builder.appendByte(Gender);
            appendString(builder, HomePage);
            builder.appendWord(Birthday.year());
            builder.appendByte(Birthday.month());
            builder.appendByte(Birthday.day());
            builder.appendByte(PrimaryLanguage);
            builder.appendByte(SecondaryLanguage);
            builder.appendByte(TertiaryLanguage);
            builder.appendWord(0); // Unknown
            appendString(builder, OriginalCity);
            appendString(builder, OriginalState);
            builder.appendWord(OriginalCountry);
            builder.appendByte(OriginalTimezone);

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(builder.getArray().size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoData);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaMoreUserInfo);
            metaPacket.appendBytes(builder.getArray());

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makeAboutInfoPacket(int sqnum, unsigned int targetUin)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendByte(0x0a); //Success byte
            appendString(builder, AboutInfo);

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(builder.getArray().size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoData);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaAboutUserInfo);
            metaPacket.appendBytes(builder.getArray());

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makeInterestsInfoPacket(int sqnum, unsigned int targetUin)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendByte(0x0a); //Success byte
            builder.appendByte(0x04);

            for(int i = 0; i < 4; i++)
            {
                builder.appendWord(InterestCode[i]);
                appendString(builder, InterestText[i]);
            }

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(builder.getArray().size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoData);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInterestsUserInfo);
            metaPacket.appendBytes(builder.getArray());

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }

        QByteArray makePastInfoPacket(int sqnum, unsigned int targetUin)
        {
            ByteArrayBuilder builder(ByteArrayBuilder::LittleEndian);
            builder.appendByte(0x0a); //Success byte

            builder.appendByte(0x03);
            for(int i = 0; i < 3; i++)
            {
                builder.appendWord(AffiliationCode[i]);
                appendString(builder, AffiliationText[i]);
            }

            builder.appendByte(0x03);
            for(int i = 0; i < 3; i++)
            {
                builder.appendWord(BackgroundCode[i]);
                appendString(builder, BackgroundText[i]);
            }

            ByteArrayBuilder metaPacket(ByteArrayBuilder::LittleEndian);
            metaPacket.appendWord(builder.getArray().size() + 10);
            metaPacket.appendDword(client->ownerIcqContact()->getUin());
            metaPacket.appendWord(MetaInfoSnacHandler::MetaInfoData);
            metaPacket.appendWord(sqnum);
            metaPacket.appendWord(MetaInfoSnacHandler::MetaPastUserInfo);
            metaPacket.appendBytes(builder.getArray());

            TlvList tlvs;
            tlvs.append(Tlv(0x01, metaPacket.getArray()));
            return tlvs.toByteArray();
        }


        ICQClient* client;
        MetaInfoSnacHandler* handler;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        ICQContactPtr contact;
    };

    TEST_F(TestMetaInfoSnacHandler, fullinfoRequest_sends_correct_packet)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId, MetaInfoSnacHandler::SnacMetaInfoRequest, _, makeFullinfoRequestPacket(1, TargetUin)));

        handler->requestFullInfo(contact);
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_emits_contact_updated_event)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_basic_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_emits_contact_updated_event_with_correct_target_uin)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_basic_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        QList<QVariant> arguments = spy.takeFirst();
        ASSERT_EQ(QString::number(TargetUin), arguments.at(0).toString());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_nickname)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(Nickname, contact->getNick());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_firstname)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(FirstName, contact->getFirstName());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_lastname)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(LastName, contact->getLastName());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_email)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(Email, contact->getEmail());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homecity)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomeCity, contact->getCity());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homestate)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomeState, contact->getState());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homephone)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomePhone, contact->getHomePhone());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homefax)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomeFax, contact->getHomeFax());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homeaddress)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomeAddress, contact->getAddress());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_cellphone)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(CellPhone, contact->getCellular());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homezip)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomeZip, contact->getZip());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_homecountry)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomeCountry, contact->getCountry());
    }

    TEST_F(TestMetaInfoSnacHandler, basicInfoResponse_sets_gmtoffset)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(GmtOffset, contact->getTimeZone());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_emits_contact_updated_event)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_work_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workCity)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkCity, contact->getWorkCity());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workState)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkState, contact->getWorkState());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workPhone)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkPhone, contact->getWorkPhone());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workFax)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkFax, contact->getWorkFax());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workAddress)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkAddress, contact->getWorkAddress());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workZip)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkZip, contact->getWorkZip());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workCountry)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkCountry, contact->getWorkCountry());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workCompany)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkCompany, contact->getWorkName());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workDepartment)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkDepartment, contact->getWorkDepartment());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workPosition)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkPosition, contact->getWorkPosition());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_occupation)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkOccupation, contact->getOccupation());
    }

    TEST_F(TestMetaInfoSnacHandler, workInfoResponse_sets_workHomePage)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeWorkInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(WorkHomePage, contact->getWorkHomepage());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_emits_contact_updated_event)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_more_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_age)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(Age, contact->getAge());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_gender)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(Gender, contact->getGender());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_homepage)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(HomePage, contact->getHomepage());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_birthday)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(Birthday, contact->getBirthday());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_primaryLanguage)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(PrimaryLanguage, contact->getPrimaryLanguage());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_secondaryLanguage)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(SecondaryLanguage, contact->getSecondaryLanguage());
    }

    TEST_F(TestMetaInfoSnacHandler, moreInfoResponse_sets_tertiaryLanguage)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeMoreInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(TertiaryLanguage, contact->getTertiaryLanguage());
    }

    TEST_F(TestMetaInfoSnacHandler, aboutInfoResponse_emits_contact_updated_event)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_about_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeAboutInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestMetaInfoSnacHandler, aboutInfoResponse_sets_aboutInfo)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeAboutInfoPacket(1, TargetUin), 0, 0);

        ASSERT_EQ(AboutInfo, contact->getAbout());
    }

    TEST_F(TestMetaInfoSnacHandler, interestsInfoResponse_emits_contact_updated_event)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_interests_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeInterestsInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestMetaInfoSnacHandler, interestsInfoResponse_sets_interests)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeInterestsInfoPacket(1, TargetUin), 0, 0);

        for(int i = 0; i < 4; i++)
        {
            ASSERT_EQ(InterestCode[i], contact->getInterest(i));
            ASSERT_EQ(InterestText[i], contact->getInterestText(i));
        }
    }

    TEST_F(TestMetaInfoSnacHandler, pastInfoResponse_emits_contact_updated_event)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_past_info_updated");
        QSignalSpy spy(event.data(), SIGNAL(eventTriggered(QString)));
        handler->requestFullInfo(contact); // Binds contact to current sqnum

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makePastInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestMetaInfoSnacHandler, pastInfoResponse_sets_affiliation)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makePastInfoPacket(1, TargetUin), 0, 0);

        for(int i = 0; i < 3; i++)
        {
            ASSERT_EQ(AffiliationCode[i], contact->getAffiliationCode(i));
            ASSERT_EQ(AffiliationText[i], contact->getAffiliationText(i));
        }
    }

    TEST_F(TestMetaInfoSnacHandler, pastInfoResponse_sets_background)
    {
        handler->requestFullInfo(contact);

        handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makePastInfoPacket(1, TargetUin), 0, 0);

        for(int i = 0; i < 3; i++)
        {
            ASSERT_EQ(BackgroundCode[i], contact->getBackgroundCode(i));
            ASSERT_EQ(BackgroundText[i], contact->getBackgroundText(i));
        }
    }

    TEST_F(TestMetaInfoSnacHandler, setBasicInfo_sends_metaRequestPacket)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId, MetaInfoSnacHandler::SnacMetaInfoRequest, _, _));

        handler->uploadBasicInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setBasicInfo_sends_correct_metaRequestPacket)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId, MetaInfoSnacHandler::SnacMetaInfoRequest, _, makeBasicInfoSetPacket(1)));

        handler->uploadBasicInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setHomeInfo_sends_correct_metaRequestPacket)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId, MetaInfoSnacHandler::SnacMetaInfoRequest, _, makeHomeInfoSetPacket(1)));

        handler->uploadHomeInfo();
    }

    static bool validWorkInfoSetPacketHeader(const QByteArray& arr)
    {
        TlvList list = TlvList::fromByteArray(arr);
        Tlv metaData = list.firstTlv(0x01);
        if(!metaData.isValid())
            return false;

        ByteArrayParser parser(metaData.data(), ByteArrayParser::LittleEndian);
        int length = parser.readWord();
        if(length != metaData.data().size() - 2)
            return false;

        int uin = parser.readDword();
        if(uin != OwnerUin)
            return false;

        int datatype = parser.readWord();
        if(datatype != MetaInfoSnacHandler::MetaInfoRequest)
            return false;

        parser.readWord(); //sqnum

        int reqtype = parser.readWord();
        if(reqtype != MetaInfoSnacHandler::MetaSetFullUserInfo)
            return false;

        return true;
    }

    class MetaRequestHasTlv
    {
    public:
        MetaRequestHasTlv(int tlvId) : m_id(tlvId) {}
        bool operator()(const QByteArray& arr) const
        {
            TlvList list = TlvList::fromByteArray(arr);
            Tlv metaData = list.firstTlv(0x01);
            ByteArrayParser parser(metaData.data(), ByteArrayParser::LittleEndian);
            parser.readWord(); // length
            parser.readDword(); // uin
            parser.readWord(); // datatype
            parser.readWord(); // sqnum
            parser.readWord(); // reqtype

            TlvList tlvs = TlvList::fromByteArray(parser.readAll(), TlvList::LittleEndian);
            return tlvs.contains(m_id);
        }

    private:
        int m_id;
    };

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_correct_metaRequestPacket)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId, MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(validWorkInfoSetPacketHeader)));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workCompanyTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkCompany))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workDepartmentTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkDepartment))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workPositionTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkPosition))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workOccupationTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkOccupation))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workAddressTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkAddress))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workCityTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkCity))));

        handler->uploadWorkInfo();
    }

// This tlv causes request to fail, so it is disabled for now
//    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workStateTlv)
//    {
//        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
//                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkState))));
//
//        handler->uploadWorkInfo();
//    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workCountryTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkCountry))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workZipTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkZip))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workPhoneTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkPhone))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workFaxTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkFax))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setWorkInfo_sends_metaRequestPacket_with_workHomepageTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvWorkHomepage))));

        handler->uploadWorkInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setMoreInfo_sends_metaRequestPacket_with_ageTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvAge))));

        handler->uploadMoreInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setMoreInfo_sends_metaRequestPacket_with_birthdayTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvBirthday))));

        handler->uploadMoreInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setMoreInfo_sends_metaRequestPacket_with_languageTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvLanguage))));

        handler->uploadMoreInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setAboutInfo_sends_metaRequestPacket_with_aboutTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvAbout))));

        handler->uploadAboutInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setInterestsInfo_sends_metaRequestPacket_with_interestsTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvInterest))));

        handler->uploadInterestsInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setPastInfo_sends_metaRequestPacket_with_backgroundTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvBackground))));

        handler->uploadPastInfo();
    }

    TEST_F(TestMetaInfoSnacHandler, setPastInfo_sends_metaRequestPacket_with_affiliationTlv)
    {
        EXPECT_CALL(*socket, snac(MetaInfoSnacHandler::SnacId,
                MetaInfoSnacHandler::SnacMetaInfoRequest, _, Truly(MetaRequestHasTlv(MetaInfoSnacHandler::TlvAffiliation))));

        handler->uploadPastInfo();
    }
}
