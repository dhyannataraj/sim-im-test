#include "icbmsnachandler.h"
#include "icqclient.h"
#include "bytearraybuilder.h"
#include "log.h"
#include "messaging/message.h"

using SIM::log;
using SIM::L_WARN;

IcbmSnacHandler::IcbmSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_MESSAGE),
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
            m_messageFlags = 0x0b;
            m_maxSnacSize = 0x1f40;
            m_maxSenderWarnLevel = 0x30e7;
            m_maxReceiverWarnLevel = 0x30e7;

            m_ready = true;
            emit ready();

            return sendNewParametersInfo();
        }
        break;
    default:
        log(L_WARN, "Unhandled icbm snac, subtype: %04x", subtype);
        return false;
    }

    return true;
}

void IcbmSnacHandler::requestParametersInfo()
{
    OscarSocket* socket = client()->oscarSocket();
    Q_ASSERT(socket);

    socket->snac(getType(), SnacIcbmParametersInfoRequest, 0, QByteArray());
}

int IcbmSnacHandler::minMessageInterval() const
{
    return m_minMessageInterval;
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
    ICQContactPtr contact = message->contact().toStrongRef().dynamicCast<ICQContact>();
    if(!contact)
    {
        log(L_WARN, "IcbmSnacHandler::sendMessage(): Unable to cast IMContact");
        return false;
    }

    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    QByteArray packet = makeSendPlainTextPacket(message);

    socket->snac(getType(), SnacIcbmSendMessage, 0, packet);
    return true;
}

QByteArray IcbmSnacHandler::makeSendPlainTextPacket(const SIM::MessagePtr& message)
{
    ICQContactPtr contact = message->contact().toStrongRef().dynamicCast<ICQContact>();
    // FIXME check
    Q_ASSERT(contact);
    ByteArrayBuilder builder;
    QByteArray cookie = generateCookie();
    builder.appendBytes(cookie);
    builder.appendWord(0x0001); // FIXME hardcoded const

    QByteArray contactId = contact->getScreen().toUtf8();
    builder.appendByte(contactId.length());
    builder.appendBytes(contactId);

    TlvList tlvs;
    tlvs.append(Tlv(TlvMessage, makeMessageTlv(message)));
    tlvs.append(Tlv(TlvServerAck, QByteArray()));
    tlvs.append(Tlv(TlvSendOffline, QByteArray()));

    builder.appendBytes(tlvs.toByteArray());
    return builder.getArray();
}

QByteArray IcbmSnacHandler::makeMessageTlv(const SIM::MessagePtr& message)
{
    ByteArrayBuilder builder;
    builder.appendWord(0x0501); // Features signature
    builder.appendWord(0x01); // Features length
    builder.appendByte(0x01); // Features
    builder.appendWord(0x0101); // Message info signature

    QByteArray messageText = message->toPlainText().toUtf8(); // FIXME ? encoding
    builder.appendWord(4 + messageText.length());
    builder.appendWord(0x0000); // Encoding set
    builder.appendWord(0xffff); // Encoding subset
    builder.appendBytes(messageText);
    return builder.getArray();
}

bool IcbmSnacHandler::processParametersInfo(const QByteArray& arr)
{
    ByteArrayParser parser(arr);
    m_channel = parser.readWord();
    m_messageFlags = parser.readDword();
    m_maxSnacSize = parser.readWord();
    m_maxSenderWarnLevel = parser.readWord();
    m_maxReceiverWarnLevel = parser.readWord();
    m_minMessageInterval = parser.readDword();

    return true;
}

bool IcbmSnacHandler::sendNewParametersInfo()
{
    OscarSocket* socket = m_client->oscarSocket();
    Q_ASSERT(socket);

    ByteArrayBuilder builder;

    builder.appendWord(m_channel);
    builder.appendDword(m_messageFlags);
    builder.appendWord(m_maxSnacSize);
    builder.appendWord(m_maxSenderWarnLevel);
    builder.appendWord(m_maxReceiverWarnLevel);
    builder.appendByte(m_minMessageInterval);

    socket->snac(getType(), SnacIcbmSetParameters, 0, builder.getArray());
    return true;
}

QByteArray IcbmSnacHandler::generateCookie()
{
    QByteArray cookie((char*)&m_currentCookie, sizeof(m_currentCookie));
    m_currentCookie++;
    return cookie;
}
