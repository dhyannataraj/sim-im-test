/*
 * testicqmessagefactory.cpp
 *
 *  Created on: Aug 16, 2011
 */

#include "../icqmessagefactory.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace
{
    class TestIcqMessageFactory : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            factory = new ICQMessageFactory();
        }

        virtual void TearDown()
        {
            delete factory;
        }

        ICQMessageFactory* factory;
    };

    TEST_F(TestIcqMessageFactory, createMessage_generic)
    {
        SIM::MessagePtr message = factory->createMessage("generic");

        ASSERT_FALSE(message.isNull());
    }
}
