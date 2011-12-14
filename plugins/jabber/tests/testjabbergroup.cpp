
#include "gtest/gtest.h"

#include "jabberclient.h"
#include "jabbergroup.h"

namespace
{
    class TestJabberGroup : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            m_client = new JabberClient(0, "Jabber.123456");
        }

        virtual void TearDown()
        {
            delete m_client;
        }

        JabberClient* m_client;
    };

    TEST_F(TestJabberGroup, loadStateFromEmptyPropertyHub)
    {
        SIM::PropertyHubPtr testHub;
        JabberGroup group(m_client);

        ASSERT_FALSE(group.loadState(testHub));
    }

}
