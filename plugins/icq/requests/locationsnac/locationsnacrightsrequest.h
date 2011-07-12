/*
 * snaclocationrightsrequest.h
 *
 *  Created on: Jul 11, 2011
 */

#ifndef SNACLOCATIONRIGHTSREQUEST_H_
#define SNACLOCATIONRIGHTSREQUEST_H_

#include "requests/icqrequest.h"

class ICQClient;
class LocationSnacRightsRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client);
    virtual ~LocationSnacRightsRequest();

    virtual void perform();

private:
    LocationSnacRightsRequest(ICQClient* client);

    ICQClient* m_client;
};

#endif /* SNACLOCATIONRIGHTSREQUEST_H_ */
