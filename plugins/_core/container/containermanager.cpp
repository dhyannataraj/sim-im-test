#include "containermanager.h"
#include "core.h"
#include "profilemanager.h"
#include "log.h"
#include "userwnd.h"

using SIM::log;
using SIM::L_DEBUG;

ContainerManager::ContainerManager(CorePlugin* parent) :
    QObject(parent), m_core(parent)
{
}

ContainerControllerPtr ContainerManager::makeContainerController(int id)
{
    return ContainerControllerPtr(new ContainerController(id));
}

bool ContainerManager::init()
{
    setContainerMode((ContainerMode)SIM::ProfileManager::instance()->getPropertyHub("_core")->value("ContainerMode").toUInt());
    return true;
}

void ContainerManager::addContainer(const ContainerControllerPtr& cont)
{
    m_containers.append(cont);
}

int ContainerManager::containerCount()
{
    return m_containers.count();
}

ContainerControllerPtr ContainerManager::containerController(int index)
{
    // FIXME Check boundaries ?
    return m_containers.at(index);
}

ContainerControllerPtr ContainerManager::containerControllerById(int id)
{
    foreach(const ContainerControllerPtr& c, m_containers) {
        if(c->id() == id)
            return c;
    }
    return ContainerControllerPtr();
}

void ContainerManager::removeContainer(int index)
{
    if((index < 0) || (index >= containerCount()))
        return;
    m_containers.removeAt(index);
}

void ContainerManager::removeContainerById(int id)
{
    int index = 0;
    foreach(const ContainerControllerPtr& c, m_containers) {
        if(c->id() == id) {
            m_containers.removeAt(index);
            return;
        }
        index++;
    }
}

void ContainerManager::contactChatRequested(int contactId)
{
    log(L_DEBUG, "contactChatRequested: %d", contactId);

    ContainerControllerPtr container = containerControllerById(0);
    if(!container)
    {
        container = makeContainerController(0);
        addContainer(container);
    }

    IUserWnd* userwnd = container->userWndById(contactId);
    if(!userwnd)
    {
        container->addUserWnd(contactId);
    }
}

ContainerManager::ContainerMode ContainerManager::containerMode() const
{
    return m_containerMode;
}

void ContainerManager::setContainerMode(ContainerManager::ContainerMode mode)
{
    m_containerMode = mode;
}
