
#include <QSignalSpy>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "signalspy.h"
#include "qt-gtest.h"
#include "icqclient.h"
#include "oscarsocket.h"
#include "buddysnachandler.h"
#include "requests/buddysnac/buddysnacrightsrequest.h"
#include "mocks/mockoscarsocket.h"
#include "icqstatus.h"
#include "events/eventhub.h"
#include "contacts/contactlist.h"
#include "contacts/contact.h"
#include "log.h"

namespace
{
    using ::testing::NiceMock;
    using ::testing::_;
    using ::testing::Return;

    static const QByteArray ContactTextUin("123456789", 9);
    static const int ContactUin = 123456789;
    static const int ContactId = 42;
    static const int MetaContactId = 21;
    static QByteArray ContactAvatarHash;

    class TestBuddySnacHandler : public ::testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            ContactAvatarHash = QByteArray(0x11, 16);
        }

        virtual void SetUp()
        {
            SIM::createContactList();
            socket = new NiceMock<MockObjects::MockOscarSocket>();
            ON_CALL(*socket, isConnected()).WillByDefault(Return(true));
            client = new ICQClient(0, "ICQ.123456", false);
            client->setOscarSocket(socket);

            metaContact = SIM::getContactList()->createContact(MetaContactId);
            SIM::getContactList()->addContact(metaContact);

            ICQContactList* list = client->contactList();
            contact = client->createIMContact().dynamicCast<ICQContact>();
            contact->setIcqID(ContactId);
            contact->setUin(ContactUin);
            list->addContact(contact);

            metaContact->addClientContact(contact);
            contact->setMetaContact(metaContact.data());

            handler = static_cast<BuddySnacHandler*>(client->snacHandler(ICQ_SNACxFOOD_BUDDY));
            ASSERT_TRUE(handler);
        }

        virtual void TearDown()
        {
            SIM::getContactList()->removeContact(MetaContactId);
            SIM::destroyContactList();
            delete client;
        }

        QByteArray makeOnlineBuddyPacket()
        {
            ByteArrayBuilder builder;

            builder.appendByte(ContactTextUin.length());
            builder.appendBytes(ContactTextUin);
            builder.appendWord(0); // Warning level

            ICQStatusPtr status = client->getDefaultStatus("online");
            TlvList list;
            list.append(Tlv::fromUint16(BuddySnacHandler::TlvUserClass, CLASS_FREE | CLASS_ICQ));
            list.append(Tlv::fromUint32(BuddySnacHandler::TlvOnlineSince, 0)); // Yeah, online since 1 Jan 1970
            list.append(Tlv::fromUint32(BuddySnacHandler::TlvOnlineStatus, status->icqId()));
            list.append(Tlv::fromUint32(BuddySnacHandler::TlvUserIp, 0));
            list.append(makeAvatarTlv());

            builder.appendWord(list.tlvCount());
            builder.appendBytes(list.toByteArray());

            return builder.getArray();
        }

        Tlv makeAvatarTlv()
        {
            ByteArrayBuilder builder;
            builder.appendWord(0x0001);
            builder.appendByte(0x01);
            builder.appendByte(ContactAvatarHash.length());
            builder.appendBytes(ContactAvatarHash);
            return Tlv(BuddySnacHandler::TlvAvatar, builder.getArray());
        }

        QByteArray makeRightsPacket()
        {
            TlvList list;
            list.append(Tlv::fromUint16(BuddySnacHandler::TlvMaxContacts, 1000));
            list.append(Tlv::fromUint16(BuddySnacHandler::TlvMaxWatchers, 3000));
            list.append(Tlv::fromUint16(BuddySnacHandler::TlvMaxOnlineNotifications, 512));
            return list.toByteArray();
        }

        SIM::ContactPtr metaContact;
        ICQClient* client;
        NiceMock<MockObjects::MockOscarSocket>* socket;
        BuddySnacHandler* handler;
        ICQContactPtr contact;
    };

    TEST_F(TestBuddySnacHandler, onlineNotificationProcessing)
    {
        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        ASSERT_TRUE(success);

        ICQContactPtr contact = client->contactList()->contact(ContactId);
        ASSERT_TRUE(contact->status());
        ASSERT_EQ("online", contact->status()->id());
    }

    TEST_F(TestBuddySnacHandler, onContactOnline_emitsEvent)
    {
        Helper::SignalSpy spy;
        SIM::getEventHub()->getEvent("contact_change_status")->connectTo(&spy, SLOT(intSlot(int)));
        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.intSlotCalls);
        ASSERT_EQ(MetaContactId, spy.intarg);
    }

    TEST_F(TestBuddySnacHandler, rightsPacket_ready)
    {
        QSignalSpy spy(handler, SIGNAL(ready()));
        bool success = handler->process(BuddySnacHandler::SnacBuddyRights, makeRightsPacket(), 0, 0);
        ASSERT_TRUE(success);

        ASSERT_EQ(1, spy.count());
        ASSERT_TRUE(handler->isReady());
    }

    TEST_F(TestBuddySnacHandler, requestRights_sendsSnac)
    {
        EXPECT_CALL(*socket, snac(handler->getType(), BuddySnacHandler::SnacBuddyRightsRequest, _, _));

        handler->requestRights();
    }

    // Disabled, because of server disconnects
    TEST_F(TestBuddySnacHandler, DISABLED_contactOnlineWithNewAvatarHash_requestsAvatar)
    {
        EXPECT_CALL(*socket, snac(BartSnacHandler::SnacId, BartSnacHandler::SnacRequestAvatar, _, _));

        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        EXPECT_TRUE(success);
    }

    TEST_F(TestBuddySnacHandler, DISABLED_contactOnlineWithOldAvatarHash_doesntRequestAvatar)
    {
        contact->setAvatarHash(ContactAvatarHash);
        EXPECT_CALL(*socket, snac(BartSnacHandler::SnacId, BartSnacHandler::SnacRequestAvatar, _, _)).Times(0);

        bool success = handler->process(BuddySnacHandler::SnacBuddyUserOnline, makeOnlineBuddyPacket(), 0, 0);
        EXPECT_TRUE(success);
    }

    TEST_F(TestBuddySnacHandler, Request_requestRights)
    {
        ICQRequestPtr rq = BuddySnacRightsRequest::create(client);
        EXPECT_CALL(*socket, snac(handler->getType(), BuddySnacHandler::SnacBuddyRightsRequest, _, _));

        rq->perform(socket);
    }
}
