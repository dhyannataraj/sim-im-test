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

    using ::testing::_;

    class TestMetaInfoSnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.666666", false);
            client->ownerIcqContact()->setUin(OwnerUin);
            handler = client->metaInfoSnacHandler();
            socket = new MockObjects::MockOscarSocket();
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

        void appendString(ByteArrayBuilder& builder, const QString str)
        {
            builder.appendWord(str.size() + 1);
            builder.appendBytes(str.toAscii());
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

        ICQClient* client;
        MetaInfoSnacHandler* handler;
        MockObjects::MockOscarSocket* socket;
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

        bool success = handler->process(MetaInfoSnacHandler::SnacMetaInfoData, makeBasicInfoPacket(1, TargetUin), 0, 0);

        ASSERT_TRUE(success);

        QList<QVariant> arguments = spy.takeFirst();
        ASSERT_EQ(QString::number(TargetUin), arguments.at(0).toString());
    }
}
