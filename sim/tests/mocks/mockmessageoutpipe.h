#ifndef MOCKMESSAGEOUTPIPE_H
#define MOCKMESSAGEOUTPIPE_H

#include "messaging/messageoutpipe.h"

namespace MockObjects
{
    class MockMessageOutPipe : public SIM::MessageOutPipe
    {
    public:
        MOCK_METHOD1(pushMessage, void(const SIM::MessagePtr& message));
    };
}

#endif // MOCKMESSAGEOUTPIPE_H
