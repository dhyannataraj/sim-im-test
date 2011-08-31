/*
 * userconfigcontext.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "userconfigcontext.h"

class ContactContext : public UserConfigContext
{
public:
    ContactContext(const SIM::ContactPtr& contact)
    {
        m_contactId = QString::number(contact->id());
    }

    virtual ~ContactContext() {}

    virtual QString widgetCollectionEventId()
    {
        return QString("contact_widget_collection");
    }

    virtual QString context()
    {
        return m_contactId;
    }

private:
    QString m_contactId;
};

UserConfigContextPtr UserConfigContext::create(const SIM::ContactPtr& contact)
{
    return UserConfigContextPtr(new ContactContext(contact));
}
