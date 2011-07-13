/*
 * bartsnachandler.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include "bartsnachandler.h"
#include "icqclient.h"
#include "bytearraybuilder.h"
#include "requests/bartsnac/bartsnacavatarrequest.h"
#include "standardoscarsocket.h"

BartSnacHandler::BartSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_SSI)
{
    m_socket = new StandardOscarSocket();
}

BartSnacHandler::~BartSnacHandler()
{
    delete m_socket;
}

bool BartSnacHandler::process(unsigned short subtype, const QByteArray & data, int flags, unsigned int requestId)
{
    switch(subtype)
    {
    case SnacResponseAvatar:
        return parseAvatarPacket(data);
    default:
        break;
    }
    return true;
}

void BartSnacHandler::requestAvatar(const QString& screen, const QByteArray& hash)
{
    ICQRequestManager* manager = client()->requestManager();
    Q_ASSERT(manager);

    manager->enqueue(BartSnacAvatarRequest::create(client(), screen, hash));
}

void BartSnacHandler::requestBartService()
{

}

void BartSnacHandler::setOscarSocket(OscarSocket* socket)
{
    if(m_socket)
        delete m_socket;
    m_socket = socket;
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
