/*
 * bartsnachandler.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include "bartsnachandler.h"
#include "icqclient.h"
#include "bytearraybuilder.h"

BartSnacHandler::BartSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_SSI)
{
}

BartSnacHandler::~BartSnacHandler()
{
}

bool BartSnacHandler::process(unsigned short subtype, const QByteArray & data, int flags, unsigned int requestId)
{
    switch(subtype)
    {
    case SnacResponseAvatar:
        return parseAvatarPacket(data);
    default:
        return false;
    }
    return true;
}

void BartSnacHandler::requestAvatar(const QString& screen, const QByteArray& hash)
{
    client()->oscarSocket()->snac(getType(), SnacRequestAvatar,
            0, makeRequestAvatarPacket(screen, hash));
}

QByteArray BartSnacHandler::makeRequestAvatarPacket(const QString& screen, const QByteArray& hash)
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

bool BartSnacHandler::parseAvatarPacket(const QByteArray& arr)
{
    ByteArrayParser parser(arr);
    int screenLength = parser.readByte();
    QByteArray screen = parser.readBytes(screenLength);
    parser.readWord(); // 0x0001
    parser.readByte(); // 0x01 ?
    int hashSize = parser.readByte();
    QByteArray hash = parser.readBytes(hashSize);
    int picSize = parser.readWord();
    QByteArray imgData = parser.readBytes(picSize);
    QImage image = QImage::fromData(imgData);

    ICQContactPtr contact = client()->contactList()->contactByScreen(QString(screen));
    if(!contact)
        return false;
    contact->setAvatar(image);

    return true;
}
