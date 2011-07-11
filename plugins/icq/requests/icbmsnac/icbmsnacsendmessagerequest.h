/*
 * icbmsnacsendmessagerequest.h
 *
 *  Created on: Jul 11, 2011
 */

#ifndef ICBMSNACSENDMESSAGEREQUEST_H_
#define ICBMSNACSENDMESSAGEREQUEST_H_

#include "requests/icqrequest.h"
#include "messaging/message.h"

class ICQClient;
class IcbmSnacSendMessageRequest : public ICQRequest
{
public:
    static ICQRequestPtr create(ICQClient* client, const SIM::MessagePtr& msg);
    virtual ~IcbmSnacSendMessageRequest();
    virtual void perform();

private:
    IcbmSnacSendMessageRequest(ICQClient* client, const SIM::MessagePtr& msg);

    QByteArray makeSendPlainTextPacket(const SIM::MessagePtr & message);
    QByteArray makeMessageTlv(const SIM::MessagePtr & message);

    ICQClient* m_client;
    SIM::MessagePtr m_message;
};

#endif /* ICBMSNACSENDMESSAGEREQUEST_H_ */
