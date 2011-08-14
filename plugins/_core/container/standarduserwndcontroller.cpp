#include "standarduserwndcontroller.h"
#include "userwnd.h"
#include "icontainer.h"
#include "log.h"

using SIM::log;
using SIM::L_DEBUG;

StandardUserWndController::StandardUserWndController(int contactId) : m_id(contactId)
{
    m_userWnd = createUserWnd(contactId);
}

StandardUserWndController::~StandardUserWndController()
{
    log(L_DEBUG, "StandardUserWndController::~StandardUserWndController(): %d", m_id);
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

void StandardUserWndController::setContainer(IContainer* cont)
{
    m_container = cont;
}

IContainer* StandardUserWndController::container() const
{
    return m_container;
}

void StandardUserWndController::raise()
{

}

void StandardUserWndController::addMessageToView(const SIM::MessagePtr& message)
{
    log(L_DEBUG, "StandardUserWndController::addMessageToView");
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
