/*
 * icbmsnacsendparameterrequest.cpp
 *
 *  Created on: Jul 10, 2011
 */

#include "icbmsnacsendparametersrequest.h"
#include "../../bytearraybuilder.h"
#include "../../icqclient.h"
#include "../../oscarsocket.h"

ICQRequestPtr IcbmSnacSendParametersRequest::create(ICQClient* client, int channel, const IcbmParameters& newParameters)
{
    IcbmSnacSendParametersRequest* request = new IcbmSnacSendParametersRequest(client);
    request->m_channel = channel;
    request->m_param = newParameters;
    return ICQRequestPtr(request);
}

IcbmSnacSendParametersRequest::IcbmSnacSendParametersRequest(ICQClient* client) : m_client(client)
{

}

IcbmSnacSendParametersRequest::~IcbmSnacSendParametersRequest()
{
}

void IcbmSnacSendParametersRequest::perform()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    ByteArrayBuilder builder;

    builder.appendWord(m_channel);
    builder.appendDword(m_param.messageFlags);
    builder.appendWord(m_param.maxSnacSize);
    builder.appendWord(m_param.maxSenderWarnLevel);
    builder.appendWord(m_param.maxReceiverWarnLevel);
    builder.appendDword(m_param.minMessageInterval);

    socket->snac(IcbmSnacHandler::SnacId, IcbmSnacHandler::SnacIcbmSetParameters, 0, builder.getArray());
}

