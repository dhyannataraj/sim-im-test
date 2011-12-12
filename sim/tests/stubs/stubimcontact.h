#ifndef STUBIMCONTACT_H
#define STUBIMCONTACT_H

#include "contacts/imcontact.h"

namespace StubObjects
{
class StubIMContact : public SIM::IMContact
{
public:
    StubIMContact(SIM::Client* client) : m_client(client) {}

    virtual SIM::Client* client()
    {
        return m_client;
    }

    virtual void setParentContactId(int contactId)
    {
        Q_UNUSED(contactId);
    }

    virtual int parentContactId() const
    {
        return 0;
    }

    virtual SIM::IMContactId id() const
    {
        return SIM::IMContactId();
    }

    virtual SIM::IMStatusPtr status() const
    {
        return SIM::IMStatusPtr();
    }

    virtual bool sendMessage(const SIM::MessagePtr& message)
    {
        Q_UNUSED(message);
        return true;
    }

    QString name() const
    {
        return "StubContact";
    }

    virtual bool hasUnreadMessages()
    {
        return false;
    }

    virtual SIM::MessagePtr dequeueUnreadMessage()
    {
        return SIM::MessagePtr();
    }

    virtual void enqueueUnreadMessage(const SIM::MessagePtr& message)
    {
        Q_UNUSED(message);
    }

    virtual SIM::IMGroupWeakPtr group()
    {
        return SIM::IMGroupWeakPtr();
    }

    virtual QString makeToolTipText()
    {
        return QString();
    }

    virtual QList<SIM::MessageTypeDescriptor> allMessageTypes() const
    {
        return QList<SIM::MessageTypeDescriptor>();
    }


    virtual void serialize(QDomElement& element)
    {
        Q_UNUSED(element);
    }

    virtual void deserialize(QDomElement& element)
    {
        Q_UNUSED(element);
    }

    virtual bool deserialize(const QString& data)
    {
        Q_UNUSED(data);
        return false;
    }

		virtual bool loadState(SIM::PropertyHubPtr state)
		{
				Q_UNUSED(state);
				return false;
		}

		virtual SIM::PropertyHubPtr saveState()
		{
				return SIM::PropertyHubPtr();
		}

private:
    SIM::Client* m_client;
};
}

#endif // STUBIMCONTACT_H
