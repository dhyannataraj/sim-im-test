/*
 * servicesnacservicerequest.h
 *
 *  Created on: Jul 13, 2011
 */

#ifndef SERVICESNACSERVICEREQUEST_H_
#define SERVICESNACSERVICEREQUEST_H_

#include "requests/icqrequest.h"
#include "icq_defines.h"

class ICQClient;
class ICQ_EXPORT ServiceSnacServiceRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client, int serviceId);
    virtual ~ServiceSnacServiceRequest();

    virtual void perform(OscarSocket* socket);

private:
    ServiceSnacServiceRequest(ICQClient* client, int serviceId);

    ICQClient* m_client;
    int m_serviceId;
};

#endif /* SERVICESNACSERVICEREQUEST_H_ */
