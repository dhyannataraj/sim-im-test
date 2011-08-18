#ifndef GENERICMESSAGE_H
#define GENERICMESSAGE_H

#include "contacts/imcontact.h"
#include "message.h"

namespace SIM {

class EXPORT GenericMessage : public Message
{
public:
    GenericMessage(const IMContactPtr& from, const IMContactPtr& to, const QString& htmlText);
    GenericMessage(const QString& fromName, const QString& toName, const QString& clientId, const QString& htmlText);
    virtual ~GenericMessage();

    virtual QString type() const;

    virtual Client* client();
    virtual IMContactWeakPtr targetContact() const;
    virtual IMContactWeakPtr sourceContact() const;

    virtual QString targetContactName() const;
    virtual QString sourceContactName() const;

    virtual QIcon icon();
    virtual QDateTime timestamp();
    virtual QString toXml();
    virtual QString toPlainText();
    virtual QString originatingClientId() const;
    virtual QStringList choices();

    void setHtml(const QString& html);

private:
    Client* m_client;
    IMContactWeakPtr m_targetContact;
    IMContactWeakPtr m_sourceContact;
    QString m_targetContactName;
    QString m_sourceContactName;
    QString m_originatingClientId;
    QString m_text;
};

} // namespace SIM

#endif // GENERICMESSAGE_H
