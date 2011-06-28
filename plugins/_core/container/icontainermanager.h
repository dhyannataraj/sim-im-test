/*
 * icontainermanager.h
 *
 *  Created on: Jun 28, 2011
 *      Author: todin
 */

#ifndef ICONTAINERMANAGER_H_
#define ICONTAINERMANAGER_H_

#include "messaging/message.h"
#include "core_api.h"

class CORE_EXPORT IContainerManager
{
public:
    virtual ~IContainerManager();

    virtual bool init() = 0;
    virtual void contactChatRequested(int contactId) = 0;

    virtual void messageSent(const SIM::MessagePtr& msg) = 0;

    enum ContainerMode
    {
        cmSimpleMode = 0,
        cmContactContainers = 1,
        cmGroupContainers = 2,
        cmOneContainer = 3
    };

    virtual ContainerMode containerMode() const = 0;
    virtual void setContainerMode(ContainerMode mode) = 0;
};

#endif /* ICONTAINERMANAGER_H_ */
