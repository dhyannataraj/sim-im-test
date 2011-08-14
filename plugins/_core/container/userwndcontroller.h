#ifndef USERWNDCONTROLLER_H
#define USERWNDCONTROLLER_H

#include <QSharedPointer>
#include "messaging/message.h"

class IContainer;
class IUserWnd;
class UserWndController
{
public:
    virtual ~UserWndController() {}

    virtual int id() const = 0;

    virtual void setUserWnd(IUserWnd* wnd) = 0;
    virtual IUserWnd* userWnd() const = 0;

    virtual void setContainer(IContainer* cont) = 0;
    virtual IContainer* container() const = 0;

    virtual void raise() = 0;

    virtual void addMessageToView(const SIM::MessagePtr& message) = 0;
    virtual int messagesCount() const = 0;
};

typedef QSharedPointer<UserWndController> UserWndControllerPtr;

#endif // USERWNDCONTROLLER_H
