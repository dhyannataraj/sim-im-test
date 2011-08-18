#ifndef STANDARDUSERWNDCONTROLLER_H
#define STANDARDUSERWNDCONTROLLER_H

#include "userwndcontroller.h"

class IUserWnd;
class IContainer;
class StandardUserWndController : public UserWndController
{
public:
    StandardUserWndController(int contactId);
    virtual ~StandardUserWndController();

    virtual int id() const;

    virtual void setUserWnd(IUserWnd* wnd);
    virtual IUserWnd* userWnd() const;

    virtual void setContainer(IContainer* cont);
    virtual IContainer* container() const;

    virtual void raise();

    virtual void addMessageToView(const SIM::MessagePtr& message);
    virtual int messagesCount() const;

    virtual void setMessageType(const QString& type);

protected:
    virtual IUserWnd* createUserWnd(int id);

private:
    IUserWnd* m_userWnd;
    IContainer* m_container;
    int m_id;
};

#endif // STANDARDUSERWNDCONTROLLER_H
