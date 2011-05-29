#ifndef GENERICMESSAGE_H
#define GENERICMESSAGE_H

#include "contacts/imcontact.h"
#include "message.h"

namespace SIM {

class EXPORT GenericMessage : public Message
{
public:
    GenericMessage(const IMContactPtr& contact, const QString& htmlText);
    virtual ~GenericMessage();

    virtual Client* client();
    virtual IMContactWeakPtr contact() const;

    virtual QIcon icon();
    virtual QDateTime timestamp();
    virtual QString toXml();
    virtual QString toPlainText();
    virtual QString originatingClientId() const;
    virtual QStringList choices();

    void setHtml(const QString& html);

private:
    Client* m_client;
    IMContactWeakPtr m_contact;
    QString m_originatingClientId;
    QString m_text;
};

} // namespace SIM

#endif // GENERICMESSAGE_H
