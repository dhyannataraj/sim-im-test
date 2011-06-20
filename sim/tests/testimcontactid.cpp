
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tests/gtest-qt.h"

#include "contacts/imcontactid.h"

namespace
{
    using namespace SIM;
    class TestIMContactId : public ::testing::Test
    {
    public:
    };

    TEST_F(TestIMContactId, ConstructionWithoutParameters_isValid_false)
    {
        IMContactId contact;

        ASSERT_FALSE(contact.isValid());
    }

    TEST_F(TestIMContactId, ConstructionFromString_isValid_true)
    {
        IMContactId contact("client/contactname.subcontact#1", 0);

        ASSERT_TRUE(contact.isValid());
    }

    TEST_F(TestIMContactId, ConstructionFromStringWithoutSlash_isValid_false)
    {
        IMContactId contact("invalid_id_without_slash", 0);

        ASSERT_FALSE(contact.isValid());
    }

    TEST_F(TestIMContactId, accessors)
    {
        IMContactId contact("client/contactname", 12);

        ASSERT_EQ("client", contact.clientId());
        ASSERT_EQ("contactname", contact.clientSpecificId());
        ASSERT_EQ(12, contact.parentContactId());
    }
}
