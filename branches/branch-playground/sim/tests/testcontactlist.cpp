
#include "testcontactlist.h"

namespace testContactList
{
    TestClient::TestClient(Protocol* protocol, Buffer* buf) : Client(protocol, buf)
    {
    }

    QString TestClient::name()
    {
    }

    QString TestClient::dataName(void*)
    {
    }

    QWidget* TestClient::setupWnd()
    {
    }

    bool TestClient::isMyData(clientData*& data, Contact*& contact)
    {
    }

    bool TestClient::createData(clientData*& data, Contact* contact)
    {
    }

    void TestClient::contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons)
    {
    }

    void TestClient::setupContact(Contact*, void *data)
    {
    }

    bool TestClient::send(Message*, void *data)
    {
    }

    bool TestClient::canSend(unsigned type, void *data)
    {
    }

    QWidget* TestClient::searchWindow(QWidget *parent)
    {
    }

    Test::Test() : QObject(), SIM::EventReceiver()
    {
    }

    bool Test::processEvent(SIM::Event* event)
    {
        switch(event->type())
        {
            case eEventGroup:
            {
                SIM::EventGroup* ev = static_cast<EventGroup*>(event);
                if(ev->action() == SIM::EventGroup::eAdded)
                {
                    m_groupAdded++;
                }
                break;
            }
            case eEventContact:
            {
                SIM::EventContact* ev = static_cast<EventContact*>(event);
                if(ev->action() == SIM::EventContact::eAdded)
                {
                    m_contactAdded++;
                }
                break;
            }
            default:
            break;
        }
    }


    void Test::initTestCase()
    {
        m_groupAdded = 0;
        SIM::createContactList();
        m_contactList = SIM::getContacts();
    }

    void Test::cleanupTestCase()
    {
        SIM::destroyContactList();
    }

    void Test::testClientManipulation()
    {
        TestClient* client1 = new TestClient(0, 0);
        TestClient* client2 = new TestClient(0, 0);
        getContacts()->addClient(client1);
        getContacts()->addClient(client2);
        QCOMPARE(client1, getContacts()->getClient(0));
        QCOMPARE(client2, getContacts()->getClient(1));

        getContacts()->moveClient(client1, false);
        QCOMPARE(client2, getContacts()->getClient(0));
        QCOMPARE(client1, getContacts()->getClient(1));
    }

    void Test::testGroupManipulation()
    {
        QCOMPARE(getContacts()->groupCount(), 1);
        Group* zero = getContacts()->group(0);

        Group* group1 = getContacts()->group(12, true);
        QVERIFY(group1);
        QCOMPARE(m_groupAdded, 1);
        Group* group2 = getContacts()->group(23, true);
        QVERIFY(group2);
        QCOMPARE(m_groupAdded, 2);
        Group* group3 = getContacts()->group(34, true);
        QVERIFY(group3);
        QCOMPARE(m_groupAdded, 3);
        Group* nonexistant = getContacts()->group(66, false);
        QVERIFY(!nonexistant);

        QCOMPARE(getContacts()->groupCount(), 4);
        QCOMPARE(getContacts()->groupIndex(12), 1);
        QCOMPARE(getContacts()->groupIndex(23), 2);
        QCOMPARE(getContacts()->groupIndex(34), 3);
        QCOMPARE(getContacts()->groupIndex(66), -1);

        QCOMPARE(getContacts()->groupExists(12), true);
        QCOMPARE(getContacts()->groupExists(23), true);
        QCOMPARE(getContacts()->groupExists(34), true);
        QCOMPARE(getContacts()->groupExists(66), false);
        QCOMPARE(getContacts()->groupExists(42), false);

        getContacts()->moveGroup(12, false);
        QCOMPARE(getContacts()->groupIndex(12), 2);
        QCOMPARE(getContacts()->groupIndex(23), 1);

        getContacts()->moveGroup(12, false);
        QCOMPARE(getContacts()->groupIndex(12), 3);
        QCOMPARE(getContacts()->groupIndex(34), 2);

        getContacts()->moveGroup(34, true);
        QCOMPARE(getContacts()->groupIndex(12), 3);
        QCOMPARE(getContacts()->groupIndex(23), 2);
        QCOMPARE(getContacts()->groupIndex(34), 1);

        getContacts()->clear();
        QCOMPARE(getContacts()->groupCount(), 1);
    }

    void Test::testGroupIterator()
    {
        Group* zero = getContacts()->group(0);
        Group* group1 = getContacts()->group(12, true);
        Group* group2 = getContacts()->group(23, true);
        Group* group3 = getContacts()->group(34, true);

        SIM::ContactList::GroupIterator it;
        Group* null = 0;
        Group* group = 0;
        group = ++it;
        QCOMPARE(group, zero);
        group = ++it;
        QCOMPARE(group, group1);
        group = ++it;
        QCOMPARE(group, group2);
        group = ++it;
        QCOMPARE(group, group3);
        group = ++it;
        QCOMPARE(group, null);
    }

    void Test::testContactManipulation()
    {
    }
}

// vim: set expandtab:

