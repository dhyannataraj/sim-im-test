/*
 * testuserconfigcontroller.cpp
 *
 *  Created on: Aug 29, 2011
 */

#include "userconfig/userconfigcontroller.h"

#include "tests/testwidgetcollectionevent.h"
#include "tests/spies/widgetcollectioneventreceiver.h"
#include "mocks/mockuserconfigview.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "events/eventhub.h"
#include "events/widgetcollectionevent.h"
#include "userconfig/userconfigcontext.h"

#include "core.h"
#include "test.h"

#include <QLabel>

namespace
{
    using namespace ::testing;

    MockObjects::MockUserConfigViewPtr userConfigView;

    class SutUserConfigController : public ::UserConfigController
    {
    public:
        SutUserConfigController() : ::UserConfigController() {}

    protected:
        virtual UserConfigPtr createUserConfigView()
        {
            return userConfigView;
        }
    };

    class TestUserConfigController : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            auto imagestorage = getMockImageStorage();
            ON_CALL(*imagestorage, icon(_)).WillByDefault(Return(QIcon()));
            core = new CorePlugin();
            userConfigView = MockObjects::MockUserConfigView::create();
        }

        virtual void TearDown()
        {
            userConfigView.clear();
            delete core;
        }

        CorePlugin* core;
    };

    TEST_F(TestUserConfigController, onCreation_emitsWidgetCollectionEvent)
    {
        TestHelper::WidgetCollectionEventReceiver receiver("");
        UserConfigContextPtr context = UserConfigContext::create(SIM::ContactPtr(new SIM::Contact(12)));
        SIM::getEventHub()->getEvent(context->widgetCollectionEventId())->connectTo(&receiver, SLOT(eventReceived(SIM::WidgetHierarchy*,QString)));

        SutUserConfigController controller;
        controller.init(context);

        ASSERT_EQ(1, receiver.receviedEventCount());
    }

    TEST_F(TestUserConfigController, onCreation_setsCollectedHierarchy)
    {
        // Setup
        TestHelper::WidgetCollectionEventReceiver receiver("");
        UserConfigContextPtr context = UserConfigContext::create(SIM::ContactPtr(new SIM::Contact(12)));
        SIM::getEventHub()->getEvent(context->widgetCollectionEventId())->connectTo(&receiver, SLOT(eventReceived(SIM::WidgetHierarchy*,QString)));

        // Expectations
        EXPECT_CALL(*userConfigView.data(), setWidgetHierarchy(_)).Times(1);

        // Exercise
        SutUserConfigController controller;
        controller.init(context);
    }
}
