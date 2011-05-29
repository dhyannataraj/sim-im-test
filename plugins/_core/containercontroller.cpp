#include "containercontroller.h"
#include "messaging/messageoutpipe.h"

ContainerController::ContainerController(Container* view, int id) : m_view(view),
    m_id(id)
{
}

int ContainerController::id() const
{
    return m_id;
}

void ContainerController::sendMessage(const SIM::MessagePtr& msg)
{
    SIM::getOutMessagePipe()->pushMessage(msg);
}
