#ifndef MOCKMESSAGEPIPE_H
#define MOCKMESSAGEPIPE_H

#include "messaging/messagepipe.h"
#include "messaging/messageprocessor.h"
#include "contacts/contactlist.h"

namespace MockObjects
{
    class MockMessagePipe : public SIM::MessagePipe
    {
    public:
        MOCK_METHOD1(pushMessage, void(const SIM::MessagePtr& message));
        MOCK_METHOD1(addMessageProcessor, void(SIM::MessageProcessor* processor));
        MOCK_METHOD1(removeMessageProcessor, void(const QString& id));
        MOCK_METHOD1(setContactList, void(SIM::ContactList* contactList));
    };
}

#endif // MOCKMESSAGEPIPE_H
