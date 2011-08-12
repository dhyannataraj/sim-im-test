
#include <QSignalSpy>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "contacts/client.h"
#include "tests/stubs/stubclient.h"
#include "clientmanager.h"
#include "contacts/protocol.h"
#include "tests/mocks/mockclientmanager.h"
#include "commonstatus.h"
#include "tests/mocks/mockclient.h"
#include "events/eventhub.h"

namespace
{
    class TestCommonStatus : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            clientmanager = new MockObjects::MockClientManager();
            SIM::setClientManager(clientmanager);
            commonStatus = new CommonStatus(clientmanager);
        }

        virtual void TearDown()
        {
            SIM::destroyClientManager();
        }

        CommonStatus* commonStatus;
        MockObjects::MockClientManager* clientmanager;
    };

    TEST_F(TestCommonStatus, hasStatus_online)
    {
        int index = commonStatus->indexOfCommonStatus("online");

        ASSERT_NE(-1, index);
    }

    TEST_F(TestCommonStatus, hasStatus_away)
    {
        int index = commonStatus->indexOfCommonStatus("away");

        ASSERT_NE(-1, index);
    }

    TEST_F(TestCommonStatus, hasStatus_na)
    {
        int index = commonStatus->indexOfCommonStatus("na");

        ASSERT_NE(-1, index);
    }

    TEST_F(TestCommonStatus, hasStatus_dnd)
    {
        int index = commonStatus->indexOfCommonStatus("dnd");

        ASSERT_NE(-1, index);
    }

    TEST_F(TestCommonStatus, hasStatus_offline)
    {
        int index = commonStatus->indexOfCommonStatus("offline");

        ASSERT_NE(-1, index);
    }

    TEST_F(TestCommonStatus, constructor_registersCommonStatusEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("common_status");

        ASSERT_FALSE(event.isNull());
    }

    TEST_F(TestCommonStatus, changeStatus_emitsEvent)
    {
        QSignalSpy spy(SIM::getEventHub()->getEvent("common_status").data(), SIGNAL(eventTriggered(QString)));

        commonStatus->setCommonStatus("online");

        ASSERT_EQ(1, spy.count());
    }

    TEST_F(TestCommonStatus, changeStatus_doesntEmitEvent_ifInvalidStatusId)
    {
        QSignalSpy spy(SIM::getEventHub()->getEvent("common_status").data(), SIGNAL(eventTriggered(QString)));

        commonStatus->setCommonStatus("invalid_status_id");

        ASSERT_EQ(0, spy.count());
    }
}
