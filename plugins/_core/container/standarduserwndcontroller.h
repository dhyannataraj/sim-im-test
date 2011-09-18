#ifndef STANDARDUSERWNDCONTROLLER_H
#define STANDARDUSERWNDCONTROLLER_H

#include <QObject>
#include "userwndcontroller.h"
#include "contacts/imcontact.h"

class IUserWnd;
class IContainer;
class StandardUserWndController : public QObject, public UserWndController
{
    Q_OBJECT
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

signals:
    void messageSendRequest(const SIM::MessagePtr& message);

protected slots:
    void slot_messageSendRequest(const SIM::MessagePtr& message);

protected:
    virtual IUserWnd* createUserWnd(int id);

    SIM::IMContactPtr targetContact() const;
    SIM::IMContactPtr sourceContact() const;

private:
    IUserWnd* m_userWnd;
    IContainer* m_container;
    int m_id;
};

#endif // STANDARDUSERWNDCONTROLLER_H
