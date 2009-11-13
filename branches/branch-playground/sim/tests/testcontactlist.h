
#ifndef SIM_TESTCONTACTLIST_H
#define SIM_TESTCONTACTLIST_H

#include <QtTest/QtTest>
#include <QObject>
#include "contacts.h"
#include "contacts/client.h"

namespace testContactList
{
    using namespace SIM;
    class TestClient : public Client
    {
    public:
        TestClient(Protocol* protocol, Buffer* buf);
        virtual QString name();
        virtual QString dataName(void*);
        virtual QWidget* setupWnd();
        virtual bool isMyData(clientData*&, Contact*&);
        virtual bool createData(clientData*&, Contact*);
        virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
        virtual void setupContact(Contact*, void *data);
        virtual bool send(Message*, void *data);
        virtual bool canSend(unsigned type, void *data);
        virtual QWidget* searchWindow(QWidget *parent);
    };

    class TestContactList : public QObject
    {
        Q_OBJECT
    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testClientManipulation();
        
    private:
        ContactList* m_contactList;
    };

}

#endif

// vim: set expandtab:

