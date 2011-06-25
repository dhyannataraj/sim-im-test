#ifndef USERWNDCONTROLLER_H
#define USERWNDCONTROLLER_H

#include <QSharedPointer>
#include "messaging/message.h"

class IUserWnd;
class UserWndController
{
public:
    virtual ~UserWndController() {}

    virtual int id() const = 0;

    virtual void setUserWnd(IUserWnd* wnd) = 0;
    virtual IUserWnd* userWnd() const = 0;

    virtual void addMessageToView(const SIM::MessagePtr& message) = 0;
    virtual int messagesCount() const = 0;
};

typedef QSharedPointer<UserWndController> UserWndControllerPtr;

#endif // USERWNDCONTROLLER_H
