#include <QTextDocument>

#include "genericmessage.h"
#include "contacts/client.h"

namespace SIM {

GenericMessage::GenericMessage(const IMContactPtr& from, const IMContactPtr& to, const QString& htmlText)
{
    m_targetContact = to.toWeakRef();
    m_sourceContact = from.toWeakRef();
    m_targetContactName = to->name();
    m_sourceContactName = from->name();
    m_client = to->client();
    if(m_client)
    {
        m_originatingClientId = m_client->name();
    }
    m_text = htmlText;
}

GenericMessage::GenericMessage(const QString& fromName, const QString& toName, const QString& clientId, const QString& htmlText)
{
    m_sourceContactName = fromName;
    m_targetContactName = toName;
    m_originatingClientId = clientId;
    m_text = htmlText;
}

GenericMessage::~GenericMessage()
{
}

QString GenericMessage::type() const
{
    return "generic";
}

QIcon GenericMessage::icon()
{
    return QIcon();
}

Client* GenericMessage::client()
{
    return m_client;
}

IMContactWeakPtr GenericMessage::targetContact() const
{
    return m_targetContact;
}

IMContactWeakPtr GenericMessage::sourceContact() const
{
    return m_sourceContact;
}

QString GenericMessage::targetContactName() const
{
    return m_targetContactName;
}

QString GenericMessage::sourceContactName() const
{
    return m_sourceContactName;
}

QDateTime GenericMessage::timestamp()
{
    return QDateTime();
}

QString GenericMessage::toXml()
{
    return m_text;
}

QString GenericMessage::toPlainText()
{
    QTextDocument doc;
    doc.setHtml(m_text);
    return doc.toPlainText();
}

QString GenericMessage::originatingClientId() const
{
    return m_originatingClientId;
}

QStringList GenericMessage::choices()
{
    return QStringList();
}

void GenericMessage::setHtml(const QString& html)
{
    m_text = html;
}

} // namespace SIM
