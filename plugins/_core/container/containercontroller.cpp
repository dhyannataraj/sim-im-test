#include "containercontroller.h"
#include "standarduserwndcontroller.h"
#include "messaging/messagepipe.h"
#include "container.h"
#include "userwnd.h"

ContainerController::ContainerController(int id) : m_id(id)
{
    Container* container = new Container(id);
    m_view = IContainerPtr(container);
    m_view->setController(this);
    connect(container, SIGNAL(closed()), this, SLOT(containerClosed()));
}

ContainerController::~ContainerController()
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

void ContainerController::addUserWnd(int contactId)
{
    UserWndControllerPtr controller = createUserWndController(contactId);
    m_controllers.append(controller);
    m_view->addUserWnd(controller->userWnd());
}

IUserWnd* ContainerController::userWndById(int id)
{
    foreach(const UserWndControllerPtr& controller, m_controllers)
    {
        IUserWnd* thisWnd = controller->userWnd();
        if(thisWnd->id() == id)
            return thisWnd;
    }
    return 0;
}

UserWndControllerPtr ContainerController::userWndController(int id)
{
    foreach(const UserWndControllerPtr& controller, m_controllers)
    {
        IUserWnd* thisWnd = controller->userWnd();
        if(thisWnd->id() == id)
            return controller;
    }
    return UserWndControllerPtr();
}

void ContainerController::raiseUserWnd(int id)
{
    m_view->raiseUserWnd(id);
}

void ContainerController::containerClosed()
{
    emit closed(id());
}

UserWndControllerPtr ContainerController::createUserWndController(int id)
{
    StandardUserWndController* controller = new StandardUserWndController(id);
    connect(controller, SIGNAL(messageSendRequest(SIM::MessagePtr)), this, SLOT(messageSendRequest(SIM::MessagePtr)));
    return UserWndControllerPtr(controller);
}

void ContainerController::messageSendRequest(const SIM::MessagePtr& message)
{
    sendMessage(message);
}
