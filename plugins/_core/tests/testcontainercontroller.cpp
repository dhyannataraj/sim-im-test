
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

#include "core.h"
#include "test.h"

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
        SIM::MessagePipe* oldpipe;

        virtual void SetUp()
        {
            auto imagestorage = new testing::NiceMock<MockObjects::MockImageStorage>();
            SIM::setImageStorage(imagestorage);
            ON_CALL(*imagestorage, icon(_)).WillByDefault(Return(QIcon()));
            core = new CorePlugin();

            oldpipe = SIM::getOutMessagePipe();
            pipe = new MockObjects::MockMessagePipe();
            SIM::setOutMessagePipe(pipe);

            SIM::createContactList();

            controller = new ContainerController(ControllerId);
        }

        virtual void TearDown()
        {
            delete controller;

            SIM::destroyContactList();
            SIM::destroyImageStorage();

            delete pipe;
            SIM::setOutMessagePipe(oldpipe);

            delete core;
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
        CorePlugin* core;
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
