/*
 * bartsnacavatarrequest.cpp
 *
 *  Created on: Jul 12, 2011
 */

#include "bartsnacavatarrequest.h"
#include "../../icqclient.h"
#include "../../bartsnachandler.h"
#include "../../bytearraybuilder.h"

ICQRequestPtr BartSnacAvatarRequest::create(ICQClient* client, const QString& screen, const QByteArray& avatarHash)
{
    return ICQRequestPtr(new BartSnacAvatarRequest(client, screen, avatarHash));
}

BartSnacAvatarRequest::BartSnacAvatarRequest(ICQClient* client, const QString& screen, const QByteArray& avatarHash) :
        m_client(client), m_screen(screen), m_hash(avatarHash)
{
}

BartSnacAvatarRequest::~BartSnacAvatarRequest()
{
}

void BartSnacAvatarRequest::perform()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(BartSnacHandler::SnacId, BartSnacHandler::SnacRequestAvatar, 0, makeRequestAvatarPacket(m_screen, m_hash));
}

QByteArray BartSnacAvatarRequest::makeRequestAvatarPacket(const QString& screen, const QByteArray& hash)
{
    ByteArrayBuilder builder;
    builder.appendByte(screen.length());
    builder.appendBytes(screen.toAscii());
    builder.appendByte(0x01);
    builder.appendWord(0x0001);
    builder.appendByte(0x01);
    builder.appendByte(hash.length());
    builder.appendBytes(hash);
    return builder.getArray();
}
