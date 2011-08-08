/*
 * uicommandaction.cpp
 *
 *  Created on: Aug 7, 2011
 */

#include "uicommandaction.h"

namespace SIM
{

UiCommandHolder::UiCommandHolder(UiCommand* cmd) : m_command(cmd)
{

}

UiCommandHolder::~UiCommandHolder()
{

}

UiCommand* UiCommandHolder::getUiCommand() const
{
    return m_command;
}

UiCommandAction::UiCommandAction(QWidget* parent, UiCommand* cmd) : QAction(parent),
        UiCommandHolder(cmd)
{

}

UiCommandAction::~UiCommandAction()
{
}


} /* namespace SIM */
