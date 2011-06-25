#ifndef STANDARDUSERWNDCONTROLLER_H
#define STANDARDUSERWNDCONTROLLER_H

#include "userwndcontroller.h"

class IUserWnd;
class StandardUserWndController : public UserWndController
{
public:
    StandardUserWndController(int contactId);
    virtual ~StandardUserWndController();

    virtual int id() const;

    virtual void setUserWnd(IUserWnd* wnd);
    virtual IUserWnd* userWnd() const;

    virtual void addMessageToView(const SIM::MessagePtr& message);
    virtual int messagesCount() const;

protected:
    virtual IUserWnd* createUserWnd(int id);

private:
    IUserWnd* m_userWnd;
    int m_id;
};

#endif // STANDARDUSERWNDCONTROLLER_H
