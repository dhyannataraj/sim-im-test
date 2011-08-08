/*
 * actionshowoffline.cpp
 *
 *  Created on: Aug 8, 2011
 */

#include "actionshowoffline.h"
#include "commands/uicommandcontext.h"
#include "mainwin.h"
#include "roster/userview.h"
#include "log.h"

SIM::UiCommandPtr ActionShowOffline::create(MainWindow* view)
{
    return SIM::UiCommandPtr(new ActionShowOffline(view));
}

ActionShowOffline::ActionShowOffline(MainWindow* view) :
        SIM::UiCommand("Show offline", "SIM", "show_offline", QStringList("main_toolbar")),
        m_view(view)
{

}

ActionShowOffline::~ActionShowOffline()
{
}

QList<SIM::UiCommandContext::UiCommandContext> ActionShowOffline::getAvailableContexts() const
{
    return QList<SIM::UiCommandContext::UiCommandContext>() << SIM::UiCommandContext::Nothing;
}

SIM::UiCommandAction* ActionShowOffline::createAction(QWidget* parent)
{
    SIM::UiCommandAction* action = SIM::UiCommand::createAction(parent);
    action->setCheckable(true);
    action->setChecked(false);
    return action;
}

void ActionShowOffline::perform(SIM::UiCommandContextProvider* provider)
{
    m_view->userview()->setShowOffline(this->state());
}
