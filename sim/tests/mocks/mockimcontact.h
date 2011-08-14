#ifndef MOCKIMCONTACT_H
#define MOCKIMCONTACT_H

#include "contacts/imcontact.h"
#include <QSharedPointer>

#include "gmock/gmock.h"

namespace MockObjects
{
    class MockIMContact;
    typedef QSharedPointer<MockIMContact> MockIMContactPtr;
    typedef QSharedPointer<testing::NiceMock<MockIMContact> > NiceMockIMContactPtr;

    class MockIMContact : public SIM::IMContact
    {
    public:
        virtual ~MockIMContact() {}
        static MockIMContactPtr create() { return MockIMContactPtr(new MockIMContact()); }
        MOCK_METHOD0(client, SIM::Client*());
        MOCK_METHOD1(setParentContactId, void(int contactId));
        MOCK_CONST_METHOD0(parentContactId, int());
        MOCK_CONST_METHOD0(id, SIM::IMContactId());
        MOCK_CONST_METHOD0(status, SIM::IMStatusPtr());
        MOCK_CONST_METHOD0(name, QString());
        MOCK_METHOD1(sendMessage, bool(const SIM::MessagePtr& message));
        MOCK_METHOD0(hasUnreadMessages, bool());
        MOCK_METHOD0(dequeueUnreadMessage, SIM::MessagePtr());
        MOCK_METHOD1(enqueueUnreadMessage, void(const SIM::MessagePtr& message));
        MOCK_METHOD0(group, SIM::IMGroupWeakPtr());
        MOCK_METHOD0(makeToolTipText, QString());
        MOCK_METHOD1(serialize, void(QDomElement& element));
        MOCK_METHOD1(deserialize, void(QDomElement& element));
        MOCK_METHOD1(deserialize, bool(const QString&));
    };
}

#endif // MOCKIMCONTACT_H
