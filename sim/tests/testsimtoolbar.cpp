/*
 * testsimtoolbar.cpp
 *
 *  Created on: Aug 7, 2011
 */

#include "../commands/simtoolbar.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtest-qt.h"

#include "mocks/mockuicommand.h"
#include "commands/commandhub.h"

namespace
{
    class TestSimToolbar : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            SIM::createCommandHub();
            toolbar = new SIM::SimToolbar();
            testCmd = MockObjects::MockUiCommandPtr(new MockObjects::MockUiCommand("test", "test_cmd", QString()));
            SIM::getCommandHub()->registerCommand(testCmd);
        }

        virtual void TearDown()
        {
            delete toolbar;
            SIM::destroyCommandHub();
        }

        SIM::SimToolbar* toolbar;
        MockObjects::MockUiCommandPtr testCmd;
    };

    TEST_F(TestSimToolbar, load)
    {
        SIM::UiCommandList list;
        list.appendCommand(testCmd);
        toolbar->load(list);

        ASSERT_EQ(1, toolbar->commandsCount());
        ASSERT_EQ(testCmd->id(), toolbar->command(0)->id());
    }
}
