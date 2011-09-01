/*
 * testicqconfigwidgetcreator.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "../icqconfig/icqconfigwidgetcreator.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "contacts/contactlist.h"
#include "core.h"

#include "icqclient.h"
#include "icqcontact.h"
#include "events/eventhub.h"
#include "events/widgetcollectionevent.h"



namespace
{
    static const int ContactId = 12;
    using namespace testing;
    class TestIcqConfigWidgetCreator : public Test
    {
    public:
        virtual ~TestIcqConfigWidgetCreator() {}

        virtual void SetUp()
        {
            SIM::createContactList();
            core = new CorePlugin();
            client = new ICQClient(0, "ICQ.123456", false);
            contact = ICQContactPtr(new ICQContact(client));
            client->contactList()->addContact(contact);

            metacontact = SIM::getContactList()->createContact(ContactId);
            SIM::getContactList()->addContact(metacontact);
            metacontact->addClientContact(contact);
        }

        virtual void TearDown()
        {
            delete core;
            SIM::destroyContactList();
        }
        CorePlugin* core;
        ICQClient* client;
        ICQContactPtr contact;
        SIM::ContactPtr metacontact;
    };

    TEST_F(TestIcqConfigWidgetCreator, contactConfigRequested_addsItemsIntoHierarchy)
    {
        IcqConfigWidgetCreator creator(client);
        SIM::getEventHub()->getEvent("contact_widget_collection")->connectTo(&creator, SLOT(contactConfigRequested(SIM::WidgetHierarchy*, QString)));

        SIM::WidgetCollectionEventDataPtr data = SIM::WidgetCollectionEventData::create("contact_widget_collection", QString::number(ContactId));
        SIM::getEventHub()->triggerEvent("contact_widget_collection", data);

        ASSERT_NE(0, data->hierarchyRoot()->children.size());
    }

    TEST_F(TestIcqConfigWidgetCreator, contactConfigRequested_addsICQInfoToHierarchy)
    {
        IcqConfigWidgetCreator creator(client);
        SIM::getEventHub()->getEvent("contact_widget_collection")->connectTo(&creator, SLOT(contactConfigRequested(SIM::WidgetHierarchy*, QString)));

        SIM::WidgetCollectionEventDataPtr data = SIM::WidgetCollectionEventData::create("contact_widget_collection", QString::number(ContactId));
        SIM::getEventHub()->triggerEvent("contact_widget_collection", data);

        QWidget* w = data->hierarchyRoot()->children.at(0).widget;
        ASSERT_TRUE(w);
        ASSERT_TRUE(w->inherits("ICQInfo"));
    }
}
