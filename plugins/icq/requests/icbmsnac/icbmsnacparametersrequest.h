/*
 * icbmsnacparametersrequest.h
 *
 *  Created on: Jul 10, 2011
 */

#ifndef ICBMSNACPARAMETERSREQUEST_H_
#define ICBMSNACPARAMETERSREQUEST_H_

#include "../icqrequest.h"
#include "../../icq_defines.h"

class ICQClient;
class ICQ_EXPORT IcbmSnacParametersRequest: public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client);
    virtual ~IcbmSnacParametersRequest();

    virtual void perform();

private:
    IcbmSnacParametersRequest(ICQClient* client);
    ICQClient* m_client;
};

#endif /* ICBMSNACPARAMETERSREQUEST_H_ */
