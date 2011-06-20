
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mocks/mockclient.h"
#include "mocks/mockclientmanager.h"
#include "mocks/mockmessage.h"
#include "mocks/mockimcontact.h"
#include "clientmanager.h"
#include "messaging/standardmessageoutpipe.h"

namespace
{
    using ::testing::Return;
    using ::testing::_;
    class TestStandardMessageOutPipe : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            pipe = new SIM::StandardMessageOutPipe();
            contact = MockObjects::MockIMContact::create();
        }

        virtual void TearDown()
        {
            delete pipe;
        }

        MockObjects::MockIMContactPtr contact;

        SIM::StandardMessageOutPipe* pipe;
    };

    TEST_F(TestStandardMessageOutPipe, pushMessageSendsMessage)
    {
        MockObjects::MockMessagePtr message = MockObjects::MockMessage::create();
        SIM::IMContactWeakPtr weakContactPtr = contact.toWeakRef();
        EXPECT_CALL(*message.data(), targetContact()).WillRepeatedly(Return(contact));
        EXPECT_CALL(*contact.data(), sendMessage(_)).Times(1);

        pipe->pushMessage(message);
    }
}

