
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
#include "tests/mocks/mockmessagepipe.h"
#include "messaging/messagepipe.h"
#include "tests/matchers/messagematchers.h"
#include "requests/icbmsnac/icbmsnacparametersrequest.h"
#include "requests/icbmsnac/icbmsnacsendparametersrequest.h"

namespace
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;
    static const int MinMessageInterval = 1000;
    static const QString ContactName = "32167";
    static const QString TestMessage = "1234";

    static const QByteArray TestCp1251Message("\xcf\xf0\xe8\xe2\xe5\xf2");
    static const QString TestCp1251DecodedMessage = QString::fromUtf8("Привет"); // Means "hello" in russian

    static const QByteArray TestUtf16beMessage("\x04\x1f\x04\x40\x04\x38\x04\x32\x04\x35\x04\x42");
    static const QString TestUtf16beDecodedMessage = QString::fromUtf8("Привет"); // Means "hello" in russian

    class TestIcbmSnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            ON_CALL(*socket, isConnected()).WillByDefault(Return(true));
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setScreen(ContactName);
            client->contactList()->addContact(contact);

            handler = static_cast<IcbmSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_MESSAGE));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
            SIM::setMessagePipe(0);
        }

        IcbmSnacHandler::IcbmParameters makeDefaultIcbmParameters()
        {
            IcbmSnacHandler::IcbmParameters params;
            params.messageFlags = 0x03;
            params.maxSnacSize = 0x200;
            params.maxSenderWarnLevel = 0x384;
            params.maxReceiverWarnLevel = 0x3e7;
            params.minMessageInterval = MinMessageInterval;
            return params;
        }

        QByteArray makeParametersInfoPacket(int channel, const IcbmSnacHandler::IcbmParameters& params)
        {
            ByteArrayBuilder builder;
            builder.appendWord(channel);
            builder.appendDword(params.messageFlags); // Flags
            builder.appendWord(params.maxSnacSize); // Max snac size
            builder.appendWord(params.maxSenderWarnLevel); // Max sender warn level
            builder.appendWord(params.maxReceiverWarnLevel); // Max receiver warn level
            builder.appendDword(params.minMessageInterval);
            return builder.getArray();
        }

        QByteArray makeMessageBlock(const QByteArray& messageText, int charset)
        {
            ByteArrayBuilder builder;
            builder.appendWord(0x0501); // Features signature
            builder.appendWord(0x01); // Features length
            builder.appendByte(0x01); // Features
            builder.appendWord(0x0101); // Block info
            builder.appendWord(0x04 + messageText.length()); // Block info length
            builder.appendWord(charset);
            builder.appendWord(0);
            builder.appendBytes(messageText);
            return builder.getArray();
        }

        SIM::MessagePtr makeTestMessage(const QString& message)
        {
            return SIM::MessagePtr(new SIM::GenericMessage(client->ownerContact(), contact, message));
        }

        QByteArray makeIncomingMessagePacket(const QByteArray& text, int charset = 0)
        {
            ByteArrayBuilder builder;
            builder.appendDword(0x12345678);
            builder.appendDword(0x87654321);
            builder.appendWord(0x0001);
            builder.appendByte(ContactName.length());
            QByteArray arr = ContactName.toAscii();
            builder.appendBytes(arr);
            builder.appendWord(0); // Warning level`

            TlvList list;
            list.append(Tlv::fromUint16(IcbmSnacHandler::TlvUserClass, 0x0050));
            list.append(Tlv::fromUint32(IcbmSnacHandler::TlvOnlineStatus, 0x10020000));
            list.append(Tlv(IcbmSnacHandler::TlvMessage, makeMessageBlock(text, charset)));

            builder.appendWord(list.size());
            builder.appendBytes(list.toByteArray());

            return builder.getArray();
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
        if(targetContactId != ContactName.toAscii())
            return false;

        if(parser.atEnd())
            return false;

        // Check Tlv 0x02 presence
        TlvList list = TlvList::fromByteArray(parser.readAll());
        if(!list.contains(IcbmSnacHandler::TlvMessage))
            return false;
        return true;
    }

    bool isMessageEncodedInUtf16be(const QByteArray& packet)
    {
        ByteArrayParser parser(packet);
        parser.readDword();
        parser.readDword(); // skip cookies
        parser.readWord(); // channel number

        int idlen = parser.readByte();
        parser.readBytes(idlen); // Target Id

        TlvList list = TlvList::fromByteArray(parser.readAll());
        Tlv msg = list.firstTlv(IcbmSnacHandler::TlvMessage);

        ByteArrayParser msgParser(msg.data());
        msgParser.readWord(); // features signature
        int featuresLength = msgParser.readWord();
        msgParser.readBytes(featuresLength);
        msgParser.readWord(); // message signature
        int msgLength = msgParser.readWord() - 4; // message block length
        int charset = msgParser.readWord();
        if(charset != IcbmSnacHandler::CharsetUtf16be)
            return false;
        msgParser.readWord(); // Charsubset
        QByteArray arr = msgParser.readBytes(msgLength);
        if(arr != TestUtf16beMessage)
            return false;
        return true;
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_processing)
    {
        IcbmSnacHandler::IcbmParameters params = makeDefaultIcbmParameters();
        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(4, params), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(params.minMessageInterval, handler->minMessageInterval());
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_sendNewParameters)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSetParameters, _, _)).Times(1);

        IcbmSnacHandler::IcbmParameters params = makeDefaultIcbmParameters();
        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(4, params), 0, 0);
        ASSERT_TRUE(success);
    }

    TEST_F(TestIcbmSnacHandler, parametersPacket_ready)
    {
        QSignalSpy spy(handler, SIGNAL(ready()));
        IcbmSnacHandler::IcbmParameters params = makeDefaultIcbmParameters();
        bool success = handler->process(IcbmSnacHandler::SnacIcbmParametersInfo, makeParametersInfoPacket(4, params), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
        ASSERT_TRUE(handler->isReady());
    }

    TEST_F(TestIcbmSnacHandler, sendMessage_plainTextMessage)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSendMessage, _, _)).Times(1);
        handler->sendMessage(makeTestMessage(TestMessage));
    }

    TEST_F(TestIcbmSnacHandler, sendMessage_plainTextMessage_validMessage)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSendMessage, _, Truly(isValidMessagePacketForContact))).Times(1);
        handler->sendMessage(makeTestMessage(TestMessage));
    }

    TEST_F(TestIcbmSnacHandler, sendMessage_plainTextMessage_encodes_in_utf16be)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_MESSAGE, IcbmSnacHandler::SnacIcbmSendMessage, _, Truly(isMessageEncodedInUtf16be))).Times(1);
        handler->sendMessage(makeTestMessage(TestUtf16beDecodedMessage));
    }

    TEST_F(TestIcbmSnacHandler, incomingMessage_pushesToPipe)
    {
        MockObjects::MockMessagePipe* pipe = new MockObjects::MockMessagePipe();
        SIM::setMessagePipe(pipe);
        EXPECT_CALL(*pipe, pushMessage(Truly(Matcher::MessageTextMatcher(TestMessage))));

        handler->process(IcbmSnacHandler::SnacIcbmIncomingMessage, makeIncomingMessagePacket(TestMessage.toUtf8()), 0, 0);
    }

    TEST_F(TestIcbmSnacHandler, incomingMessage_sourceContact)
    {
        MockObjects::MockMessagePipe* pipe = new MockObjects::MockMessagePipe();
        SIM::setMessagePipe(pipe);
        EXPECT_CALL(*pipe, pushMessage(Truly(Matcher::MessageSourceContactMatcher(contact))));

        handler->process(IcbmSnacHandler::SnacIcbmIncomingMessage, makeIncomingMessagePacket(TestMessage.toUtf8()), 0, 0);
    }


    TEST_F(TestIcbmSnacHandler, incomingMessage_uses_contact_encoding_if_packet_encoding_is_unknown)
    {
        contact->setEncoding("cp1251");
        MockObjects::MockMessagePipe* pipe = new MockObjects::MockMessagePipe();
        SIM::setMessagePipe(pipe);
        EXPECT_CALL(*pipe, pushMessage(Truly(Matcher::MessageTextMatcher(TestCp1251DecodedMessage))));

        handler->process(IcbmSnacHandler::SnacIcbmIncomingMessage, makeIncomingMessagePacket(TestCp1251Message), 0, 0);
    }

    TEST_F(TestIcbmSnacHandler, incomingMessage_uses_packet_encoding_utf16le)
    {
        contact->setEncoding("cp1251");
        MockObjects::MockMessagePipe* pipe = new MockObjects::MockMessagePipe();
        SIM::setMessagePipe(pipe);
        EXPECT_CALL(*pipe, pushMessage(Truly(Matcher::MessageTextMatcher(TestUtf16beDecodedMessage))));

        handler->process(IcbmSnacHandler::SnacIcbmIncomingMessage, makeIncomingMessagePacket(TestUtf16beMessage, IcbmSnacHandler::CharsetUtf16be), 0, 0);
    }

    TEST_F(TestIcbmSnacHandler, requestParametersInfo_sendsSnac)
    {
        EXPECT_CALL(*socket, snac(handler->getType(), IcbmSnacHandler::SnacIcbmParametersInfoRequest, _, _));

        handler->requestParametersInfo();
    }

    TEST_F(TestIcbmSnacHandler, Request_requestParameters)
    {
        ICQRequestPtr rq = IcbmSnacParametersRequest::create(client);
        EXPECT_CALL(*socket, snac(IcbmSnacHandler::SnacId, IcbmSnacHandler::SnacIcbmParametersInfoRequest, _, _));

        rq->perform(socket);
    }

    TEST_F(TestIcbmSnacHandler, Request_sendNewParameters)
    {
        IcbmSnacHandler::IcbmParameters params = makeDefaultIcbmParameters();
        ICQRequestPtr rq = IcbmSnacSendParametersRequest::create(client, 0, params);
        EXPECT_CALL(*socket, snac(IcbmSnacHandler::SnacId, IcbmSnacHandler::SnacIcbmSetParameters, _, _));

        rq->perform(socket);
    }

    TEST_F(TestIcbmSnacHandler, Request_sendNewParameters_packetStructure)
    {
        IcbmSnacHandler::IcbmParameters params = makeDefaultIcbmParameters();
        ICQRequestPtr rq = IcbmSnacSendParametersRequest::create(client, 0, params);
        EXPECT_CALL(*socket, snac(IcbmSnacHandler::SnacId, IcbmSnacHandler::SnacIcbmSetParameters, _, makeParametersInfoPacket(0, params)));

        rq->perform(socket);
    }
}
