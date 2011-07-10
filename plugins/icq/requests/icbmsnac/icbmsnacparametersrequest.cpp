/*
 * icbmsnacparametersrequest.cpp
 *
 *  Created on: Jul 10, 2011
 */

#include "icbmsnacparametersrequest.h"
#include "icqclient.h"
#include "oscarsocket.h"

ICQRequestPtr IcbmSnacParametersRequest::create(ICQClient* client)
{
    return ICQRequestPtr(new IcbmSnacParametersRequest(client));
}

IcbmSnacParametersRequest::IcbmSnacParametersRequest(ICQClient* client) : m_client(client)
{
}

IcbmSnacParametersRequest::~IcbmSnacParametersRequest()
{
}

void IcbmSnacParametersRequest::perform()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(IcbmSnacHandler::SnacId, IcbmSnacHandler::SnacIcbmParametersInfoRequest, 0, QByteArray());
}

