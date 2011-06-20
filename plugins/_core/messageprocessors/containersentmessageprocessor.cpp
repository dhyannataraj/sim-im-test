#include "containersentmessageprocessor.h"
#include "containercontroller.h"

ContainerSentMessageProcessor::ContainerSentMessageProcessor(ContainerController* controller) : m_controller(controller)
{
}

QString ContainerSentMessageProcessor::id() const
{
    return QString("container_sent/%1").arg(m_controller->id());
}

SIM::MessageProcessor::ProcessResult ContainerSentMessageProcessor::process(const SIM::MessagePtr& message)
{
    return Success;
}
