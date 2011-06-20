#include "containercontroller.h"
#include "standarduserwndcontroller.h"
#include "messaging/messagepipe.h"
#include "icontainer.h"
#include "userwnd.h"

ContainerController::ContainerController(IContainer* view, int id) : m_view(view),
    m_id(id)
{
    m_sentMessage = new ContainerSentMessageProcessor(this);
}

ContainerController::~ContainerController()
{
    delete m_sentMessage;
}

int ContainerController::id() const
{
    return m_id;
}

void ContainerController::sendMessage(const SIM::MessagePtr& msg)
{
    SIM::getOutMessagePipe()->pushMessage(msg);
}

void ContainerController::addUserWnd(UserWnd* wnd)
{
	UserWndControllerPtr controller(new StandardUserWndController());
    controller->setUserWnd(wnd);
    m_controllers.append(controller);
    m_view->addUserWnd(wnd);
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
