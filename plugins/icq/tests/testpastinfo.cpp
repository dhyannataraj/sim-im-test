/*
 * testpastinfo.cpp
 *
 *  Created on: Sep 12, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "icqconfig/pastinfo.h"
#include "icqclient.h"
#include "events/eventhub.h"
#include "events/icqcontactupdate.h"

namespace
{
    static const int Uin = 12345678;
    static const int AffiliationCode[] = {200, 201, 202};
    static const QString AffiliationName[] = {"Alumni Org.", "Charity Org.", "Club/Social Org."};
    static const QString AffiliationText[] = {"Test affiliation1", "Test affiliation2", "Test affiliation3" };

    static const int BackgroundCode[] = {300, 301, 302};
    static const QString BackgroundName[] = { "Elementary School", "High School", "College" };
    static const QString BackgroundText[] = { "Test background1", "Test background2", "Test background3" };

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
            contact->setUin(Uin);

            for(int i = 0; i < 3; i++)
                contact->setAffiliation(i, AffiliationCode[i], AffiliationText[i]);

            for(int i = 0; i < 3; i++)
                contact->setBackground(i, BackgroundCode[i], BackgroundText[i]);

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
        ASSERT_EQ(AffiliationName[0], info->ui()->cmbAf1->currentText());
        ASSERT_EQ(AffiliationText[0], info->ui()->edtAf1->text());

        ASSERT_EQ(AffiliationName[1], info->ui()->cmbAf2->currentText());
        ASSERT_EQ(AffiliationText[1], info->ui()->edtAf2->text());

        ASSERT_EQ(AffiliationName[2], info->ui()->cmbAf3->currentText());
        ASSERT_EQ(AffiliationText[2], info->ui()->edtAf3->text());
    }

    TEST_F(TestPastInfo, constructor_setsBackgrounds)
    {
        ASSERT_EQ(BackgroundName[0], info->ui()->cmbBg1->currentText());
        ASSERT_EQ(BackgroundText[0], info->ui()->edtBg1->text());

        ASSERT_EQ(BackgroundName[1], info->ui()->cmbBg2->currentText());
        ASSERT_EQ(BackgroundText[1], info->ui()->edtBg2->text());

        ASSERT_EQ(BackgroundName[2], info->ui()->cmbBg3->currentText());
        ASSERT_EQ(BackgroundText[2], info->ui()->edtBg3->text());
    }

    TEST_F(TestPastInfo, icq_contact_past_info_updated_event_updates_affiliations)
    {
        int newAffiliationCode = 214;
        QString newAffiliationName = "Sports Org.";
        QString newAffiliationText = "New Affiliation";
        contact->setAffiliation(0, newAffiliationCode, newAffiliationText);

        SIM::getEventHub()->triggerEvent("icq_contact_past_info_updated",
                IcqContactUpdateData::create("icq_contact_past_info_updated", QString::number(Uin)));

        ASSERT_EQ(newAffiliationName, info->ui()->cmbAf1->currentText());
        ASSERT_EQ(newAffiliationText, info->ui()->edtAf1->text());
    }

    TEST_F(TestPastInfo, icq_contact_past_info_updated_event_updates_background)
    {
        int newBackgroundCode = 306;
        QString newBackgroundName = "Past Organization";
        QString newBackgroundText = "New Background";
        contact->setBackground(0, newBackgroundCode, newBackgroundText);

        SIM::getEventHub()->triggerEvent("icq_contact_past_info_updated",
                IcqContactUpdateData::create("icq_contact_past_info_updated", QString::number(Uin)));

        ASSERT_EQ(newBackgroundName, info->ui()->cmbBg1->currentText());
        ASSERT_EQ(newBackgroundText, info->ui()->edtBg1->text());
    }
}
