/*
 * testpastinfo.cpp
 *
 *  Created on: Sep 12, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "icqconfig/pastinfo.h"
#include "icqclient.h"

namespace 
{
static const int AffiliationCode = 200;
    static const QString AffiliationName = "Alumni Org.";
    static const QString AffiliationText = "Test affiliation";

    static const int BackgroundCode = 300;
    static const QString BackgroundName = "Elementary School";
    static const QString BackgroundText = "Test background";

    class SutPastInfo : public PastInfo
    {
    public:
        SutPastInfo(QWidget *parent, const ICQContactPtr& contact, ICQClient* client) : PastInfo(parent, contact, client) {}
        Ui::PastInfoBase* ui() const { return PastInfo::ui(); }
    };

    using testing::Test;
    class TestPastInfo : public Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "123456", false);
            contact = ICQContactPtr(new ICQContact(client));

            contact->setAffiliation(0, AffiliationCode, AffiliationText);
            contact->setBackground(0, BackgroundCode, BackgroundText);

            info = new SutPastInfo(0, contact, client);
        }

        virtual void TearDown()
        {
            delete info;
            delete client;
        }

        ICQClient* client;
        ICQContactPtr contact;
        SutPastInfo* info;
    };

    TEST_F(TestPastInfo, constructor_setsAffiliations)
    {
        ASSERT_EQ(AffiliationName, info->ui()->cmbAf1->currentText());
        ASSERT_EQ(AffiliationText, info->ui()->edtAf1->text());
    }

    TEST_F(TestPastInfo, constructor_setsBackgrounds)
    {
        ASSERT_EQ(BackgroundName, info->ui()->cmbBg1->currentText());
        ASSERT_EQ(BackgroundText, info->ui()->edtBg1->text());
    }
}
