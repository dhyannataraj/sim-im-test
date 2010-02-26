
#include "icqstatus.h"

ICQStatus::ICQStatus(const QString& id, const QString& name, const QString& defaultText, const QIcon& icon) : IMStatus(),
    m_id(id),
    m_name(name),
    m_text(defaultText),
    m_icon(icon)
{
}

ICQStatus::~ICQStatus()
{
}

QString ICQStatus::id() const
{
    return m_id;
}

QString ICQStatus::name() const
{
    return m_name;
}

QString ICQStatus::text() const
{
    return m_text;
}

QIcon ICQStatus::icon() const
{
    return m_icon;
}

QStringList ICQStatus::substatuses()
{
    return QStringList();
}

SIM::IMStatus const* ICQStatus::substatus(const QString& id)
{
    return 0;
}

// vim: set expandtab:

