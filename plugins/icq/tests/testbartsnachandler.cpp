/*
 * testbartsnachandler.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include <QCryptographicHash>

#include "bartsnachandler.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mocks/mockoscarsocket.h"
#include "icqclient.h"

namespace
{
    using ::testing::NiceMock;
    using ::testing::_;

    static QImage TestAvatar;
    static QString ContactId = "123456";

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
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setScreen(ContactId);
            client->contactList()->addContact(contact);

            handler = static_cast<BartSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_SSI));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
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

        ICQClient* client;
        ICQContactPtr contact;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        BartSnacHandler* handler;
    };

    TEST_F(TestBartSnacHandler, requestAvatar)
    {
        EXPECT_CALL(*socket, snac(handler->getType(), BartSnacHandler::SnacRequestAvatar, _, _));

        handler->requestAvatar("123456", QByteArray::fromHex("00112233445566778899aabbccddeeff"));
    }

    TEST_F(TestBartSnacHandler, avatarResponse_changesContactAvatar)
    {
        handler->process(BartSnacHandler::SnacResponseAvatar, makeAvatarResponse(), 0, 0);

        ASSERT_TRUE(TestAvatar == contact->getAvatar());
    }
}
