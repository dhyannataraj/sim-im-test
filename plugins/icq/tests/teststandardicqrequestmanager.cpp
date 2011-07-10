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

namespace
{
    using ::testing::_;
    using ::testing::NiceMock;

    class TestStandardICQRequestManager : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.123456", false);
            manager = client->requestManager();
        }

        virtual void TearDown()
        {
            delete client;
        }
        ICQRequestManager* manager;
        ICQClient* client;
    };

    TEST_F(TestStandardICQRequestManager, enqueue)
    {
        // Now, ICQRequestManager::enqueue just calls ICQRequest::perform and that's it
        MockObjects::MockICQRequestPtr rq = MockObjects::MockICQRequest::create();
        EXPECT_CALL(*rq.data(), perform());

        manager->enqueue(rq);
    }
}


