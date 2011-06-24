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

ContainerPtr ContainerManager::makeContainer(int id)
{
    return ContainerPtr(new Container(id));
}

bool ContainerManager::init()
{
    setContainerMode((ContainerMode)SIM::ProfileManager::instance()->getPropertyHub("_core")->value("ContainerMode").toUInt());
    return true;
}


void ContainerManager::addContainer(const ContainerPtr& cont)
{
    m_containers.append(cont);
}

int ContainerManager::containerCount()
{
    return m_containers.count();
}

ContainerPtr ContainerManager::container(int index)
{
    // Check boundaries ?
    return m_containers.at(index);
}

ContainerPtr ContainerManager::containerById(int id)
{
    foreach(const ContainerPtr& c, m_containers) {
        if(c->id() == id)
            return c;
    }
    return ContainerPtr();
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
    foreach(const ContainerPtr& c, m_containers) {
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

    ContainerPtr container = containerById(0);
    if(!container)
    {
        container = makeContainer(0);
        addContainer(container);
    }

    IUserWnd* userwnd = container->wnd(contactId);
    if(!userwnd)
    {
        userwnd = new UserWnd(contactId, false, false);
        container->addUserWnd(userwnd);
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
