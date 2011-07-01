#include "containermanager.h"
#include "core.h"
#include "profilemanager.h"
#include "log.h"
#include "userwnd.h"
#include "messaging/messagepipe.h"

using SIM::log;
using SIM::L_DEBUG;
using SIM::L_ERROR;

ContainerManager::ContainerManager(CorePlugin* parent) :
    m_core(parent)
{
    m_sendProcessor = new SendMessageProcessor(this);
    SIM::getOutMessagePipe()->addMessageProcessor(m_sendProcessor);

    m_receiveProcessor = new ReceiveMessageProcessor(this);
    SIM::getMessagePipe()->addMessageProcessor(m_receiveProcessor);
}

ContainerManager::~ContainerManager()
{
    delete m_receiveProcessor;
    delete m_sendProcessor;
}

ContainerControllerPtr ContainerManager::makeContainerController(int id)
{
    return ContainerControllerPtr(new ContainerController(id));
}

bool ContainerManager::init()
{
    setContainerMode((ContainerMode)SIM::ProfileManager::instance()->getPropertyHub("_core")->value("ContainerMode").toUInt());
    return true;
}

void ContainerManager::addContainer(const ContainerControllerPtr& cont)
{
    m_containers.append(cont);
}

int ContainerManager::containerCount()
{
    return m_containers.count();
}

ContainerControllerPtr ContainerManager::containerController(int index)
{
    // FIXME Check boundaries ?
    return m_containers.at(index);
}

ContainerControllerPtr ContainerManager::containerControllerById(int id)
{
    foreach(const ContainerControllerPtr& c, m_containers) {
        if(c->id() == id)
            return c;
    }
    return ContainerControllerPtr();
}

void ContainerManager::removeContainer(int index)
{
    if((index < 0) || (index >= containerCount()))
        return;
    m_containers.removeAt(index);
}

void ContainerManager::messageSent(const SIM::MessagePtr& msg)
{
    SIM::IMContactPtr contact = msg->targetContact().toStrongRef();
    if(!contact)
    {
        SIM::log(SIM::L_ERROR, "ContainerManager::messageSent null contact");
        return;
    }

    log(L_DEBUG, "ContainerManager::messageSent: %d/%d", m_containers.size(), contact->parentContactId());
    foreach(const ContainerControllerPtr& c, m_containers)
    {
        UserWndControllerPtr userwnd = c->userWndController(contact->parentContactId());
        log(L_DEBUG, "%d/%d", userwnd->id(), contact->parentContactId());
        if(userwnd)
        {
            userwnd->addMessageToView(msg);
        }
    }
}

void ContainerManager::messageReceived(const SIM::MessagePtr& msg)
{
    SIM::IMContactPtr contact = msg->sourceContact().toStrongRef();
    if(!contact)
    {
        log(L_ERROR, "ContainerManager::messageReceived null contact");
        return;
    }

    log(L_ERROR, "ContainerManager::messageReceived");

    foreach(const ContainerControllerPtr& c, m_containers)
    {
        UserWndControllerPtr userwnd = c->userWndController(contact->parentContactId());
        if(userwnd)
        {
            log(L_ERROR, "ContainerManager::messageReceived userwnd found");
            userwnd->addMessageToView(msg);
        }
    }
}

void ContainerManager::removeContainerById(int id)
{
    int index = 0;
    foreach(const ContainerControllerPtr& c, m_containers) {
        if(c->id() == id) {
            m_containers.removeAt(index);
            return;
        }
        index++;
    }
}

void ContainerManager::contactChatRequested(int contactId)
{
    log(L_DEBUG, "contactChatRequested: %d", contactId);

    ContainerControllerPtr container = containerControllerById(0);
    if(!container)
    {
        container = makeContainerController(0);
        addContainer(container);
    }

    IUserWnd* userwnd = container->userWndById(contactId);
    if(!userwnd)
    {
        container->addUserWnd(contactId);
    }
}

ContainerManager::ContainerMode ContainerManager::containerMode() const
{
    return m_containerMode;
}

void ContainerManager::setContainerMode(ContainerManager::ContainerMode mode)
{
    m_containerMode = mode;
}
