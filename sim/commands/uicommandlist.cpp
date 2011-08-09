/*
 * uicommandlist.cpp
 *
 *  Created on: Aug 7, 2011
 */

#include "uicommandlist.h"

namespace SIM
{

UiCommandList::UiCommandList()
{

}

UiCommandList::~UiCommandList()
{
}

QStringList UiCommandList::allCommandIds() const
{
    return m_commandIds;
}

void UiCommandList::clear()
{
    m_commandIds.clear();
}

void UiCommandList::appendCommand(const QString& commandId)
{
    m_commandIds.append(commandId);
}

} /* namespace SIM */
