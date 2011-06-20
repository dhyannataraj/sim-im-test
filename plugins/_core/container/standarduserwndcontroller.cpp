#include "standarduserwndcontroller.h"
#include "userwnd.h"

StandardUserWndController::StandardUserWndController()
{
}

void StandardUserWndController::setUserWnd(UserWnd* wnd)
{
    m_userWnd = wnd;
}

UserWnd* StandardUserWndController::userWnd() const
{
    return m_userWnd;
}

void StandardUserWndController::addMessageToView(const SIM::MessagePtr& message)
{
    m_userWnd->addMessageToView(message);
}

int StandardUserWndController::messagesCount() const
{
    return m_userWnd->messagesInViewArea();
}
