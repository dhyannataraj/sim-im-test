/*
 * testcontainermanager.cpp
 *
 *  Created on: Aug 14, 2011
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messaging/messagepipe.h"
#include "messaging/messageoutpipe.h"

#include "container/containermanager.h"
#include "mocks/mockcontainercontroller.h"
#include "mocks/mockuserwndcontroller.h"

namespace
{
    using ::testing::Return;
    class SutContainerManager : public ContainerManager
    {
    public:
        SutContainerManager() : ContainerManager(0), containerControllersCreated(0) {}
    protected:
        virtual ContainerControllerPtr makeContainerController()
        {
            containerControllersCreated++;
            controller = new testing::NiceMock<MockObjects::MockContainerController>();
            return ContainerControllerPtr(controller);
        }
    public:
        int containerControllersCreated;
        testing::NiceMock<MockObjects::MockContainerController>* controller;
    };

    class TestControllerManager : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            SIM::createMessagePipe();
            SIM::createOutMessagePipe();
            mockUserwndController = UserWndControllerPtr(new MockObjects::MockUserWndController());
            manager = new SutContainerManager();
        }

        virtual void TearDown()
        {
            delete manager;
            SIM::destroyOutMessagePipe();
            SIM::destroyMessagePipe();
        }

        SutContainerManager* manager;
        UserWndControllerPtr mockUserwndController;
    };

    TEST_F(TestControllerManager, contactChatRequested_createsContainerController_ifDoesntExist)
    {
        manager->contactChatRequested(12);

        ASSERT_EQ(1, manager->containerControllersCreated);
    }

    TEST_F(TestControllerManager, contactChatRequested_raisesContainerController_ifExists)
    {
        EXPECT_CALL(*manager->controller, userWndController(12)).WillRepeatedly(Return(mockUserwndController));
        EXPECT_CALL(*manager->controller, raiseUserWnd(12));

        manager->contactChatRequested(12);
    }
}


