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

namespace 
{
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
}
