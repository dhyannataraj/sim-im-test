/*
 * standardicqrequestmanager.h
 *
 *  Created on: Jul 10, 2011
 */

#ifndef STANDARDICQREQUESTMANAGER_H_
#define STANDARDICQREQUESTMANAGER_H_

#include "icqrequestmanager.h"
#include "icq_defines.h"

class ICQClient;
class ICQ_EXPORT StandardICQRequestManager : public ICQRequestManager
{
public:
    StandardICQRequestManager(ICQClient* client);
    virtual ~StandardICQRequestManager();

    virtual void enqueue(const ICQRequestPtr& request);

private:
    ICQClient* m_client;
};

#endif /* STANDARDICQREQUESTMANAGER_H_ */
