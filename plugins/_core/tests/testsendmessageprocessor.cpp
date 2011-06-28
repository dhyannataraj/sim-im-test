/*
 * testsendmessageprocessor.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: todin
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mocks/mockcontainermanager.h"
#include "container/sendmessageprocessor.h"
#include "messaging/messagepipe.h"
#include "tests/stubs/stubmessage.h"

namespace
{
    using ::testing::_;
    class TestSendMessageProcessor : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            SIM::createOutMessagePipe();
            manager = new MockObjects::MockContainerManager();
            processor = new SendMessageProcessor(manager);
            SIM::getOutMessagePipe()->addMessageProcessor(processor);
        }

        virtual void TearDown()
        {
            delete processor;
            delete manager;
            SIM::destroyOutMessagePipe();
        }

        SendMessageProcessor* processor;
        MockObjects::MockContainerManager* manager;
    };

    TEST_F(TestSendMessageProcessor, outcomingMessagePassedToContainerManager)
    {
        EXPECT_CALL(*manager, messageSent(_));

        SIM::getOutMessagePipe()->pushMessage(SIM::MessagePtr(new StubObjects::StubMessage()));
    }
}




