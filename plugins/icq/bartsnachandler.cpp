/*
 * bartsnachandler.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include "bartsnachandler.h"
#include "icqclient.h"
#include "bytearraybuilder.h"
#include "requests/standardicqrequestmanager.h"
#include "requests/bartsnac/bartsnacavatarrequest.h"
#include "requests/servicesnac/servicesnacservicerequest.h"
#include "standardoscarsocket.h"
#include "log.h"

using SIM::log;
using SIM::L_DEBUG;
using SIM::L_WARN;
using SIM::L_ERROR;

BartSnacHandler::BartSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_SSI)
{
    m_socket = new StandardOscarSocket();
    connect(m_socket, SIGNAL(packet(int,QByteArray)), this, SLOT(oscarSocketPacket(int,QByteArray)));
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));

    m_requestManager = new StandardICQRequestManager();
    m_requestManager->setOscarSocket(m_socket);
}

BartSnacHandler::~BartSnacHandler()
{
    if(m_socket)
        delete m_socket;
    delete m_requestManager;
}

bool BartSnacHandler::process(unsigned short subtype, const QByteArray & data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacResponseAvatar:
        return parseAvatarPacket(data);
    default:
        break;
    }
    return true;
}

bool BartSnacHandler::processService(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case ServiceSnacHandler::SnacServiceServerReady:
        return parseServerReady(data);
    case ServiceSnacHandler::SnacServiceCapabilitiesAck:
        requestRateInfo();
        break;
    case ServiceSnacHandler::SnacServiceRateInfo:
        ackRateInfo();
        sendReady();
        break;
    default:
        break;
    }
    return true;
}

void BartSnacHandler::requestAvatar(const QString& screen, const QByteArray& hash)
{
    m_requestManager->enqueue(BartSnacAvatarRequest::create(client(), screen, hash));
}

void BartSnacHandler::requestBartService()
{
    ICQRequestPtr req = ServiceSnacServiceRequest::create(client(), SnacId);
    client()->requestManager()->enqueue(req);
}

void BartSnacHandler::setOscarSocket(OscarSocket* socket)
{
    if(m_socket)
        delete m_socket;
    m_socket = socket;
    m_requestManager->setOscarSocket(m_socket);
    connect(m_socket, SIGNAL(packet(int,QByteArray)), this, SLOT(oscarSocketPacket(int,QByteArray)));
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
}

OscarSocket* BartSnacHandler::oscarSocket() const
{
    return m_socket;
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

void BartSnacHandler::bartServiceAvailable(const QString& address, const QByteArray& cookie)
{
    log(L_DEBUG, "BartSnacHandler::bartServiceAvailable");
    QString host;
    int port = 5190;  // FIXME hardcoded const
    QStringList addr = address.split(':');
    if(addr.size() == 2)
    {
        host = addr.at(0);
        port = addr.at(1).toUInt();
    }
    else if(addr.size() == 1)
    {
        host = addr.at(0);
    }
    else if(addr.size() > 2)
        return;

    if(addr.isEmpty())
        return;

    m_loginCookie = cookie;

    m_socket->connectToHost(host, port);

}

void BartSnacHandler::sendLoginCookie(const QByteArray& cookie)
{
    ByteArrayBuilder builder;
    builder.appendDword(0x00000001);

    TlvList tlvs;
    tlvs.append(Tlv(ServiceSnacHandler::TlvAuthCookie, cookie));

    builder.appendBytes(tlvs.toByteArray());

    m_socket->flap(0x01, builder.getArray());
}

void BartSnacHandler::oscarSocketPacket(int channel, const QByteArray& data)
{
    if((channel == ICQ_CHNxNEW) || (channel == ICQ_CHNxCLOSE))
        handleLoginAndCloseChannels(channel, data);
    else if (channel == ICQ_CHNxDATA)
    {
        ByteArrayParser parser(data);
        int food = parser.readWord();
        int type = parser.readWord();
        int flags = parser.readWord();
        unsigned int requestId = parser.readDword();
        QByteArray snacData = parser.readAll();
        if(food == SnacId)
        {
            process(type, snacData, flags, requestId);
        }
        else if(food == 0x01)
        {
            processService(type, snacData, flags, requestId);
        }
        else
        {
            log(L_WARN, "BART: Unknown foodgroup %04X", food);
        }
    }
    else
        log(L_ERROR, "BART: Unknown channel %u", channel & 0xFF);
}

void BartSnacHandler::socketConnected()
{
    sendLoginCookie(m_loginCookie);
}

void BartSnacHandler::handleLoginAndCloseChannels(int channel, const QByteArray& data)
{
}

ICQRequestManager* BartSnacHandler::requestManager() const
{
    return m_requestManager;
}

bool BartSnacHandler::parseServerReady(const QByteArray& data)
{
    Q_UNUSED(data);
    sendCapabilities();
    return true;
}

void BartSnacHandler::sendCapabilities()
{
    ByteArrayBuilder builder;
    builder.appendWord(SnacId);
    builder.appendWord(0x01);
    builder.appendWord(ServiceSnacHandler::SnacId);
    builder.appendWord(0x03);
    m_socket->snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceCapabilities, 0, builder.getArray());
}

void BartSnacHandler::requestRateInfo()
{
    m_socket->snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRateInfoRequest, 0, QByteArray());
}

void BartSnacHandler::ackRateInfo()
{
    ByteArrayBuilder builder;
    for(int i = 0; i < 5; i++)
    {
        builder.appendWord(i + 1);
    }

    m_socket->snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceRateInfoAck, 0, builder.getArray());
}

void BartSnacHandler::sendReady()
{
    ByteArrayBuilder builder;
    builder.appendWord(SnacId);
    builder.appendWord(0x01);
    builder.appendDword(0x100629);
    builder.appendWord(ServiceSnacHandler::SnacId);
    builder.appendWord(0x03);
    builder.appendDword(0x01100629);

    m_socket->snac(ServiceSnacHandler::SnacId, ServiceSnacHandler::SnacServiceReady, 0, builder.getArray());
}
