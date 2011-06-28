/*
 * mockcontainermanager.h
 *
 *  Created on: Jun 28, 2011
 *      Author: todin
 */

#ifndef MOCKCONTAINERMANAGER_H_
#define MOCKCONTAINERMANAGER_H_

#include "gmock/gmock.h"
#include "container/icontainermanager.h"

namespace MockObjects
{
    class MockContainerManager : public IContainerManager
    {
    public:
        virtual ~MockContainerManager() {}
        MOCK_METHOD0(init, bool());
        MOCK_METHOD1(contactChatRequested, void(int contactId));
        MOCK_METHOD1(messageSent, void(const SIM::MessagePtr& msg));
        MOCK_CONST_METHOD0(containerMode, ContainerMode());
        MOCK_METHOD1(setContainerMode, void(ContainerMode mode));
    };
}

#endif /* MOCKCONTAINERMANAGER_H_ */
