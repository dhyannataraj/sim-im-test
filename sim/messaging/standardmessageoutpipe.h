
#ifndef STANDARDMESSAGEOUTPIPE_H
#define STANDARDMESSAGEOUTPIPE_H

#include "messageoutpipe.h"
#include "misc.h"

namespace SIM
{
    class SIM_EXPORT StandardMessageOutPipe : public MessageOutPipe
	{
	public:
        StandardMessageOutPipe();
        virtual ~StandardMessageOutPipe();

        virtual void pushMessage(const MessagePtr& message);

	};
}

#endif
