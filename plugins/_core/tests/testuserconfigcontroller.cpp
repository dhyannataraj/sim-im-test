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

    MockObjects::NiceMockUserConfigViewPtr userConfigView;

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
            auto imagestorage = new testing::NiceMock<MockObjects::MockImageStorage>();
            SIM::setImageStorage(imagestorage);
            ON_CALL(*imagestorage, icon(_)).WillByDefault(Return(QIcon()));
            core = new CorePlugin();
            userConfigView = MockObjects::MockUserConfigView::createNice();
        }

        virtual void TearDown()
        {
            userConfigView.clear();
            SIM::destroyImageStorage();
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

    TEST_F(TestUserConfigController, delegates_exec)
    {
        EXPECT_CALL(*userConfigView.data(), setWidgetHierarchy(_)).Times(1);
        UserConfigContextPtr context = UserConfigContext::create(SIM::ContactPtr(new SIM::Contact(12)));
        SutUserConfigController controller;
        controller.init(context);

        controller.exec();
    }
}
