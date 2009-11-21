

#include "testprotocolmanager.h"
#include "contacts/protocol.h"
#include "contacts.h"

namespace testProtocolManager
{
    class TestProtocol : public SIM::Protocol
    {
    public:
        TestProtocol() : Protocol(0)
        {
        }

        ~TestProtocol()
        {
        }

        virtual Client  *createClient(Buffer *cfg)
        {
            return 0;
        }

        virtual const CommandDef *description()
        {
            return 0;
        }

        virtual const CommandDef *statusList()
        {
            return 0;
        }

        virtual const DataDef *userDataDef()
        {
            return 0;
        }
    };

    void Test::initTestCase()
    {
        createContactList();
        createProtocolManager();
        QVERIFY(getProtocolManager() != 0);
    }

    void Test::cleanupTestCase()
    {
        destroyProtocolManager();
        destroyContactList();
        QVERIFY(getProtocolManager() == 0);
    }

    void Test::protocolManipulation()
    {
        ProtocolPtr first = ProtocolPtr(new TestProtocol()); 
        ProtocolPtr second = ProtocolPtr(new TestProtocol());
        QCOMPARE(getProtocolManager()->protocolCount(), 0);

        getProtocolManager()->addProtocol(first);
        QCOMPARE(getProtocolManager()->protocolCount(), 1);

        getProtocolManager()->addProtocol(second);
        QCOMPARE(getProtocolManager()->protocolCount(), 2);

        ProtocolPtr f = getProtocolManager()->protocol(0);
        QCOMPARE(f.data(), first.data());
        
        getProtocolManager()->removeProtocol(first);
        QCOMPARE(getProtocolManager()->protocolCount(), 1);

        getProtocolManager()->removeProtocol(first);
        QCOMPARE(getProtocolManager()->protocolCount(), 1);

        getProtocolManager()->removeProtocol(second);
        QCOMPARE(getProtocolManager()->protocolCount(), 0);
    }

    void Test::testProtocolIterator()
    {
        ProtocolPtr first = ProtocolPtr(new TestProtocol()); 
        ProtocolPtr second = ProtocolPtr(new TestProtocol());
        getProtocolManager()->addProtocol(first);
        getProtocolManager()->addProtocol(second);

        SIM::ProtocolIterator it;
        ProtocolPtr proto;
        proto = ++it;
        QCOMPARE(proto, first);
        proto = ++it;
        QCOMPARE(proto, second);
        proto = ++it;
        QCOMPARE(proto, ProtocolPtr());
    }
}

// vim: set expandtab:

