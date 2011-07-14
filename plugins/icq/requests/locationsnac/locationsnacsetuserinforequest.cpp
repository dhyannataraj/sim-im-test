/*
 * locationsnacsetuserinforequest.cpp
 *
 *  Created on: Jul 12, 2011
 */

#include "locationsnacsetuserinforequest.h"
#include "../../icqclient.h"
#include "../../tlvlist.h"

ICQRequestPtr LocationSnacSetUserInfoRequest::create(ICQClient* client, const QByteArray& caps)
{
    return ICQRequestPtr(new LocationSnacSetUserInfoRequest(client, caps));
}

LocationSnacSetUserInfoRequest::LocationSnacSetUserInfoRequest(ICQClient* client, const QByteArray& caps) : m_client(client), m_caps(caps)
{
}

LocationSnacSetUserInfoRequest::~LocationSnacSetUserInfoRequest()
{
}

void LocationSnacSetUserInfoRequest::perform(OscarSocket* socket)
{
    TlvList list;
    list.append(Tlv(LocationSnacHandler::TlvClientCapabilities, m_caps));

    socket->snac(LocationSnacHandler::SnacId, LocationSnacHandler::SnacSetUserInfo, 0, list.toByteArray());
}
