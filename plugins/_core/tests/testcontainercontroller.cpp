
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "containercontroller.h"
#include "tests/mocks/mockmessageoutpipe.h"
#include "tests/stubs/stubmessage.h"
#include "messaging/messageoutpipe.h"


namespace
{
    static const int ControllerId = 12;
    class TestContainerController : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            pipe = new MockObjects::MockMessageOutPipe();
            SIM::setOutMessagePipe(pipe);

            controller = new ContainerController(0, ControllerId);
        }

        virtual void TearDown()
        {
            delete controller;
            delete pipe;
            SIM::setOutMessagePipe(0);
        }

        ContainerController* controller;
        MockObjects::MockMessageOutPipe* pipe;
    };

    TEST_F(TestContainerController, id_returnsCorrectId)
    {
        ASSERT_EQ(ControllerId, controller->id());
    }

    TEST_F(TestContainerController, sendMessage_pushesMessageToPipe)
    {
        SIM::MessagePtr msg = SIM::MessagePtr(new StubObjects::StubMessage());
        EXPECT_CALL(*pipe, pushMessage(msg)).Times(1);
        controller->sendMessage(msg);
    }
}
