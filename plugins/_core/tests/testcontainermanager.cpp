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
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::_;

    // Outside due to bug in gtest
    MockObjects::NiceMockUserWndControllerPtr mockUserwndController;

    class SutContainerManager : public ContainerManager
    {
    public:
        SutContainerManager() : ContainerManager(0), containerControllersCreated(0) {}
    protected:
        virtual ContainerControllerPtr makeContainerController()
        {
            containerControllersCreated++;
            if(!controller)
            {
                controller = MockObjects::NiceMockContainerControllerPtr(new NiceMock<MockObjects::MockContainerController>());
            }
            ON_CALL(*controller.data(), userWndController(_)).WillByDefault(Return(mockUserwndController));
            return controller;
        }
    public:
        void setController(const MockObjects::NiceMockContainerControllerPtr& c)
        {
            controller = c;
        }

        int containerControllersCreated;
        MockObjects::NiceMockContainerControllerPtr controller;
    };

    class TestControllerManager : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            manager = new SutContainerManager();
            mockUserwndController = MockObjects::NiceMockUserWndControllerPtr(new NiceMock<MockObjects::MockUserWndController>());
        }

        virtual void TearDown()
        {
            delete manager;
            mockUserwndController.clear();
        }

        SutContainerManager* manager;
    };

    TEST_F(TestControllerManager, contactChatRequested_createsContainerController_ifDoesntExist)
    {
        manager->contactChatRequested(12, "generic");

        ASSERT_EQ(1, manager->containerControllersCreated);
    }

    TEST_F(TestControllerManager, contactChatRequested_raisesContainerController_ifExists)
    {
        manager->contactChatRequested(12, "generic");
        EXPECT_CALL(*manager->controller, raiseUserWnd(_));

        manager->contactChatRequested(12, "generic");
    }

    TEST_F(TestControllerManager, contactChatRequested_addsUserWndToController)
    {
        MockObjects::NiceMockContainerControllerPtr controller =
                MockObjects::NiceMockContainerControllerPtr(new MockObjects::NiceMockContainerController());
        manager->setController(controller);
        EXPECT_CALL(*controller.data(), addUserWnd(12));

        manager->contactChatRequested(12, "generic");

    }

    TEST_F(TestControllerManager, contactChatRequested_setsMessageType)
    {
        MockObjects::NiceMockContainerControllerPtr controller =
                MockObjects::NiceMockContainerControllerPtr(new MockObjects::NiceMockContainerController());
        manager->setController(controller);

        EXPECT_CALL(*mockUserwndController.data(), setMessageType(QString("generic")));

        manager->contactChatRequested(12, "generic");
    }
}


