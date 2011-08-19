#include "icbmsnachandler.h"
#include "icqclient.h"
#include "bytearraybuilder.h"
#include "log.h"
#include "messaging/message.h"
#include "messaging/messagepipe.h"
#include "messaging/genericmessage.h"
#include "requests/icqrequestmanager.h"
#include "requests/icbmsnac/icbmsnacparametersrequest.h"
#include "requests/icbmsnac/icbmsnacsendparametersrequest.h"
#include "requests/icbmsnac/icbmsnacsendmessagerequest.h"

using SIM::log;
using SIM::L_DEBUG;
using SIM::L_WARN;

IcbmSnacHandler::IcbmSnacHandler(ICQClient* client) : SnacHandler(client, SnacId),
    m_ready(false), m_currentCookie(0)
{
}

bool IcbmSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    Q_UNUSED(flags);
    Q_UNUSED(requestId);
    switch(subtype)
    {
    case SnacIcbmParametersInfo:
        {
            if(!processParametersInfo(data))
                return false;
            m_channel = 0;
            m_params.messageFlags = 0x0b;
            m_params.maxSnacSize = 0x1f40;
            m_params.maxSenderWarnLevel = 0x30e7;
            m_params.maxReceiverWarnLevel = 0x30e7;

            m_ready = true;
            log(L_DEBUG, "ICBM snac ready");
            emit ready();

            return sendNewParametersInfo();
        }
        break;
    case SnacIcbmIncomingMessage:
        {
            return handleIncomingMessage(data);
        }
        break;
    default:
        log(L_WARN, "Unhandled icbm snac, subtype: %04x", subtype);
        return false;
    }

    return true;
}

void IcbmSnacHandler::disconnect()
{
    m_ready = false;
    m_currentCookie = 0;
}

void IcbmSnacHandler::requestParametersInfo()
{
    ICQRequestManager* manager = client()->requestManager();
    Q_ASSERT(manager);

    manager->enqueue(IcbmSnacParametersRequest::create(client()));
}

int IcbmSnacHandler::minMessageInterval() const
{
    return m_params.minMessageInterval;
}

void IcbmSnacHandler::forceReady()
{
    m_ready = true;
    emit ready();
}

bool IcbmSnacHandler::isReady() const
{
    return m_ready;
}

bool IcbmSnacHandler::sendMessage(const SIM::MessagePtr& message)
{
    ICQRequestPtr rq = IcbmSnacSendMessageRequest::create(client(), message);

    ICQRequestManager* manager = client()->requestManager();
    Q_ASSERT(manager);
    manager->enqueue(rq);
    return true;
}

bool IcbmSnacHandler::processParametersInfo(const QByteArray& arr)
{
    ByteArrayParser parser(arr);
    m_channel = parser.readWord();
    m_params.messageFlags = parser.readDword();
    m_params.maxSnacSize = parser.readWord();
    m_params.maxSenderWarnLevel = parser.readWord();
    m_params.maxReceiverWarnLevel = parser.readWord();
    m_params.minMessageInterval = parser.readDword();

    return true;
}

bool IcbmSnacHandler::sendNewParametersInfo()
{
    ICQRequestPtr rq = IcbmSnacSendParametersRequest::create(client(), m_channel, m_params);

    ICQRequestManager* manager = client()->requestManager();
    manager->enqueue(rq);
    return true;
}

QByteArray IcbmSnacHandler::generateCookie()
{
    QByteArray cookie((char*)&m_currentCookie, sizeof(m_currentCookie));
    m_currentCookie++;
    return cookie;
}

bool IcbmSnacHandler::handleIncomingTextMessage(const Tlv& messageTlv, const QByteArray& name)
{
    QByteArray messageBlockData = messageTlv.data();
    QString message = parseMessageBlock(messageBlockData);
    ICQContactList *contactList = m_client->contactList();
    ICQContactPtr sourceContact = contactList->contactByScreen(name);

    if(!sourceContact)
        return false;

    log(L_DEBUG, "handleIncomingMessage: %s/%x", name.data(), sourceContact.data());

    SIM::MessagePtr msg = SIM::MessagePtr(new SIM::GenericMessage(sourceContact, m_client->ownerContact(), message));
    SIM::getMessagePipe()->pushMessage(msg);

    return true;
}

bool IcbmSnacHandler::handleIncomingMessage(const QByteArray& data)
{
    ByteArrayParser parser(data);
    parser.readBytes(8); // Skip cookie
    parser.readWord(); // Channel

    int nameLength = parser.readByte();
    QByteArray name = parser.readBytes(nameLength);

    parser.readWord(); // Warning level
    parser.readWord(); // tlv count
    TlvList list = TlvList::fromByteArray(parser.readAll());

    Tlv messageTlv = list.firstTlv(TlvMessage);
    if(messageTlv.isValid())
    {
        return handleIncomingTextMessage(messageTlv, name);
    }

    return true;
}

QString IcbmSnacHandler::parseMessageBlock(const QByteArray& block)
{
    ByteArrayParser parser(block);
    while(!parser.atEnd())
    {
        int id = parser.readWord();
        int length = parser.readWord();
        if(id != MessageBlockInfo)
        {
            parser.readBytes(length);
        }
        else
        {
            parser.readWord(); // Charset
            parser.readWord(); // Subcharset
            QByteArray msgText = parser.readBytes(length - 4); // -4 for charset & subcharset fields
            return QString::fromUtf8(msgText.data(), msgText.size());
        }
    }
    return QString();
}
