/*
 * buddysnacrightsrequest.h
 *
 *  Created on: Jul 10, 2011
 *      Author: todin
 */

#ifndef BUDDYSNACRIGHTSREQUEST_H_
#define BUDDYSNACRIGHTSREQUEST_H_

#include <QSharedPointer>
#include "../../icq_defines.h"
#include "../icqrequest.h"


class ICQClient;
class ICQ_EXPORT BuddySnacRightsRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client);
    virtual ~BuddySnacRightsRequest();

    virtual void perform();

private:
    BuddySnacRightsRequest(ICQClient* client);
    ICQClient* m_client;
};

#endif /* BUDDYSNACRIGHTSREQUEST_H_ */
