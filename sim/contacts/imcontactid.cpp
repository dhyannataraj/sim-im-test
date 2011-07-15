#include "imcontactid.h"

namespace SIM
{
    IMContactId::IMContactId() : m_valid(false)
    {
    }

    IMContactId::IMContactId(const QString& str, int parentContactId) : m_valid(true),
        m_parentContactId(parentContactId)
    {
        QStringList list = str.split('/');
        if(list.size() < 2)
        {
            m_parentContactId = -1;
            m_valid = false;
            return;
        }
        m_clientId = list.at(0);
        m_clientSpecificId = list.at(1);
    }

    QString IMContactId::toString() const
    {
        return m_clientId + '/' + m_clientSpecificId;
    }

    QString IMContactId::clientId() const
    {
        return m_clientId;
    }

    int IMContactId::parentContactId() const
    {
        return m_parentContactId;
    }

    QString IMContactId::clientSpecificId() const
    {
        return m_clientSpecificId;
    }

    bool IMContactId::isValid() const
    {
        return m_valid;
    }

    bool IMContactId::operator==(const IMContactId& id) const
    {
        return (m_parentContactId == id.parentContactId()) && (m_clientId == id.clientId()) && (m_clientSpecificId == id.clientSpecificId());
    }
}
