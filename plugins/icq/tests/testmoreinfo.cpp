/*
 * testmoreinfo.cpp
 *
 *  Created on: Sep 4, 2011
 */

#include <QDate>

#include "icqconfig/moreinfo.h"
#include "icqclient.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "events/eventhub.h"
#include "events/icqcontactupdate.h"

#include "icqcontact.h"

namespace 
{
    using testing::Test;

    static const int Uin = 12345678;
    static const QDate Birthday(2000, 01, 01);
    static const QString Homepage = "Test Homepage";
    static const int GenderCode = 1;
    static const QString GenderName = "Female";
    static const int PrimaryLanguageCode = 1;
    static const QString PrimaryLanguageName = "Arabic";
    static const int SecondaryLanguageCode = 2;
    static const QString SecondaryLanguageName = "Bhojpuri";
    static const int TertiaryLanguageCode = 3;
    static const QString TertiaryLanguageName = "Bulgarian";

    class SutMoreInfo : public MoreInfo
    {
    public:
        SutMoreInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) : MoreInfo(parent, contact, client) {}

        Ui::MoreInfo* ui() const { return MoreInfo::ui(); }

    protected:
        virtual QDate currentDate() const
        {
            return QDate(2010, 01, 01);
        }
    };

    class TestMoreInfo : public Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "123456", false);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setUin(Uin);
            contact->setBirthday(Birthday);
            contact->setHomepage(Homepage);
            contact->setGender(GenderCode);
            contact->setPrimaryLanguage(PrimaryLanguageCode);
            contact->setSecondaryLanguage(SecondaryLanguageCode);
            contact->setTertiaryLanguage(TertiaryLanguageCode);
            contact->setAge(11);

            info = new SutMoreInfo(0, contact, client);
        }

        virtual void TearDown()
        {
            delete info;
            delete client;
        }

        ICQClient* client;
        ICQContactPtr contact;
        SutMoreInfo* info;
    };

    TEST_F(TestMoreInfo, constructor_setsAge)
    {
        ASSERT_EQ(11, info->ui()->spnAge->value());
    }

    TEST_F(TestMoreInfo, constructor_setsHomepage)
    {
        ASSERT_EQ(Homepage, info->ui()->edtHomePage->text());
    }

    TEST_F(TestMoreInfo, constructor_setsGender)
    {
        ASSERT_EQ(GenderName, info->ui()->cmbGender->currentText());
    }

    TEST_F(TestMoreInfo, constructor_setsPrimaryLanguage)
    {
        ASSERT_EQ(PrimaryLanguageName, info->ui()->cmbLang1->currentText());
    }

    TEST_F(TestMoreInfo, constructor_setsSecondaryLanguage)
    {
        ASSERT_EQ(SecondaryLanguageName, info->ui()->cmbLang2->currentText());
    }

    TEST_F(TestMoreInfo, constructor_setsTertiaryLanguage)
    {
        ASSERT_EQ(TertiaryLanguageName, info->ui()->cmbLang3->currentText());
    }

    TEST_F(TestMoreInfo, icq_contact_more_info_updated_event_updates_age)
    {
        int newAge = 42;
        contact->setAge(newAge);

        SIM::getEventHub()->triggerEvent("icq_contact_more_info_updated",
                IcqContactUpdateData::create("icq_contact_more_info_updated", QString::number(Uin)));

        ASSERT_EQ(newAge, info->ui()->spnAge->value());
    }

    TEST_F(TestMoreInfo, icq_contact_more_info_updated_event_updates_birthday)
    {
        QDate newBirthday = QDate(1990, 1, 12);
        contact->setBirthday(newBirthday);

        SIM::getEventHub()->triggerEvent("icq_contact_more_info_updated",
                IcqContactUpdateData::create("icq_contact_more_info_updated", QString::number(Uin)));

        ASSERT_EQ(newBirthday, info->ui()->edtDate->date());
    }

    TEST_F(TestMoreInfo, icq_contact_more_info_updated_event_updates_gender)
    {
        int newGender = 2;
        contact->setGender(newGender);

        SIM::getEventHub()->triggerEvent("icq_contact_more_info_updated",
                IcqContactUpdateData::create("icq_contact_more_info_updated", QString::number(Uin)));

        ASSERT_EQ("Male", info->ui()->cmbGender->currentText());
    }
}
