/*
 * icbmsnacsendparameterrequest.h
 *
 *  Created on: Jul 10, 2011
 */

#ifndef ICBMSNACSENDPARAMETERSREQUEST_H_
#define ICBMSNACSENDPARAMETERSREQUEST_H_

#include "../icqrequest.h"
#include "../../icq_defines.h"
#include "icbmsnachandler.h"

class ICQClient;
class ICQ_EXPORT IcbmSnacSendParametersRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client, int channel, const IcbmSnacHandler::IcbmParameters& newParameters);
    virtual ~IcbmSnacSendParametersRequest();

    virtual void perform();

private:
    IcbmSnacSendParametersRequest(ICQClient* client);
    ICQClient* m_client;
    int m_channel;
    IcbmSnacHandler::IcbmParameters m_param;
};

#endif /* ICBMSNACSENDPARAMETERREQUEST_H_ */
