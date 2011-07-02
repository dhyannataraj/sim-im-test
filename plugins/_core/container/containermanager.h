#ifndef CONTAINERMANAGER_H
#define CONTAINERMANAGER_H

#include <QObject>
#include <QList>
#include "container.h"
#include "icontainermanager.h"
#include "containercontroller.h"
#include "sendmessageprocessor.h"
#include "receivemessageprocessor.h"

class CorePlugin;
class ContainerManager : public QObject, public IContainerManager
{
    Q_OBJECT
public:
    explicit ContainerManager(CorePlugin* parent);
    virtual ~ContainerManager();

    virtual bool init();
    virtual void contactChatRequested(int contactId);

    virtual void messageSent(const SIM::MessagePtr& msg);
    virtual void messageReceived(const SIM::MessagePtr& msg);

    virtual ContainerMode containerMode() const;
    virtual void setContainerMode(ContainerMode mode);

signals:

public slots:
    void containerClosed(int id);

private:
    ContainerControllerPtr makeContainerController();
    void addContainer(const ContainerControllerPtr& cont);
    int containerCount();
    ContainerControllerPtr containerController(int index);
    ContainerControllerPtr containerControllerById(int id);
    void removeContainer(int index);
    void removeContainerById(int id);

    UserWndControllerPtr findUserWnd(int id);

    SendMessageProcessor* m_sendProcessor;
    ReceiveMessageProcessor* m_receiveProcessor;

    QList<ContainerControllerPtr> m_containers;
    int m_containerControllerId;
    ContainerMode m_containerMode;
    CorePlugin* m_core;
};

#endif // CONTAINERMANAGER_H
