/*
 * testicqrequestmanager.cpp
 *
 *  Created on: Jul 10, 2011
 *      Author: todin
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "icqclient.h"
#include "requests/standardicqrequestmanager.h"
#include "tests/mocks/mockicqrequest.h"
#include "tests/mocks/mockoscarsocket.h"

#include <cstdio>

namespace
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;

    class TestStandardICQRequestManager : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.123456", false);
            manager = client->requestManager();
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            client->setOscarSocket(socket);
        }

        virtual void TearDown()
        {
            delete client;
        }
        ICQRequestManager* manager;
        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
    };

    TEST_F(TestStandardICQRequestManager, enqueue_whenConnected_performsRequest)
    {
        ON_CALL(*socket, isConnected()).WillByDefault(Return(true));
        // Now, ICQRequestManager::enqueue just calls ICQRequest::perform and that's it
        MockObjects::MockICQRequestPtr rq = MockObjects::MockICQRequest::create();
        EXPECT_CALL(*rq.data(), perform(_));

        manager->enqueue(rq);
    }

    TEST_F(TestStandardICQRequestManager, enqueue_whenNotConnected_doesntPerformsRequest)
    {
        ON_CALL(*socket, isConnected()).WillByDefault(Return(false));
        MockObjects::MockICQRequestPtr rq = MockObjects::MockICQRequest::create();
        EXPECT_CALL(*rq.data(), perform(_)).Times(0);

        manager->enqueue(rq);

        manager->clearQueue();
    }
}


