/*
 * simtoolbar.cpp
 *
 *  Created on: Aug 7, 2011
 */

#include "simtoolbar.h"
#include "commandhub.h"
#include "uicommandaction.h"
#include "log.h"

namespace SIM
{

SimToolbar::SimToolbar(QWidget* parent) : QToolBar(parent),
        m_contextProvider(0)
{
}

SimToolbar::~SimToolbar()
{
}

void SimToolbar::clear()
{

}

bool SimToolbar::load(const UiCommandList& list)
{
    QStringList cmdIds = list.allCommandIds();
    foreach(const QString& cmdId, cmdIds)
    {
        UiCommandPtr cmd = getCommandHub()->command(cmdId);
        if(!cmd)
        {
            log(L_ERROR, "SimToolbar: unable to load cmd id: %s", qPrintable(cmd->id()));
            return false;
        }
        addCommand(cmd);
    }
    return true;
}

UiCommandList SimToolbar::save()
{
    return UiCommandList();
}

int SimToolbar::commandsCount() const
{
    return m_commands.count();
}

UiCommandPtr SimToolbar::command(int i)
{
    return m_commands.at(i);
}

void SimToolbar::setContextProvider(UiCommandContextProvider* provider)
{
    m_contextProvider = provider;
}

void SimToolbar::actionTriggered(bool checked)
{
    UiCommandAction* action = qobject_cast<UiCommandAction*>(sender());
    if(!action)
        return;

    UiCommand* cmd = action->getUiCommand();
    cmd->updateState(checked);

    cmd->perform(m_contextProvider);
}

void SimToolbar::addCommand(const UiCommandPtr& cmd)
{
    m_commands.append(cmd);

    UiCommandAction* action = cmd->createAction(this);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(actionTriggered(bool)));
    addAction(((QAction*)action));

}

} /* namespace SIM */
