#include "standardmessageoutpipe.h"
#include "contacts/imcontact.h"

namespace SIM
{
    StandardMessageOutPipe::StandardMessageOutPipe()
    {

    }

    StandardMessageOutPipe::~StandardMessageOutPipe()
    {

    }

    void StandardMessageOutPipe::pushMessage(const MessagePtr& message)
    {
        IMContactPtr contact = message->contact().toStrongRef();
        if(!contact)
            return;
        contact->sendMessage(message);
    }
}
