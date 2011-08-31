/*
 * testuserconfigcontext.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "userconfig/userconfigcontext.h"

namespace
{
    static int ContactId = 12;
    using namespace ::testing;

    class TestUserConfigContext : public Test
    {
    public:
        virtual void SetUp()
        {

        }

        virtual void TearDown()
        {

        }
    };

    TEST_F(TestUserConfigContext, ContactContext_widgetCollectionEventId)
    {
        SIM::ContactPtr contact = SIM::ContactPtr(new SIM::Contact(ContactId));
        UserConfigContextPtr context = UserConfigContext::create(contact);

        ASSERT_EQ("contact_widget_collection", context->widgetCollectionEventId());
    }

    TEST_F(TestUserConfigContext, ContactContext_context)
    {
        SIM::ContactPtr contact = SIM::ContactPtr(new SIM::Contact(ContactId));
        UserConfigContextPtr context = UserConfigContext::create(contact);

        ASSERT_EQ(QString::number(ContactId), context->context());
    }
}

