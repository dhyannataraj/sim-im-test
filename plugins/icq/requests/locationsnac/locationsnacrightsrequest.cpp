/*
 * snaclocationrightsrequest.cpp
 *
 *  Created on: Jul 11, 2011
 */

#include "locationsnacrightsrequest.h"
#include "../../icqclient.h"
#include "../../locationsnachandler.h"

ICQRequestPtr LocationSnacRightsRequest::create(ICQClient* client)
{
    return ICQRequestPtr(new LocationSnacRightsRequest(client));
}

LocationSnacRightsRequest::LocationSnacRightsRequest(ICQClient* client) : m_client(client)
{
}

LocationSnacRightsRequest::~LocationSnacRightsRequest()
{
}

void LocationSnacRightsRequest::perform()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(LocationSnacHandler::SnacId, LocationSnacHandler::SnacLocationRightsRequest, 0, QByteArray());
}
