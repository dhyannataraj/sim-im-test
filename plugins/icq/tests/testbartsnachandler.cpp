/*
 * testbartsnachandler.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include <QCryptographicHash>

#include "servicesnachandler.h"
#include "bartsnachandler.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mocks/mockoscarsocket.h"
#include "tests/mocks/mockavatarstorage.h"
#include "imagestorage/imagestorage.h"
#include "mocks/mockicqrequestmanager.h"
#include "icqclient.h"

namespace
{
    using ::testing::NiceMock;
    using ::testing::_;
    using ::testing::InSequence;
    using ::testing::Return;

    static QImage TestAvatar;
    static QString ContactId = "123456";
    static QString ServiceIp = "192.168.1.1";
    static const int ServicePort = 5190;
    static const QByteArray AuthCookie = QByteArray(16, 0x12);

    class TestBartSnacHandler : public ::testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            TestAvatar = QImage(QSize(100, 100), QImage::Format_RGB32);
            TestAvatar.fill(0x12345678);
        }

        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            ON_CALL(*socket, isConnected()).WillByDefault(Return(true));
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setScreen(ContactId);
            client->contactList()->addContact(contact);

            handler = static_cast<BartSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_SSI));
            ASSERT_TRUE(handler);

            bartSocket = new NiceMock<MockObjects::MockOscarSocket>();
            handler->setOscarSocket(bartSocket);
            ON_CALL(*bartSocket, isConnected()).WillByDefault(Return(true));

            avatarStorage = new MockObjects::MockAvatarStorage();
            SIM::setAvatarStorage(avatarStorage);
        }

        virtual void TearDown()
        {
            SIM::setAvatarStorage(0); // deletes it
            delete client;
        }

        QByteArray makeAvatarResponse()
        {
            QBuffer b;
            TestAvatar.save(&b, "png");
            QByteArray hash = QCryptographicHash::hash(b.data(), QCryptographicHash::Md5);

            ByteArrayBuilder builder;
            builder.appendByte(ContactId.length());
            builder.appendBytes(ContactId.toAscii());
            builder.appendWord(0x0001);
            builder.appendByte(0x01);
            builder.appendByte(hash.length());
            builder.appendBytes(hash);
            builder.appendWord(b.data().length());
            builder.appendBytes(b.data());

            return builder.getArray();
        }

        QByteArray makeAuthPacket()
        {
            ByteArrayBuilder builder;
            builder.appendDword(0x01);

            TlvList tlvs;
            tlvs.append(Tlv(ServiceSnacHandler::TlvAuthCookie, AuthCookie));

            builder.appendBytes(tlvs.toByteArray());

            return builder.getArray();
        }

        QByteArray makeServerReadyPacket()
        {
            ByteArrayBuilder builder;
            builder.appendWord(ServiceSnacHandler::SnacId);
            builder.appendWord(BartSnacHandler::SnacId);
            return builder.getArray();
        }

        ICQClient* client;
        ICQContactPtr contact;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        NiceMock<MockObjects::MockOscarSocket>* bartSocket;
        BartSnacHandler* handler;
        MockObjects::MockAvatarStorage* avatarStorage;
    };

    TEST_F(TestBartSnacHandler, requestAvatar)
    {
        EXPECT_CALL(*bartSocket, snac(handler->getType(), BartSnacHandler::SnacRequestAvatar, _, _));

        handler->requestAvatar("123456", QByteArray::fromHex("00112233445566778899aabbccddeeff"));
    }

    TEST_F(TestBartSnacHandler, avatarResponse_changesContactAvatar)
    {
        EXPECT_CALL(*avatarStorage, addAvatar(contact->id(), _, _));

        handler->process(BartSnacHandler::SnacResponseAvatar, makeAvatarResponse(), 0, 0);
    }

    TEST_F(TestBartSnacHandler, requestBartService_doesRequestToManager)
    {
        EXPECT_CALL(*socket, snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRequestService, _, _));

        handler->requestBartService();
    }

    TEST_F(TestBartSnacHandler, serviceAvailable_connectsToServer)
    {
        EXPECT_CALL(*bartSocket, connectToHost(ServiceIp, ServicePort));

        handler->bartServiceAvailable(QString("%1:%2").arg(ServiceIp).arg(ServicePort), AuthCookie);
    }

    TEST_F(TestBartSnacHandler, connected_sendsCookie)
    {
        EXPECT_CALL(*bartSocket, flap(0x01, makeAuthPacket()));

        handler->bartServiceAvailable(QString("%1:%2").arg(ServiceIp).arg(ServicePort), AuthCookie);
        bartSocket->provokeConnectedSignal();
    }

    TEST_F(TestBartSnacHandler, serverReady_sendsCaps)
    {
        EXPECT_CALL(*bartSocket, snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceCapabilities, _, _));

        handler->processService(ServiceSnacHandler::SnacServiceServerReady, makeServerReadyPacket(), 0, 0);
    }

    TEST_F(TestBartSnacHandler, capsAck_requestsRateInfo)
    {
        EXPECT_CALL(*bartSocket, snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRateInfoRequest, _, _));

        handler->processService(ServiceSnacHandler::SnacServiceCapabilitiesAck, QByteArray(), 0, 0);
    }

    TEST_F(TestBartSnacHandler, incomingRateInfo_sendsAckRateInfoAndClientReady)
    {
        {
            InSequence seq;
            EXPECT_CALL(*bartSocket, snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRateInfoAck, _, _));
            EXPECT_CALL(*bartSocket, snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceReady, _, _));
        }

        handler->processService(ServiceSnacHandler::SnacServiceRateInfo, QByteArray(), 0, 0);
    }

    TEST_F(TestBartSnacHandler, disconnect_disconnectsFromHost)
    {
        EXPECT_CALL(*bartSocket, disconnectFromHost());

        handler->disconnect();
    }
}
