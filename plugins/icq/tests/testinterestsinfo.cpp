/*
 * testinterestsinfo.cpp
 *
 *  Created on: Sep 4, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "icqclient.h"

#include "icqconfig/interestsinfo.h"

namespace 
{
    static const int InterestCode = 100;
    static const QString InterestName = "Art";
    static const QString InterestText = "Interest";
    class SutInterestsInfo : public InterestsInfo
    {
    public:
        SutInterestsInfo(QWidget *parent, const ICQContactPtr& contact, ICQClient* client) : InterestsInfo(parent, contact, client) {}
        Ui::InterestsInfoBase* ui() const { return InterestsInfo::ui(); }
    };
    using testing::Test;
    class TestInterestsInfo : public Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "123456", false);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setInterest(0, InterestCode, InterestText);

            info = new SutInterestsInfo(0, contact, client);
        }

        virtual void TearDown()
        {
            delete info;
            delete client;
        }

        ICQClient* client;
        ICQContactPtr contact;
        SutInterestsInfo* info;
    };

    TEST_F(TestInterestsInfo, constructor_setsInterests)
    {
        ASSERT_EQ(InterestName, info->ui()->cmbBg1->currentText());
        ASSERT_EQ(InterestText, info->ui()->edtBg1->text());
    }
}
