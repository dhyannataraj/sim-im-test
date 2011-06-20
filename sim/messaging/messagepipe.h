#ifndef MESSAGEPIPE_H
#define MESSAGEPIPE_H

#include "simapi.h"

#include "message.h"
#include "messageprocessor.h"
#include "contacts/contactlist.h"

namespace SIM {

class EXPORT MessagePipe
{
public:
    virtual ~MessagePipe() {}
    virtual void pushMessage(const MessagePtr& message) = 0;
    virtual void addMessageProcessor(MessageProcessor* processor) = 0;
    virtual void removeMessageProcessor(const QString& id) = 0;
    virtual void setContactList(ContactList* contactList) = 0;

};

EXPORT void createMessagePipe();
EXPORT void destroyMessagePipe();

EXPORT void setMessagePipe(MessagePipe* pipe);
EXPORT MessagePipe* getMessagePipe();

EXPORT void createOutMessagePipe();
EXPORT void destroyOutMessagePipe();

EXPORT void setOutMessagePipe(MessagePipe* pipe);
EXPORT MessagePipe* getOutMessagePipe();


} // namespace SIM

#endif // MESSAGEPIPE_H
