/*
 * testicqinfo.cpp
 *
 *  Created on: Sep 17, 2011
 */


#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "icqclient.h"
#include "icqcontact.h"

#include "icqconfig/icqinfo.h"
#include "mocks/mockoscarsocket.h"
#include "metainfosnachandler.h"

namespace 
{
    static const int Uin = 1234567;
    static const QString FirstName = "First Name";
    static const QString LastName = "Last Name";
    static const QString NickName = "Nick Name";

    using ::testing::_;

    class SutICQInfo : public ICQInfo
    {
    public:
        SutICQInfo(QWidget* parent, const ICQContactPtr& contact, ICQClient* client) : ICQInfo(parent, contact, client) {}
        Ui::MainInfo* ui() { return ICQInfo::ui(); }
    };

    class TestIcqInfo : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            client = new ICQClient(0, "123456", false);
            contact = ICQContactPtr(new ICQContact(client));
            contact->setUin(Uin);
            contact->setFirstName(FirstName);
            contact->setLastName(LastName);
            contact->setNick(NickName);

            socket = new MockObjects::MockOscarSocket();
            client->setOscarSocket(socket);

            info = new SutICQInfo(0, contact, client);
        }

        virtual void TearDown()
        {
            delete info;
            delete client;
        }

        ICQClient* client;
        ICQContactPtr contact;
        SutICQInfo* info;
        MockObjects::MockOscarSocket* socket;
    };

    TEST_F(TestIcqInfo, constructor_sets_uin)
    {
        ASSERT_EQ(QString::number(Uin), info->ui()->edtUin->text());
    }

    TEST_F(TestIcqInfo, constructor_sets_first_name)
    {
        ASSERT_EQ(FirstName, info->ui()->edtFirst->text());
    }

    TEST_F(TestIcqInfo, constructor_sets_last_name)
    {
        ASSERT_EQ(LastName, info->ui()->edtLast->text());
    }

    TEST_F(TestIcqInfo, constructor_sets_nick)
    {
        ASSERT_EQ(NickName, info->ui()->edtNick->text());
    }
}
