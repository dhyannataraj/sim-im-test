#include <gtest/gtest.h>
#include <QSignalSpy>
#include "mocks/mockoscarsocket.h"
#include "icqclient.h"
#include "servicesnachandler.h"
#include "rateinfo.h"
#include "requests/servicesnac/servicesnacservicerequest.h"

namespace
{
    using ::testing::_;
    using ::testing::AnyNumber;
    using ::testing::NiceMock;

    static const int TestServiceId = 0x10;
    static const QString ServiceAddress = "192.168.1.1:5190";
    static const QByteArray ServiceCookie = QByteArray(16, 0x12);

    class TestServiceSnacHandler : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<ServiceSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_SERVICE));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeServerServicesPacket()
        {
            ByteArrayBuilder builder;
            // FIXME:
            builder.appendWord(0x0001);
            return socket->makeSnacPacket(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceServerReady, 0, builder.getArray());
        }

        QByteArray makeServerCapabilitiesAckPacket()
        {
            ByteArrayBuilder builder;
            builder.appendDword(0x00010004);

            return socket->makeSnacPacket(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceCapabilitiesAck, 0, builder.getArray());
        }

        QByteArray makeRateInfoReply()
        {
            ByteArrayBuilder builder;
            int rateGroups = 5;
            builder.appendWord(rateGroups);
            for(int group = 0; group < rateGroups; group++)
            {
                builder.appendWord(group + 1);
                // More or less arbitrary values:
                builder.appendDword(0x50);
                builder.appendDword(0x9c4);
                builder.appendDword(0x7d0);
                builder.appendDword(0x5dc);
                builder.appendDword(0x320);
                builder.appendDword(0x1600);
                builder.appendDword(0x1700);
                builder.appendDword(0);
                builder.appendByte(0);
            }

            for(int group = 0; group < rateGroups; group++)
            {
                builder.appendWord(group + 1);
                builder.appendWord(2); // Two entries below
                builder.appendDword(0x00010002);
                builder.appendDword(0x00010004);
            }
            return socket->makeSnacPacket(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRateInfo, 0, builder.getArray());
        }

        QByteArray makeStatusReplyPacket()
        {
            return QByteArray("\x00\x02\x04\x00", 4);
        }

        QByteArray makeAcknowledgedRateInfoClassesPacket()
        {
            return QByteArray("\x00\x01\x00\x02\x00\x03\x00\x04\x00\x05", 10);
        }

        QByteArray makeServiceResponsePacket()
        {
            TlvList tlvs;

            tlvs.append(Tlv::fromUint16(ServiceSnacHandler::TlvServiceId, TestServiceId));
            tlvs.append(Tlv(ServiceSnacHandler::TlvServiceAddress, ServiceAddress.toAscii()));
            tlvs.append(Tlv(ServiceSnacHandler::TlvAuthCookie, ServiceCookie));

            return tlvs.toByteArray();
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        ServiceSnacHandler* handler;
    };

    TEST_F(TestServiceSnacHandler, negotiation_serverReady)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceCapabilities, _, _));
        QByteArray serverServices = makeServerServicesPacket();
        socket->provokePacketSignal(0x02, serverServices);
    }

    TEST_F(TestServiceSnacHandler, negotiation_capabilitiesAck)
    {
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRateInfoRequest, _, _));
        QByteArray capabilitiesAck = makeServerCapabilitiesAckPacket();
        socket->provokePacketSignal(0x02, capabilitiesAck);
    }

    TEST_F(TestServiceSnacHandler, negotiation_rateInfoReply)
    {
        QByteArray acknowledgedRateInfoClasses = makeAcknowledgedRateInfoClassesPacket();
        EXPECT_CALL(*socket, snac(_, _, _, _)).Times(AnyNumber()); // Lest the gmock fail the test
        EXPECT_CALL(*socket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRateInfoAck, _, acknowledgedRateInfoClasses));
        QByteArray rateInfoReply = makeRateInfoReply();
        socket->provokePacketSignal(0x02, rateInfoReply);
    }

    TEST_F(TestServiceSnacHandler, negotiation_rateInfoReply_parsesRateInfoCorrectly)
    {
        QByteArray rateInfoReply = makeRateInfoReply();
        socket->provokePacketSignal(0x02, rateInfoReply);

        RateInfoPtr info = handler->rateInfo(1);
        ASSERT_TRUE(info);
        ASSERT_TRUE(info->hasSnac(0x0001, 0x0002));
    }

    TEST_F(TestServiceSnacHandler, incomingServerPacket_emitsSignal)
    {
        QSignalSpy spy(handler, SIGNAL(serviceAvailable(int,QString,QByteArray)));

        handler->process(ServiceSnacHandler::SnacServiceAvailable, makeServiceResponsePacket(), 0, 0);

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestServiceSnacHandler, Request_serviceRequest_sendsSnac)
    {
        EXPECT_CALL(*socket, snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRequestService, _, _));

        ICQRequestPtr rq = ServiceSnacServiceRequest::create(client, 0x10);
        rq->perform(socket);
    }

    TEST_F(TestServiceSnacHandler, serviceStatusInfo_emitsSignal)
    {
        QSignalSpy spy(handler, SIGNAL(statusTransitionComplete()));

        bool success = handler->process(ServiceSnacHandler::SnacServiceStatus, makeStatusReplyPacket(), 0, 0);

        ASSERT_TRUE(success);
        ASSERT_EQ(1, spy.count());
    }
}
