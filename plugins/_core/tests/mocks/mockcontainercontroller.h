/*
 * mockcontainercontroller.h
 *
 *  Created on: Aug 14, 2011
 */

#ifndef MOCKCONTAINERCONTROLLER_H_
#define MOCKCONTAINERCONTROLLER_H_

#include "container/icontainercontroller.h"

namespace MockObjects
{
    class MockContainerController : public IContainerController
    {
    public:
        virtual ~MockContainerController() {}
        MOCK_CONST_METHOD0(id, int());

        MOCK_METHOD1(sendMessage, void(const SIM::MessagePtr& msg));
        MOCK_METHOD1(addUserWnd, void(int contactId));
        MOCK_METHOD1(userWndById, IUserWnd*(int id));
        MOCK_METHOD1(userWndController, UserWndControllerPtr(int id));
        MOCK_METHOD1(raiseUserWnd, void(int id));
    };
}


#endif /* MOCKCONTAINERCONTROLLER_H_ */
