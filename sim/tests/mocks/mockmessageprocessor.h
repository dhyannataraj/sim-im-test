#ifndef MOCKMESSAGEPROCESSOR_H
#define MOCKMESSAGEPROCESSOR_H

#include "messaging/messageprocessor.h"

namespace MockObjects
{
    class MockMessageProcessor : public SIM::MessageProcessor
    {
    public:
        MOCK_CONST_METHOD0(id, QString());
        MOCK_METHOD1(process, MessageProcessor::ProcessResult(const SIM::MessagePtr& message));
    };
}


#endif // MOCKMESSAGEPROCESSOR_H
