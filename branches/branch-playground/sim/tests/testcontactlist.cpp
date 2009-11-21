
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

    void TestContactList::initTestCase()
    {
        SIM::createContactList();
        m_contactList = SIM::getContacts();
    }

    void TestContactList::cleanupTestCase()
    {
        SIM::destroyContactList();
    }

    void TestContactList::testClientManipulation()
    {
        TestClient* client1 = new TestClient(0, 0);
        TestClient* client2 = new TestClient(0, 0);
        m_contactList->addClient(client1);
        m_contactList->addClient(client2);
        QCOMPARE(client1, m_contactList->getClient(0));
        QCOMPARE(client2, m_contactList->getClient(1));
    }
}

// vim: set expandtab:

