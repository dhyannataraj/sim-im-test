
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "container/containercontroller.h"
#include "tests/mocks/mockmessagepipe.h"
#include "tests/stubs/stubmessage.h"
#include "messaging/messagepipe.h"
#include "mocks/mockuserwndcontroller.h"
#include "tests/mocks/mockmessage.h"
#include "container/userwnd.h"
#include "container/userwndcontroller.h"
#include "tests/mocks/mockimcontact.h"

#include "contacts/contactlist.h"

namespace
{
    using ::testing::Return;
    using ::testing::_;

    static const int ControllerId = 12;
    static const int ContactId = 23;

    class ContainerController : public ::ContainerController
    {
    public:
         ContainerController(int id) : ::ContainerController(id) {}
         virtual ~ContainerController() {}

         MockObjects::MockUserWndControllerPtr lastCreatedUserWnd()
         {
             return m_lastUserWnd;
         }
    protected:
        virtual UserWndControllerPtr createUserWndController(int)
        {
            m_lastUserWnd = MockObjects::MockUserWndControllerPtr(new MockObjects::MockUserWndController());
            ON_CALL(*m_lastUserWnd.data(), id()).WillByDefault(Return(ContactId));
            return m_lastUserWnd;
        }

    private:
        MockObjects::MockUserWndControllerPtr m_lastUserWnd;
    };

    class TestContainerController : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            pipe = new MockObjects::MockMessagePipe();
            SIM::setOutMessagePipe(pipe);

            SIM::createContactList();

            controller = new ContainerController(ControllerId);
        }

        virtual void TearDown()
        {
            delete controller;

            SIM::destroyContactList();

            delete pipe;
            SIM::setOutMessagePipe(0);
        }

        void createContact()
        {
            SIM::ContactPtr contact = SIM::getContactList()->createContact(ContactId);
            imcontact = MockObjects::MockIMContact::create();
            ON_CALL(*imcontact.data(), id()).WillByDefault(Return(SIM::IMContactId("client/contact", ContactId)));
        }

        MockObjects::MockIMContactPtr imcontact;
        ContainerController* controller;
        MockObjects::MockMessagePipe* pipe;
    };

    TEST_F(TestContainerController, id_returnsCorrectId)
    {
        ASSERT_EQ(ControllerId, controller->id());
    }

    TEST_F(TestContainerController, sendMessage_pushesMessageToPipe)
    {
        SIM::MessagePtr msg = SIM::MessagePtr(new StubObjects::StubMessage());
        EXPECT_CALL(*pipe, pushMessage(msg)).Times(1);
        controller->sendMessage(msg);
    }
}
