
#include "messageoutpipe.h"
#include "standardmessageoutpipe.h"

namespace SIM
{
    static MessageOutPipe* gs_outPipe = 0;
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

    void setOutMessagePipe(MessageOutPipe* pipe)
    {
        gs_outPipe = pipe;
    }

    MessageOutPipe* getOutMessagePipe()
    {
        return gs_outPipe;
    }
}
