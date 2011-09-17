/*
 * testinterestsinfo.cpp
 *
 *  Created on: Sep 4, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "icqclient.h"
#include "events/eventhub.h"
#include "events/icqcontactupdate.h"


#include "icqconfig/interestsinfo.h"

namespace 
{
    static const int Uin = 12345678;
    static const int InterestCode[] = {100, 101, 103, 107};
    static const QString InterestName[] = {"Art", "Cars", "Collections", "Games"};
    static const QString InterestText[] = {"Interest1", "Interest2", "Interest3", "Interest4"};
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
            contact->setUin(Uin);
            for(int i = 0; i < 4; i++)
                contact->setInterest(i, InterestCode[i], InterestText[i]);

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
        ASSERT_EQ(InterestName[0], info->ui()->cmbBg1->currentText());
        ASSERT_EQ(InterestText[0], info->ui()->edtBg1->text());

        ASSERT_EQ(InterestName[1], info->ui()->cmbBg2->currentText());
        ASSERT_EQ(InterestText[1], info->ui()->edtBg2->text());

        ASSERT_EQ(InterestName[2], info->ui()->cmbBg3->currentText());
        ASSERT_EQ(InterestText[2], info->ui()->edtBg3->text());

        ASSERT_EQ(InterestName[3], info->ui()->cmbBg4->currentText());
        ASSERT_EQ(InterestText[3], info->ui()->edtBg4->text());
    }

    TEST_F(TestInterestsInfo, icq_contact_interests_info_updated_event_updates_interests)
    {
        int newInterestCode = 132;
        QString newInterestsName = "Women";
        QString newInterestsText = "New Interest";
        contact->setInterest(0, newInterestCode, newInterestsText);

        SIM::getEventHub()->triggerEvent("icq_contact_interests_info_updated",
                IcqContactUpdateData::create("icq_contact_interests_info_updated", QString::number(Uin)));

        ASSERT_EQ(newInterestsName, info->ui()->cmbBg1->currentText());
        ASSERT_EQ(newInterestsText, info->ui()->edtBg1->text());
    }
}
