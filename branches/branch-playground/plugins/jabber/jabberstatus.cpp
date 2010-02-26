
#include "jabberstatus.h"


JabberStatus::JabberStatus(const QString& id, const QString& name, const QString& defaultText, const QIcon& icon) : IMStatus(),
    m_id(id),
    m_name(name),
    m_text(defaultText),
    m_icon(icon)
{
}

JabberStatus::~JabberStatus()
{
}


QString JabberStatus::id() const
{
    return m_id;
}

QString JabberStatus::name() const
{
    return m_name;
}

QString JabberStatus::text() const
{
    return m_text;
}

QIcon JabberStatus::icon() const
{
    return m_icon;
}

QStringList JabberStatus::substatuses()
{
    return QStringList();
}

SIM::IMStatus const* JabberStatus::substatus(const QString& id)
{
    return 0;
}

// vim: set expandtab:

