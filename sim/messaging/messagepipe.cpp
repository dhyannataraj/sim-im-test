#include "messagepipe.h"
#include "standardmessagepipe.h"
#include "standardmessageoutpipe.h"

namespace SIM {

static MessagePipe* gs_messagePipe = NULL;

EXPORT void createMessagePipe()
{
    Q_ASSERT(!gs_messagePipe);
    gs_messagePipe = new StandardMessagePipe();
}

EXPORT void destroyMessagePipe()
{
    Q_ASSERT(gs_messagePipe);
    delete gs_messagePipe;
    gs_messagePipe = NULL;
}

EXPORT void setMessagePipe(MessagePipe* pipe)
{
    if(gs_messagePipe)
        delete gs_messagePipe;
    gs_messagePipe = pipe;
}

EXPORT MessagePipe* getMessagePipe()
{
    return gs_messagePipe;
}

static MessagePipe* gs_outPipe = 0;
void createOutMessagePipe()
{
    Q_ASSERT(!gs_outPipe);
    gs_outPipe = new StandardMessageOutPipe();
}

void destroyOutMessagePipe()
{
    Q_ASSERT(gs_outPipe);
    delete gs_outPipe;
    gs_outPipe = 0;
}

void setOutMessagePipe(MessagePipe* pipe)
{
    gs_outPipe = pipe;
}

MessagePipe* getOutMessagePipe()
{
    return gs_outPipe;
}

} // namespace SIM
