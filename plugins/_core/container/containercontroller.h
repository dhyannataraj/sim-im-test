#ifndef CONTAINERCONTROLLER_H
#define CONTAINERCONTROLLER_H

#include <QObject>
#include "messaging/message.h"
#include "userwndcontroller.h"
#include "core_api.h"
#include "icontainer.h"

class CORE_EXPORT ContainerController : public QObject
{
    Q_OBJECT
public:
    explicit ContainerController(int id);
    virtual ~ContainerController();

    int id() const;

    void sendMessage(const SIM::MessagePtr& msg);
    void addUserWnd(int contactId);
    IUserWnd* userWndById(int id);
    UserWndControllerPtr userWndController(int id);

signals:
    void closed(int id);

public slots:
    void containerClosed();

protected:
    virtual UserWndControllerPtr createUserWndController(int id);

private:
    QList<UserWndControllerPtr> m_controllers;
    IContainerPtr m_view;
    int m_id;
};
typedef QSharedPointer<ContainerController> ContainerControllerPtr;

#endif // CONTAINERCONTROLLER_H
