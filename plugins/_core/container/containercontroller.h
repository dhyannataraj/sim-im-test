#ifndef CONTAINERCONTROLLER_H
#define CONTAINERCONTROLLER_H

#include <QObject>
#include "messaging/message.h"
#include "messageprocessors/containersentmessageprocessor.h"
#include "userwndcontroller.h"
#include "core_api.h"

class IContainer;
class CORE_EXPORT ContainerController : public QObject
{
    Q_OBJECT
public:
    explicit ContainerController(IContainer* view, int id);
    virtual ~ContainerController();

    int id() const;

    void sendMessage(const SIM::MessagePtr& msg);
    void addUserWnd(UserWnd* wnd);
    UserWnd* userWndById(int id);

signals:

public slots:

private:
    QList<UserWndControllerPtr> m_controllers;
    ContainerSentMessageProcessor* m_sentMessage;
    IContainer* m_view;
    int m_id;
};

#endif // CONTAINERCONTROLLER_H
