
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core.h"
#include "events/ievent.h"
#include "events/eventhub.h"

#include "test.h"

namespace
{
    using namespace testing;
	class TestCorePlugin : public ::testing::Test
	{
	protected:
	    virtual void SetUp()
            {
                auto imagestorage = new NiceMock<MockObjects::MockImageStorage>();
                SIM::setImageStorage(imagestorage);
                ON_CALL(*imagestorage, icon(_)).WillByDefault(Return(QIcon()));
	    }

	    virtual void TearDown()
            {
                SIM::destroyImageStorage();
            }
	};

	TEST_F(TestCorePlugin, constructor_registersWidgetCollectionEventForContact)
	{
        CorePlugin plugin;

        SIM::IEventPtr event = SIM::getEventHub()->getEvent("contact_widget_collection");

        ASSERT_FALSE(event.isNull());
	}
}
