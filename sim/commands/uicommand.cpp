#include <QMenu>
#include "uicommand.h"
#include "imagestorage/imagestorage.h"
#include "log.h"
#include "uicommandaction.h"

namespace SIM
{

UiCommand::UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags) : QObject(0),
    m_id(id),
    m_tags(tags),
    m_text(text)
{
    setIconId(iconId);
}

UiCommand::~UiCommand()
{

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

void UiCommand::perform(UiCommandContextProvider* provider)
{
    log(L_WARN, "UiCommand::perform without context called");
}


UiCommandAction* UiCommand::createAction(QWidget* parent)
{
    UiCommandAction* action = new UiCommandAction(parent, this);
    action->setIcon(getImageStorage()->icon(m_iconId));
    connect(this, SIGNAL(stateChanged(bool)), action, SLOT(setChecked(bool)));
    return action;
}

bool UiCommand::state() const
{
    return m_state;
}

void UiCommand::updateState(bool active)
{
    emit stateChanged(active);
    m_state = active;
}

}
