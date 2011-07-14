/*
 * icbmsnacsendmessagerequest.cpp
 *
 *  Created on: Jul 11, 2011
 */

#include "../../icqclient.h"
#include "../../tlvlist.h"
#include "icbmsnacsendmessagerequest.h"
#include "log.h"
#include "../../bytearraybuilder.h"

using SIM::log;
using SIM::L_WARN;

ICQRequestPtr IcbmSnacSendMessageRequest::create(ICQClient* client, const SIM::MessagePtr& msg)
{
    return ICQRequestPtr(new IcbmSnacSendMessageRequest(client, msg));
}

IcbmSnacSendMessageRequest::IcbmSnacSendMessageRequest(ICQClient* client, const SIM::MessagePtr& msg) : m_client(client), m_message(msg)
{
}

IcbmSnacSendMessageRequest::~IcbmSnacSendMessageRequest()
{
}

void IcbmSnacSendMessageRequest::perform(OscarSocket* socket)
{
    ICQContactPtr contact = m_message->targetContact().toStrongRef().dynamicCast<ICQContact>();
    if(!contact)
    {
        log(L_WARN, "IcbmSnacHandler::sendMessage(): Unable to cast IMContact");
        return; // TODO signal error
    }

    QByteArray packet = makeSendPlainTextPacket(m_message);

    socket->snac(IcbmSnacHandler::SnacId, IcbmSnacHandler::SnacIcbmSendMessage, 0, packet);
}

QByteArray IcbmSnacSendMessageRequest::makeSendPlainTextPacket(const SIM::MessagePtr& message)
{
    ICQContactPtr contact = message->targetContact().toStrongRef().dynamicCast<ICQContact>();
    // FIXME check
    Q_ASSERT(contact);
    ByteArrayBuilder builder;
    QByteArray cookie = QByteArray(8, 0); // FIXME
    builder.appendBytes(cookie);
    builder.appendWord(0x0001); // FIXME hardcoded const

    QByteArray contactId = contact->getScreen().toUtf8();
    builder.appendByte(contactId.length());
    builder.appendBytes(contactId);

    TlvList tlvs;
    tlvs.append(Tlv(IcbmSnacHandler::TlvMessage, makeMessageTlv(message)));
    tlvs.append(Tlv(IcbmSnacHandler::TlvServerAck, QByteArray()));
    tlvs.append(Tlv(IcbmSnacHandler::TlvSendOffline, QByteArray()));

    builder.appendBytes(tlvs.toByteArray());
    return builder.getArray();
}

QByteArray IcbmSnacSendMessageRequest::makeMessageTlv(const SIM::MessagePtr& message)
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
