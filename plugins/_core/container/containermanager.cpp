#include "containermanager.h"
#include "core.h"
#include "profilemanager.h"
#include "log.h"
#include "userwnd.h"
#include "messaging/messagepipe.h"

#include <cstdio>

using SIM::log;
using SIM::L_DEBUG;
using SIM::L_ERROR;

ContainerManager::ContainerManager(CorePlugin* parent) :
    m_containerControllerId(0),
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

ContainerControllerPtr ContainerManager::makeContainerController()
{
    ContainerController* controller = new ContainerController(m_containerControllerId++);
    connect(controller, SIGNAL(closed(int)), this, SLOT(containerClosed(int)));
    return ContainerControllerPtr(controller);
}

bool ContainerManager::init()
{
    setContainerMode((ContainerMode)SIM::getProfileManager()->getPropertyHub("_core")->value("ContainerMode").toUInt());
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

    log(L_DEBUG, "ContainerManager::messageSent");
    UserWndControllerPtr userwnd = findUserWnd(contact->parentContactId());
    if(userwnd)
    {
        userwnd->addMessageToView(msg);
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

    log(L_DEBUG, "ContainerManager::messageReceived");
    UserWndControllerPtr userwnd = findUserWnd(contact->parentContactId());
    if(!userwnd)
    {
        log(L_DEBUG, "ContainerManager::messageReceived, creating container");
        ContainerControllerPtr container = containerControllerById(0);
        if(!container)
        {
            container = makeContainerController();
            addContainer(container);
        }

        container->addUserWnd(contact->parentContactId());
        userwnd = container->userWndController(contact->parentContactId());
    }
    userwnd->addMessageToView(msg);
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

UserWndControllerPtr ContainerManager::findUserWnd(int id)
{
    foreach(const ContainerControllerPtr& c, m_containers)
    {
        UserWndControllerPtr userwnd = c->userWndController(id);
        if(userwnd)
        {
            return userwnd;
        }
    }
    return UserWndControllerPtr();
}

ContainerControllerPtr ContainerManager::containerControllerForUserWnd(int userWndId)
{
    foreach(const ContainerControllerPtr& c, m_containers)
    {
        UserWndControllerPtr userwnd = c->userWndController(userWndId);
        if(userwnd)
        {
            return c;
        }
    }
    return ContainerControllerPtr();
}

void ContainerManager::contactChatRequested(int contactId, const QString& messageType)
{
    log(L_DEBUG, "contactChatRequested: %d", contactId);

    ContainerControllerPtr container = containerControllerForUserWnd(contactId);
    if(container)
    {
        container->raiseUserWnd(contactId);
        return;
    }
    else
    {
        container = containerControllerById(0);
        if(!container)
        {
            container = makeContainerController();
            addContainer(container);
        }
        container->addUserWnd(contactId);
        container->raiseUserWnd(contactId);

        UserWndControllerPtr userWnd = container->userWndController(contactId);
        userWnd->setMessageType(messageType);
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

void ContainerManager::containerClosed(int id)
{
    log(L_DEBUG, "ContainerManager::containerClosed: %d", id);
    removeContainerById(id);
}
