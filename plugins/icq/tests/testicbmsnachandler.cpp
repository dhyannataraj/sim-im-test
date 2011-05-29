
#include <QSignalSpy>

#include "gtest/gtest.h"
#include "signalspy.h"
#include "qt-gtest.h"
#include "icqclient.h"
#include "icbmsnachandler.h"
#include "oscarsocket.h"
#include "mocks/mockoscarsocket.h"
#include "messaging/genericmessage.h"
#include "contacts/imcontact.h"
#include "tests/mocks/mockimcontact.h"

namespace
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;
    static const int MinMessageInterval = 1000;
    static const QString TargetContact = "32167";
    static const QString TestMessage = "1234";

    class TestIcbmSnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setScreen(TargetContact);

            handler = static_cast<IcbmSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_MESSAGE));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeParametersInfoPacket()
        {
            ByteArrayBuilder builder;
            builder.appendWord(0x0004); // Channel
            builder.appendDword(0x00000003); // Flags
            builder.appendWord(0x0200); // Max snac size
            builder.appendWord(0x0384); // Max sender warn level
            builder.appendWord(0x03e7); // Max receiver warn level
            builder.appendDword(MinMessageInterval);
            return builder.getArray();
        }

        QByteArray makeMessageBlock(const QString& messageText)
        {
            ByteArrayBuilder builder;
            builder.appendWord(0x0501); // Features signature
            builder.appendByte(0x01); // Features length
            builder.appendByte(0x01); // Features
            builder.appendWord(0x0101); // Block info
            builder.appendByte(0x04 + messageText.length()); // Block info length
            builder.appendWord(0x0000);
            builder.appendWord(0xffff);
            QByteArray arr = messageText.toAscii();
            arr.chop(1);
            builder.appendBytes(arr);
            return builder.getArray();
        }

        SIM::MessagePtr makeTestMessage()
        {
            return SIM::MessagePtr(new SIM::GenericMessage(contact, TestMessage));
        }

        ICQClient* client;
        ICQContactPtr contact;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        IcbmSnacHandler* handler;
    };

    bool isValidMessagePacketForContact(const QByteArray& packet)
    {
        ByteArrayParser parser(packet);
        parser.readDword();
        parser.readDword(); // skip cookies
        // Check channel number
        int channel = parser.readWord();
        if(channel != 0x01)
            return false;

        if(parser.atEnd())
            return false;

        // Check target contact id
        int idlen = parser.readByte();
        QByteArray targetContactId = parser.readBytes(idlen);
        if(targetContactId != TargetContact.toAscii())
            return false;

        if(parser.atEnd())
            return false;

        // Check Tlv 0x02 presence
        TlvList list = TlvList::fromByteArray(parser.readAll());
        if(!list.contains(IcbmSnacHandler::TlvMessage))
            return false;
        return true;
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_processing)
    {
        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(MinMessageInterval, handler->minMessageInterval());
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_sendNewParameters)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSetParameters, _, _)).Times(1);

        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(), 0, 0);
        ASSERT_TRUE(success);
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_ready)
    {
        QSignalSpy spy(handler, SIGNAL(ready()));
        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
        ASSERT_TRUE(handler->isReady());
    }

    TEST_F(TestIcbmSnacHandler, sendMessage_plainTextMessage)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSendMessage, _, _)).Times(1);
        handler->sendMessage(makeTestMessage());
    }

    TEST_F(TestIcbmSnacHandler, sendMessage_plainTextMessage_validMessage)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSendMessage, _, Truly(isValidMessagePacketForContact))).Times(1);
        handler->sendMessage(makeTestMessage());
    }
}
