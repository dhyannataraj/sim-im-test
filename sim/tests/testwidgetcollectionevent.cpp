/*
 * testwidgetcollectionevent.cpp
 *
 *  Created on: Aug 29, 2011
 */

#include "../events/widgetcollectionevent.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace
{
    using namespace SIM;
    class TestWidgetCollectionEvent : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {

        }

        virtual void TearDown()
        {

        }
    };

    TEST_F(TestWidgetCollectionEvent, id)
    {
        WidgetCollectionEvent ev;

        ASSERT_EQ("widget_collection", ev.id());
    }
}
