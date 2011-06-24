#ifndef CONTAINERMANAGER_H
#define CONTAINERMANAGER_H

#include <QObject>
#include <QList>
#include "container.h"
#include "containercontroller.h"

class CorePlugin;
class ContainerManager : public QObject
{
    Q_OBJECT
public:
    explicit ContainerManager(CorePlugin* parent);

    bool init();
    void contactChatRequested(int contactId);

    enum ContainerMode
    {
        cmSimpleMode = 0,
        cmContactContainers = 1,
        cmGroupContainers = 2,
        cmOneContainer = 3
    };

    ContainerMode containerMode() const;
    void setContainerMode(ContainerMode mode);

signals:

public slots:

private:
    ContainerControllerPtr makeContainerController(int id);
    void addContainer(const ContainerControllerPtr& cont);
    int containerCount();
    ContainerControllerPtr containerController(int index);
    ContainerControllerPtr containerControllerById(int id);
    void removeContainer(int index);
    void removeContainerById(int id);

    QList<ContainerControllerPtr> m_containers;
    ContainerMode m_containerMode;
    CorePlugin* m_core;
};

#endif // CONTAINERMANAGER_H
