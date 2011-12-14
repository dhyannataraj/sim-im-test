
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "qt-gtest.h"

#include "icqcontact.h"
#include "icqclient.h"

namespace
{
    class TestIcqContact : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            m_client = new ICQClient(0, "ICQ.123456", false);
        }

        virtual void TearDown()
        {
            delete m_client;
        }

        ICQClient* m_client;
    };

    TEST_F(TestIcqContact, Creation)
    {
        ICQContact contact(m_client);

        ASSERT_TRUE(contact.status());
        ASSERT_EQ(contact.status()->id(), "offline");
    }

    TEST_F(TestIcqContact, setUin_setScreenName)
    {
        ICQContact contact(m_client);

        contact.setUin(123456);
        ASSERT_EQ("123456", contact.getScreen());
    }

    TEST_F(TestIcqContact, loadStateFromEmptyPropertyHub)
    {
        SIM::PropertyHubPtr testHub;
        ICQContact contact(m_client);

        ASSERT_FALSE(contact.loadState(testHub));
    }
}
