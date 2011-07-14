/*
 * mockicqrequestmanager.h
 *
 *  Created on: Jul 13, 2011
 */

#ifndef MOCKICQREQUESTMANAGER_H_
#define MOCKICQREQUESTMANAGER_H_

#include "requests/icqrequestmanager.h"
#include "gmock/gmock.h"

namespace MockObjects
{
    class MockICQRequestManager : public ICQRequestManager
    {
    public:
        MOCK_METHOD1(enqueue, void(const ICQRequestPtr& request));
    };
}

#endif /* MOCKICQREQUESTMANAGER_H_ */
