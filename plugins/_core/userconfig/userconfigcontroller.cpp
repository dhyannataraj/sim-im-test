/*
 * userconfigcontroller.cpp
 *
 *  Created on: Aug 29, 2011
 */

#include "userconfigcontroller.h"
#include "events/eventhub.h"
#include "events/widgetcollectionevent.h"

UserConfigController::UserConfigController()
{
}

UserConfigController::~UserConfigController()
{
}

bool UserConfigController::init(const UserConfigContextPtr& context)
{
    SIM::WidgetCollectionEventDataPtr data = SIM::WidgetCollectionEventData::create(context->widgetCollectionEventId(), context->context());
    SIM::getEventHub()->triggerEvent(context->widgetCollectionEventId(), data);

    m_view = createUserConfigView();
    m_view->setWidgetHierarchy(data->hierarchyRoot());
    return true;
}

UserConfigPtr UserConfigController::createUserConfigView()
{
    return UserConfig::create();
}
