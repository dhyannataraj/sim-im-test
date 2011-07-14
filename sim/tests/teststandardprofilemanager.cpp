/*
 * teststandardprofilemanager.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include "standardprofilemanager.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace SIM
{
    class TestStandardProfileManager : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
        }

        virtual void TearDown()
        {
        }
    };

    TEST_F(TestStandardProfileManager, fakeTest)
    {
        ASSERT_TRUE(1 == 1);
    }

} /* namespace SIM */
