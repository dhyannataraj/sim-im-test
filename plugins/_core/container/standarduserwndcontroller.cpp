#include "standarduserwndcontroller.h"
#include "userwnd.h"

StandardUserWndController::StandardUserWndController(int contactId) : m_id(contactId)
{
    m_userWnd = createUserWnd(contactId);
}

StandardUserWndController::~StandardUserWndController()
{
    if(m_userWnd)
        delete m_userWnd;
}

int StandardUserWndController::id() const
{
    return m_id;
}

void StandardUserWndController::setUserWnd(IUserWnd* wnd)
{
    if(m_userWnd == wnd)
        return;
    if(m_userWnd)
        delete m_userWnd;
    m_userWnd = wnd;
}

IUserWnd* StandardUserWndController::userWnd() const
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

IUserWnd* StandardUserWndController::createUserWnd(int id)
{
    return new UserWnd(id, false, false);
}
