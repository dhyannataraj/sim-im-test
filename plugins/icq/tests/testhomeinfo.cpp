/*
 * testhomeinfo.cpp
 *
 *  Created on: Sep 4, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "icqconfig/homeinfo.h"
#include "events/icqcontactupdate.h"
#include "events/eventhub.h"

#include "icqclient.h"
#include "icqcontact.h"

namespace 
{
    static const unsigned long Uin = 123456789;
    static const QString Address = "Test address";
    static const QString City = "Test city";
    static const QString State = "Test state";
    static const QString Zip = "Test zip";
    static const int Country = 263;
    static const QString CountryName = "Zimbabwe";
    static const int Zone = 6;
    static const QString ZoneName = "+6";

    class SutHomeInfo : public HomeInfo
    {
    public:
        SutHomeInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) : HomeInfo(parent, contact, client) {}
        Ui::HomeInfo* ui() { return HomeInfo::ui(); }
    };

    using namespace testing;
    class TestHomeInfo : public Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "123456", false);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setUin(Uin);
            contact->setAddress(Address);
            contact->setCity(City);
            contact->setState(State);
            contact->setZip(Zip);
            contact->setCountry(Country);
            contact->setTimeZone(Zone);

            info = new SutHomeInfo(0, contact, client);
        }

        virtual void TearDown()
        {
            delete info;
            delete client;
        }

        ICQClient* client;
        ICQContactPtr contact;
        SutHomeInfo* info;
    };

    TEST_F(TestHomeInfo, construction_InitializesAddress)
    {
        ASSERT_EQ(Address, info->ui()->edtAddress->toPlainText());
    }

    TEST_F(TestHomeInfo, construction_InitializesCity)
    {
        ASSERT_EQ(City, info->ui()->edtCity->text());
    }

    TEST_F(TestHomeInfo, construction_InitializesState)
    {
        ASSERT_EQ(State, info->ui()->edtState->text());
    }

    TEST_F(TestHomeInfo, construction_InitializesZip)
    {
        ASSERT_EQ(Zip, info->ui()->edtZip->text());
    }

    TEST_F(TestHomeInfo, construction_InitializesCountry)
    {
        ASSERT_EQ(CountryName, info->ui()->cmbCountry->currentText());
    }

    TEST_F(TestHomeInfo, construction_InitializesTimezone)
    {
        ASSERT_EQ(ZoneName, info->ui()->cmbZone->currentText());
    }

    TEST_F(TestHomeInfo, icq_contact_basic_info_updated_event_updates_address)
    {
        QString newAddress = "New address";
        contact->setAddress(newAddress);

        SIM::getEventHub()->triggerEvent("icq_contact_basic_info_updated",
                IcqContactUpdateData::create("icq_contact_basic_info_updated", QString::number(Uin)));

        ASSERT_EQ(newAddress, info->ui()->edtAddress->toPlainText());
    }

    TEST_F(TestHomeInfo, icq_contact_basic_info_updated_event_updates_city)
    {
        QString newCity = "New city";
        contact->setCity(newCity);

        SIM::getEventHub()->triggerEvent("icq_contact_basic_info_updated",
                IcqContactUpdateData::create("icq_contact_basic_info_updated", QString::number(Uin)));

        ASSERT_EQ(newCity, info->ui()->edtCity->text());
    }

    TEST_F(TestHomeInfo, icq_contact_basic_info_updated_event_updates_state)
    {
        QString newState = "New state";
        contact->setState(newState);

        SIM::getEventHub()->triggerEvent("icq_contact_basic_info_updated",
                IcqContactUpdateData::create("icq_contact_basic_info_updated", QString::number(Uin)));

        ASSERT_EQ(newState, info->ui()->edtState->text());
    }

    TEST_F(TestHomeInfo, icq_contact_basic_info_updated_event_updates_zip)
    {
        QString newZip = "New zip";
        contact->setZip(newZip);

        SIM::getEventHub()->triggerEvent("icq_contact_basic_info_updated",
                IcqContactUpdateData::create("icq_contact_basic_info_updated", QString::number(Uin)));

        ASSERT_EQ(newZip, info->ui()->edtZip->text());
    }

    TEST_F(TestHomeInfo, icq_contact_basic_info_updated_event_updates_country)
    {
        int newCountry = 93;
        QString newCountryName = "Afghanistan";
        contact->setCountry(newCountry);

        SIM::getEventHub()->triggerEvent("icq_contact_basic_info_updated",
                IcqContactUpdateData::create("icq_contact_basic_info_updated", QString::number(Uin)));

        ASSERT_EQ(newCountryName, info->ui()->cmbCountry->currentText());
    }
}
