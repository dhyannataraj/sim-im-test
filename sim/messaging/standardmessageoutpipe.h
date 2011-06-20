
#ifndef STANDARDMESSAGEOUTPIPE_H
#define STANDARDMESSAGEOUTPIPE_H

#include "messagepipe.h"
#include "contacts/contactlist.h"
#include "messageprocessor.h"
#include "misc.h"

namespace SIM
{
    class SIM_EXPORT StandardMessageOutPipe : public MessagePipe
	{
	public:
        StandardMessageOutPipe();
        virtual ~StandardMessageOutPipe();

        virtual void pushMessage(const MessagePtr& message);

        virtual void addMessageProcessor(MessageProcessor* processor);
        virtual void removeMessageProcessor(const QString& id);
        virtual void setContactList(ContactList* contactList);

    private:
        ContactList* m_contactList;
        QList<MessageProcessor*> m_processors;

	};
}

#endif
