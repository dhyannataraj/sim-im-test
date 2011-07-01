/*
 * testreceivemessageprocessor.cpp
 *
 *  Created on: Jul 1, 2011
 *      Author: todin
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mocks/mockcontainermanager.h"
#include "container/receivemessageprocessor.h"
#include "messaging/messagepipe.h"
#include "tests/stubs/stubmessage.h"

namespace
{
    using ::testing::_;
    class TestReceiveMessageProcessor : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            manager = new MockObjects::MockContainerManager();
            processor = new ReceiveMessageProcessor(manager);
        }

        virtual void TearDown()
        {
            delete processor;
            delete manager;
        }
        MockObjects::MockContainerManager* manager;
        ReceiveMessageProcessor* processor;
    };

    TEST_F(TestReceiveMessageProcessor, process_callsAddMessageToManager)
    {
        EXPECT_CALL(*manager, messageReceived(_));

        processor->process(SIM::MessagePtr(new StubObjects::StubMessage()));
    }
}

