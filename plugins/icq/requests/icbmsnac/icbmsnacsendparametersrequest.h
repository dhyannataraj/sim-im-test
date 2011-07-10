/*
 * icbmsnacsendparameterrequest.h
 *
 *  Created on: Jul 10, 2011
 */

#ifndef ICBMSNACSENDPARAMETERSREQUEST_H_
#define ICBMSNACSENDPARAMETERSREQUEST_H_

#include "requests/icqrequest.h"
#include "icq_defines.h"

class ICQClient;
class ICQ_EXPORT IcbmSnacSendParametersRequest: public ICQRequest
{
public:
    struct IcbmParameters
    {
        unsigned int messageFlags;
        int maxSnacSize;
        int maxSenderWarnLevel;
        int maxReceiverWarnLevel;
        int minMessageInterval;
    };

    static ICQRequestPtr create(ICQClient* client, int channel, const IcbmParameters& newParameters);
    virtual ~IcbmSnacSendParametersRequest();

    virtual void perform();

private:
    IcbmSnacSendParametersRequest(ICQClient* client);
    ICQClient* m_client;
    int m_channel;
    IcbmParameters m_param;
};

#endif /* ICBMSNACSENDPARAMETERREQUEST_H_ */
