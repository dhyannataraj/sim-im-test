/*
 * uicommandcontextprovider.cpp
 *
 *  Created on: Aug 7, 2011
 */

#include "uicommandcontextprovider.h"

namespace SIM
{

UiCommandContextProvider::~UiCommandContextProvider()
{
}

ContactPtr UiCommandContextProvider::getContextContact()
{
    return ContactPtr();
}

GroupPtr UiCommandContextProvider::getContextGroup()
{
    return GroupPtr();
}

IMContactPtr UiCommandContextProvider::getContextIMContact()
{
    return IMContactPtr();
}

IMGroupPtr UiCommandContextProvider::getContextIMGroup()
{
    return IMGroupPtr();
}

} /* namespace SIM */
