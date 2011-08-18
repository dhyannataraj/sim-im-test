/*
 * mockuserwnd.h
 *
 *  Created on: Aug 14, 2011
 */

#ifndef MOCKUSERWND_H_
#define MOCKUSERWND_H_

#include "container/iuserwnd.h"

namespace MockObjects
{
    class MockUserWnd : public IUserWnd
    {
        virtual ~MockUserWnd() {}
        MOCK_METHOD1(addMessageToView, void(const SIM::MessagePtr& message));
        MOCK_CONST_METHOD0(messagesInViewArea, int());

        MOCK_CONST_METHOD0(id, int());
        MOCK_METHOD0(getName, QString());

        MOCK_METHOD1(setMessageEditor, void(SIM::MessageEditor* editor));
        MOCK_CONST_METHOD0(selectedClientId, QString());
    };
}


#endif /* MOCKUSERWND_H_ */
