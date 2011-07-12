/*
 * buddysnacrightsrequest.cpp
 *
 *  Created on: Jul 10, 2011
 *      Author: todin
 */

#include "buddysnacrightsrequest.h"
#include "../../buddysnachandler.h"
#include "../../oscarsocket.h"
#include "../../icqclient.h"

ICQRequestPtr BuddySnacRightsRequest::create(ICQClient* client)
{
    return ICQRequestPtr(new BuddySnacRightsRequest(client));
}

BuddySnacRightsRequest::BuddySnacRightsRequest(ICQClient* client) : m_client(client)
{
}

BuddySnacRightsRequest::~BuddySnacRightsRequest()
{
}

void BuddySnacRightsRequest::perform()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(BuddySnacHandler::SnacId, BuddySnacHandler::SnacBuddyRightsRequest, 0, QByteArray());
}



