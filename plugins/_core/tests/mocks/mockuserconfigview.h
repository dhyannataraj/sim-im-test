/*
 * mockuserconfigview.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef MOCKUSERCONFIGVIEW_H_
#define MOCKUSERCONFIGVIEW_H_

#include "userconfig/usercfg.h"
#include <QSharedPointer>
#include "gmock/gmock.h"

namespace MockObjects
{
    class MockUserConfigView;
    typedef QSharedPointer<MockUserConfigView> MockUserConfigViewPtr;
    typedef QSharedPointer<testing::NiceMock<MockUserConfigView>> NiceMockUserConfigViewPtr;
    class MockUserConfigView : public UserConfig
    {
    public:
        virtual ~MockUserConfigView() {}
        static MockUserConfigViewPtr create() { return MockUserConfigViewPtr(new MockUserConfigView()); }
        static NiceMockUserConfigViewPtr createNice() { return NiceMockUserConfigViewPtr(new testing::NiceMock<MockUserConfigView>()); }

        MOCK_METHOD1(setWidgetHierarchy, void(SIM::WidgetHierarchy* hierarchy));
        MOCK_METHOD0(exec, int());
    };
}


#endif /* MOCKUSERCONFIGVIEW_H_ */
