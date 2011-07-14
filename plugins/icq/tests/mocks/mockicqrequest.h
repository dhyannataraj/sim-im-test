/*
 * mockicqrequest.h
 *
 *  Created on: Jul 10, 2011
 */

#ifndef MOCKICQREQUEST_H_
#define MOCKICQREQUEST_H_

#include "gmock/gmock.h"
#include "requests/icqrequest.h"
#include <QSharedPointer>
#include "oscarsocket.h"

namespace MockObjects
{
    class MockICQRequest;
    typedef QSharedPointer<MockICQRequest> MockICQRequestPtr;
    class MockICQRequest : public ICQRequest
    {
    public:
        virtual ~MockICQRequest() {}

        MOCK_METHOD1(perform, void(OscarSocket* socket));

        static MockICQRequestPtr create() { return MockICQRequestPtr(new MockICQRequest()); }
    };


}

#endif /* MOCKICQREQUEST_H_ */
