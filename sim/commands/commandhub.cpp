#include "commandhub.h"
#include "imagestorage/imagestorage.h"

namespace SIM
{
CommandHub::CommandHub(QObject *parent) :
    QObject(parent)
{
    ActionDescriptor separator;
    separator.action = new QAction(this);
    separator.action->setSeparator(true);
    separator.id = "separator";
    registerAction(separator);
}

void CommandHub::registerAction(const ActionDescriptor& action)
{
    m_commands.append(action);
    if(!action.iconId.isEmpty())
        action.action->setIcon(getImageStorage()->icon(action.iconId));
    action.action->setText(action.text);
    action.action->setParent(this);
}

void CommandHub::unregisterAction(const QString& id)
{
    for(QList<ActionDescriptor>::iterator it = m_commands.begin(); it != m_commands.end(); ++it) {
        if((*it).id == id) {
            m_commands.erase(it);
            return;
        }
    }
}

QAction* CommandHub::action(const QString& id) const
{
    for(QList<ActionDescriptor>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++it) {
        if((*it).id == id) {
            return (*it).action;
        }
    }
    return 0;
}

QStringList CommandHub::actionsForTag(const QString& tag) const
{
    QStringList ids;
    for(QList<ActionDescriptor>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++it)
    {
        if((*it).tags.contains(tag))
        {
            ids.append((*it).id);
        }
    }
    return ids;
}

static CommandHub* g_commandHub = 0;

EXPORT CommandHub* getCommandHub()
{
    return g_commandHub;
}

void EXPORT createCommandHub()
{
    Q_ASSERT(!g_commandHub);
    g_commandHub = new CommandHub();
}

void EXPORT destroyCommandHub()
{
    Q_ASSERT(g_commandHub);
    delete g_commandHub;
    g_commandHub = 0;
}

}
