
#include <QSignalSpy>
#include "gtest/gtest.h"
#include "signalspy.h"
#include "qt-gtest.h"
#include "icqclient.h"
#include "privacysnachandler.h"
#include "oscarsocket.h"
#include "mocks/mockoscarsocket.h"

namespace
{
    using ::testing::NiceMock;
    class TestPrivacySnacHandler : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            handler = static_cast<PrivacySnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_BOS));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            delete client;
        }

        QByteArray makeRightsPacket()
        {
            TlvList list;
            list.append(Tlv::fromUint16(PrivacySnacHandler::TlvMaxVisible, 1000));
            list.append(Tlv::fromUint16(PrivacySnacHandler::TlvMaxInvisible, 1000));
            return list.toByteArray();
        }

        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        PrivacySnacHandler* handler;
    };

    TEST_F(TestPrivacySnacHandler, rightsPacket_ready)
    {
        QSignalSpy spy(handler, SIGNAL(ready()));
        bool success = handler->process(PrivacySnacHandler::SnacPrivacyRightsInfo, makeRightsPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
        ASSERT_TRUE(handler->isReady());
    }
}
