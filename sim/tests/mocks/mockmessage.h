
#ifndef MOCKMESSAGE_H
#define MOCKMESSAGE_H

#include <QSharedPointer>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "messaging/message.h"

namespace MockObjects
{
    class MockMessage;
    typedef QSharedPointer<MockMessage> MockMessagePtr;
    class MockMessage : public SIM::Message
    {
    public:
        static MockMessagePtr create() { return MockMessagePtr(new MockMessage()); }

        MOCK_CONST_METHOD0(type, QString());
        MOCK_METHOD0(client, SIM::Client*());
        MOCK_CONST_METHOD0(targetContact, SIM::IMContactWeakPtr());
        MOCK_CONST_METHOD0(sourceContact, SIM::IMContactWeakPtr());
        MOCK_CONST_METHOD0(targetContactName, QString());
        MOCK_CONST_METHOD0(sourceContactName, QString());

        MOCK_METHOD0(icon, QIcon());
        MOCK_METHOD0(timestamp, QDateTime());
        MOCK_METHOD0(toXml, QString());
        MOCK_METHOD0(toPlainText, QString());
        MOCK_CONST_METHOD0(originatingClientId, QString());
        MOCK_METHOD0(choices, QStringList());
    };
}

#endif

