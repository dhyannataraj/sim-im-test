
#ifndef MESSAGEOUTPIPE_H
#define MESSAGEOUTPIPE_H

#include "message.h"
namespace SIM
{
    class MessageOutPipe
    {
    public:
        virtual ~MessageOutPipe() {};
        virtual void pushMessage(const MessagePtr& message) = 0;

    };


}

#endif

