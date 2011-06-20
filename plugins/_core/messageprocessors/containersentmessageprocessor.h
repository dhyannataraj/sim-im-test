#ifndef CONTAINERSENTMESSAGEPROCESSOR_H
#define CONTAINERSENTMESSAGEPROCESSOR_H

#include "messaging/messageprocessor.h"

class ContainerController;
class ContainerSentMessageProcessor : public SIM::MessageProcessor
{
public:
    ContainerSentMessageProcessor(ContainerController* controller);

    virtual QString id() const;
    virtual ProcessResult process(const SIM::MessagePtr& message);

private:
    ContainerController* m_controller;
};

#endif // CONTAINERSENTMESSAGEPROCESSOR_H
