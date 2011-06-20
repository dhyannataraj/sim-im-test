#ifndef STANDARDUSERWNDCONTROLLER_H
#define STANDARDUSERWNDCONTROLLER_H

#include "userwndcontroller.h"

class StandardUserWndController : public UserWndController
{
public:
    StandardUserWndController();

    virtual void setUserWnd(UserWnd* wnd);
    virtual UserWnd* userWnd() const;

    virtual void addMessageToView(const SIM::MessagePtr& message);
    virtual int messagesCount() const;
private:
    UserWnd* m_userWnd;
};

#endif // STANDARDUSERWNDCONTROLLER_H
