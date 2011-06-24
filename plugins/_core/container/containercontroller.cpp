#include "containercontroller.h"
#include "standarduserwndcontroller.h"
#include "messaging/messagepipe.h"
#include "container.h"
#include "userwnd.h"

ContainerController::ContainerController(int id) : m_id(id)
{
    m_view = IContainerPtr(new Container(id));
    m_view->setController(this);
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
    UserWndControllerPtr controller = createUserWndController();
    m_controllers.append(controller);
}

UserWnd* ContainerController::userWndById(int id)
{
    foreach(const UserWndControllerPtr& controller, m_controllers)
    {
        UserWnd* thisWnd = controller->userWnd();
        if(thisWnd->id() == id)
            return thisWnd;
    }
    return 0;
}

UserWndControllerPtr ContainerController::userWndController(int id)
{
    foreach(const UserWndControllerPtr& controller, m_controllers)
    {
        UserWnd* thisWnd = controller->userWnd();
        if(thisWnd->id() == id)
            return controller;
    }
    return UserWndControllerPtr();
}

UserWndControllerPtr ContainerController::createUserWndController()
{
    return UserWndControllerPtr(new StandardUserWndController());
}
