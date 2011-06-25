#ifndef MOCKUSERWNDCONTROLLER_H
#define MOCKUSERWNDCONTROLLER_H

#include "container/userwndcontroller.h"
#include <QSharedPointer>

namespace MockObjects
{
    class MockUserWndController : public UserWndController
    {
    public:
        MOCK_CONST_METHOD0(id, int());
        MOCK_METHOD1(setUserWnd, void(IUserWnd* wnd));
        MOCK_CONST_METHOD0(userWnd, IUserWnd*());

        MOCK_METHOD1(addMessageToView, void(const SIM::MessagePtr& message));
        MOCK_CONST_METHOD0(messagesCount, int());
    };
    typedef QSharedPointer<MockUserWndController> MockUserWndControllerPtr;
}

#endif // MOCKUSERWNDCONTROLLER_H
