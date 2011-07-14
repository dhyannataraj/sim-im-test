/*
 * servicesnacservicerequest.cpp
 *
 *  Created on: Jul 13, 2011
 */

#include "servicesnacservicerequest.h"
#include "icqclient.h"
#include "bytearraybuilder.h"

ICQRequestPtr ServiceSnacServiceRequest::create(ICQClient* client, int serviceId)
{
    return ICQRequestPtr(new ServiceSnacServiceRequest(client, serviceId));
}

ServiceSnacServiceRequest::ServiceSnacServiceRequest(ICQClient* client, int serviceId) :
    m_client(client), m_serviceId(serviceId)
{
}

ServiceSnacServiceRequest::~ServiceSnacServiceRequest()
{
}

void ServiceSnacServiceRequest::perform(OscarSocket* socket)
{
    ByteArrayBuilder builder;
    builder.appendWord(m_serviceId);

    socket->snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRequestService, 0, builder.getArray());
}
