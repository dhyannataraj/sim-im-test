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
#include "mocks/mockuserwnd.h"

namespace
{
    using ::testing::Return;
    using ::testing::_;
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
            manager = new SutContainerManager();
            mockUserwndController = UserWndControllerPtr(new MockObjects::MockUserWndController());
        }

        virtual void TearDown()
        {
            delete manager;
            SIM::destroyOutMessagePipe();
            SIM::destroyMessagePipe();
            mockUserwndController.clear();
        }

        UserWndControllerPtr mockUserwndController;
        SutContainerManager* manager;
    };

    TEST_F(TestControllerManager, contactChatRequested_createsContainerController_ifDoesntExist)
    {
        manager->contactChatRequested(12);

        ASSERT_EQ(1, manager->containerControllersCreated);
    }

    TEST_F(TestControllerManager, contactChatRequested_raisesContainerController_ifExists)
    {
        MockObjects::MockUserWnd* userwnd = new MockObjects::MockUserWnd();
        manager->contactChatRequested(12);
        EXPECT_CALL(*manager->controller, userWndController(_)).WillRepeatedly(Return(mockUserwndController));
        EXPECT_CALL(*manager->controller, raiseUserWnd(_));

        manager->contactChatRequested(12);
    }
}


