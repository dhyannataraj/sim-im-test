#include <QMenu>
#include "uicommand.h"
#include "imagestorage/imagestorage.h"
#include "log.h"

namespace SIM
{

UiCommandPtr UiCommand::create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags)
{
    return UiCommandPtr(new UiCommand(text, iconId, id, tags));
}

UiCommand::UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags) : QObject(0),
    m_id(id),
    m_tags(tags),
    m_text(text),
    m_checkable(false),
    m_checked(false)
{
    setIconId(iconId);
}

QString UiCommand::text() const
{
    return m_text;
}

QIcon UiCommand::icon() const
{
    return m_icon;
}

QString UiCommand::id() const
{
    return m_id;
}

void UiCommand::addTag(const QString& tag)
{
    m_tags.append(tag);
}

void UiCommand::clearTags()
{
    m_tags.clear();
}

QStringList UiCommand::tags() const
{
    return m_tags;
}

void UiCommand::setIconId(const QString& id)
{
    m_iconId = id;
    if(getImageStorage())
        m_icon = getImageStorage()->icon(id);
}

QString UiCommand::iconId() const
{
    return m_iconId;
}

bool UiCommand::isCheckable() const
{
    return m_checkable;
}

void UiCommand::setCheckable(bool b)
{
    m_checkable = b;
}

bool UiCommand::isChecked() const
{
    return m_checked;
}

}
