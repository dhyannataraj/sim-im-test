/*
 * testwidgetcollectionevent.cpp
 *
 *  Created on: Aug 29, 2011
 */

#include "../events/widgetcollectionevent.h"

#include "testwidgetcollectionevent.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "events/eventhub.h"

#include "widgethierarchy.h"
#include "spies/widgetcollectioneventreceiver.h"

static const char* EventId = "widget_collection_test";
static const char* WidgetName = "TestWidget";
static const char* ContextId = "TestContext";

namespace
{
    using namespace SIM;
    class TestWidgetCollectionEvent : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            SIM::createEventHub();
            SIM::getEventHub()->registerEvent(WidgetCollectionEvent::create(EventId));
        }

        virtual void TearDown()
        {
            SIM::destroyEventHub();
        }
    };

    TEST_F(TestWidgetCollectionEvent, id)
    {
        WidgetCollectionEvent ev("widget_collection_test");

        ASSERT_EQ("widget_collection_test", ev.id());
    }

    TEST_F(TestWidgetCollectionEvent, create)
    {
        IEventPtr ev = WidgetCollectionEvent::create(EventId);

        ASSERT_EQ(EventId, ev->id());
    }

    TEST_F(TestWidgetCollectionEvent, connectTo)
    {
        TestHelper::WidgetCollectionEventReceiver receiver(WidgetName);
        WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(EventId);

        bool rc = SIM::getEventHub()->getEvent(EventId)->connectTo(&receiver, SLOT(eventReceived(SIM::WidgetHierarchy*, QString)));

        ASSERT_TRUE(rc);
    }

    TEST_F(TestWidgetCollectionEvent, triggered)
    {
        TestHelper::WidgetCollectionEventReceiver receiver(WidgetName);
        WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(EventId);
        SIM::getEventHub()->getEvent(EventId)->connectTo(&receiver, SLOT(eventReceived(SIM::WidgetHierarchy*, QString)));

        SIM::getEventHub()->triggerEvent(EventId, data);

        ASSERT_EQ(1, receiver.receviedEventCount());
    }

    TEST_F(TestWidgetCollectionEvent, triggered_context)
    {
        TestHelper::WidgetCollectionEventReceiver receiver(WidgetName);
        WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(EventId, ContextId);
        SIM::getEventHub()->getEvent(EventId)->connectTo(&receiver, SLOT(eventReceived(SIM::WidgetHierarchy*, QString)));

        SIM::getEventHub()->triggerEvent(EventId, data);

        ASSERT_EQ(ContextId, receiver.lastContext());
    }

    TEST_F(TestWidgetCollectionEvent, widgetCollectionEventData_eventId)
    {
        WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(EventId);

        ASSERT_EQ(EventId, data->eventId());
    }

    TEST_F(TestWidgetCollectionEvent, widgetCollectionEventData_hierarchyRoot)
    {
        WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(EventId);

        ASSERT_TRUE(data->hierarchyRoot() != nullptr);
    }

    TEST_F(TestWidgetCollectionEvent, hierarchyCollection)
    {
        TestHelper::WidgetCollectionEventReceiver receiver(WidgetName);
        WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(EventId);
        SIM::getEventHub()->getEvent(EventId)->connectTo(&receiver, SLOT(eventReceived(SIM::WidgetHierarchy*, QString)));

        SIM::getEventHub()->triggerEvent(EventId, data);

        SIM::WidgetHierarchy* root = data->hierarchyRoot();

        ASSERT_TRUE(root != nullptr);
        ASSERT_EQ(1, root->children.size());
    }
}
