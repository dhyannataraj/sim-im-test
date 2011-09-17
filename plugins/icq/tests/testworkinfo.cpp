/*
 * testworkinfo.cpp
 *
 *  Created on: Sep 4, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "icqconfig/workinfo.h"
#include "icqclient.h"
#include "icqcontact.h"
#include "events/icqcontactupdate.h"
#include "events/eventhub.h"

namespace 
{
    static const int Uin = 12345678;
    static const QString WorkAddress = "Test address";
    static const QString WorkCity = "Test city";
    static const QString WorkState = "Test state";
    static const QString WorkZip = "Test zip";
    static const int WorkCountryCode = 263;
    static const QString WorkCountryName = "Zimbabwe";
    static const int OccupationCode = 1;
    static const QString OccupationName = "Academic";
    static const QString WorkName = "Test workname";
    static const QString WorkDepartment = "Test department";
    static const QString WorkPosition = "Test Position";
    static const QString WorkSite = "Test site";

    class SutWorkInfo : public WorkInfo
    {
    public:
        SutWorkInfo(QWidget *parent, const ICQContactPtr& contact, ICQClient *client) : WorkInfo(parent, contact, client) {}
        Ui::WorkInfo* ui() const { return WorkInfo::ui(); }
    };

    class TestWorkInfo : public testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "123456", false);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setUin(Uin);
            contact->setWorkAddress(WorkAddress);
            contact->setWorkCity(WorkCity);
            contact->setWorkState(WorkState);
            contact->setWorkZip(WorkZip);
            contact->setWorkCountry(WorkCountryCode);
            contact->setOccupation(OccupationCode);
            contact->setWorkName(WorkName);
            contact->setWorkDepartment(WorkDepartment);
            contact->setWorkPosition(WorkPosition);
            contact->setWorkHomepage(WorkSite);

            info = new SutWorkInfo(0, contact, client);
        }

        virtual void TearDown()
        {
            delete info;
            delete client;
        }

        ICQClient* client;
        ICQContactPtr contact;
        SutWorkInfo* info;
    };

    TEST_F(TestWorkInfo, constructor_setsWorkAddress)
    {
        ASSERT_EQ(WorkAddress, info->ui()->edtAddress->toPlainText());
    }

    TEST_F(TestWorkInfo, constructor_setsWorkCity)
    {
        ASSERT_EQ(WorkCity, info->ui()->edtCity->text());
    }

    TEST_F(TestWorkInfo, constructor_setsWorkState)
    {
        ASSERT_EQ(WorkState, info->ui()->edtState->text());
    }

    TEST_F(TestWorkInfo, constructor_setsWorkZip)
    {
        ASSERT_EQ(WorkZip, info->ui()->edtZip->text());
    }

    TEST_F(TestWorkInfo, constructor_setsWorkCountry)
    {
        ASSERT_EQ(WorkCountryName, info->ui()->cmbCountry->currentText());
    }

    TEST_F(TestWorkInfo, constructor_setsOccupation)
    {
        ASSERT_EQ(OccupationName, info->ui()->cmbOccupation->currentText());
    }

    TEST_F(TestWorkInfo, constructor_setsWorkName)
    {
        ASSERT_EQ(WorkName, info->ui()->edtName->text());
    }

    TEST_F(TestWorkInfo, constructor_setsDepartment)
    {
        ASSERT_EQ(WorkDepartment, info->ui()->edtDept->text());
    }

    TEST_F(TestWorkInfo, constructor_setsPosition)
    {
        ASSERT_EQ(WorkPosition, info->ui()->edtPosition->text());
    }

    TEST_F(TestWorkInfo, constructor_setsHomepage)
    {
        ASSERT_EQ(WorkSite, info->ui()->edtSite->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_workAddress)
    {
        QString newWorkAddress = "New Work Address";
        contact->setWorkAddress(newWorkAddress);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newWorkAddress, info->ui()->edtAddress->toPlainText());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_workCity)
    {
        QString newWorkCity = "New Work City";
        contact->setWorkCity(newWorkCity);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newWorkCity, info->ui()->edtCity->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_workState)
    {
        QString newWorkState = "New Work State";
        contact->setWorkState(newWorkState);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newWorkState, info->ui()->edtState->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_workZip)
    {
        QString newWorkZip = "New Work Zip";
        contact->setWorkZip(newWorkZip);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newWorkZip, info->ui()->edtZip->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_workCountry)
    {
        int newWorkCountry = 355;
        QString newWorkCountryName = "Albania";
        contact->setWorkCountry(newWorkCountry);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newWorkCountryName, info->ui()->cmbCountry->currentText());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_occupation)
    {
        int newOccupation = 14;
        QString newOccupationName = "Law";
        contact->setOccupation(newOccupation);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newOccupationName, info->ui()->cmbOccupation->currentText());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_workName)
    {
        QString newWorkName = "New Work Name";
        contact->setWorkName(newWorkName);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newWorkName, info->ui()->edtName->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_department)
    {
        QString newDepartment = "New Department";
        contact->setWorkDepartment(newDepartment);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newDepartment, info->ui()->edtDept->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_position)
    {
        QString newPosition = "New Position";
        contact->setWorkPosition(newPosition);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newPosition, info->ui()->edtPosition->text());
    }

    TEST_F(TestWorkInfo, icq_contact_work_info_updated_event_updates_homepage)
    {
        QString newHomepage = "New Work Homepage";
        contact->setWorkHomepage(newHomepage);

        SIM::getEventHub()->triggerEvent("icq_contact_work_info_updated",
                IcqContactUpdateData::create("icq_contact_work_info_updated", QString::number(Uin)));

        ASSERT_EQ(newHomepage, info->ui()->edtSite->text());
    }
}
