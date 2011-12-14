
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "icqclient.h"
#include "icqstatuswidget.h"
#include "contacts/imstatus.h"
#include "testicqclient.h"
#include "tests/mocks/mockoscarsocket.h"
#include "events/eventhub.h"
#include "events/ievent.h"

namespace
{
    using namespace SIM;
    using namespace MockObjects;
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::InSequence;
    using ::testing::AnyNumber;
    class TestIcqClient : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            client = new ICQClient(0, "ICQ.123456", false);
        }

        virtual void TearDown()
        {
            delete client;
        }

        void forceAllSnacsReady()
        {
            LocationSnacHandler* location = static_cast<LocationSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_LOCATION));
            location->forceReady();

            BuddySnacHandler* buddy = static_cast<BuddySnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_BUDDY));
            buddy->forceReady();

            IcbmSnacHandler* icbm = static_cast<IcbmSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_MESSAGE));
            icbm->forceReady();

            PrivacySnacHandler* privacy = static_cast<PrivacySnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_BOS));
            privacy->forceReady();

            SsiSnacHandler* ssi = static_cast<SsiSnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_LISTS));
            ssi->forceReady();
        }

        ICQClient* client;
    };

    TEST_F(TestIcqClient, getDefaultStatus_offline)
    {
        IMStatusPtr status = client->getDefaultStatus("offline");

        ASSERT_FALSE(status.isNull());
        ASSERT_TRUE(status->flag(IMStatus::flOffline));
    }

    TEST_F(TestIcqClient, getDefaultStatus_online)
    {
        IMStatusPtr status = client->getDefaultStatus("online");

        ASSERT_FALSE(status.isNull());
        ASSERT_FALSE(status->flag(IMStatus::flOffline));
    }

    TEST_F(TestIcqClient, onCreation_statusIsOffline)
    {
        IMStatusPtr status = client->currentStatus();

        ASSERT_FALSE(status.isNull());
        ASSERT_TRUE(status->flag(IMStatus::flOffline));
    }

    TEST_F(TestIcqClient, changeStatus_fromOfflineToOnline_causesStatusWidgetBlink)
    {
        StatusWidget* widget = qobject_cast<StatusWidget*>(client->createStatusWidget());
        ASSERT_TRUE(widget);

        client->changeStatus(client->getDefaultStatus("online"));
        ASSERT_TRUE(widget->isBlinking());
    }

    TEST_F(TestIcqClient, changeStatus_fromOfflineToOnline)
    {
        MockOscarSocket* oscarSocket = new MockObjects::MockOscarSocket();
        client->setOscarSocket(oscarSocket);
        EXPECT_CALL(*oscarSocket, connectToHost(_, _));

        client->changeStatus(client->getDefaultStatus("online"));
    }

    TEST_F(TestIcqClient, allSnacsReady_sendsSetStatus_sendsReady_requestsSerivces)
    {
        NiceMock<MockOscarSocket>* oscarSocket = new NiceMock<MockObjects::MockOscarSocket>();
        client->setOscarSocket(oscarSocket);

        {
            EXPECT_CALL(*oscarSocket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceSetStatus, _, _));
            EXPECT_CALL(*oscarSocket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceReady, _, _));
            EXPECT_CALL(*oscarSocket, snac(ICQ_SNACxFOOD_SERVICE, ServiceSnacHandler::SnacServiceRequestService, _, _)).Times(AnyNumber());
        }

        forceAllSnacsReady();
    }

    TEST_F(TestIcqClient, registersIcqBasicInfoRequestEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_basic_info_updated");

        ASSERT_TRUE(event);
    }

    TEST_F(TestIcqClient, registersIcqWorkInfoRequestEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_work_info_updated");

        ASSERT_TRUE(event);
    }

    TEST_F(TestIcqClient, registersIcqMoreInfoRequestEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_more_info_updated");

        ASSERT_TRUE(event);
    }

    TEST_F(TestIcqClient, registersIcqAboutInfoRequestEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_about_info_updated");

        ASSERT_TRUE(event);
    }

    TEST_F(TestIcqClient, registersIcqInterestsInfoRequestEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_interests_info_updated");

        ASSERT_TRUE(event);
    }

    TEST_F(TestIcqClient, registersIcqPastInfoRequestEvent)
    {
        SIM::IEventPtr event = SIM::getEventHub()->getEvent("icq_contact_past_info_updated");

        ASSERT_TRUE(event);
    }

    TEST_F(TestIcqClient, loadStateFromEmptyPropertyHub)
    {
        PropertyHubPtr testHub;

        ASSERT_FALSE(client->loadState(testHub));
    }

}
