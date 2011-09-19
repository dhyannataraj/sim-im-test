/*
 * userconfigcontroller.cpp
 *
 *  Created on: Aug 29, 2011
 */

#include "userconfigcontroller.h"
#include "events/eventhub.h"
#include "events/widgetcollectionevent.h"

using namespace SIM;

UserConfigController::UserConfigController()
{
}

UserConfigController::~UserConfigController()
{
}

bool UserConfigController::init(const UserConfigContextPtr& context)
{
    WidgetCollectionEventDataPtr data = WidgetCollectionEventData::create(context->widgetCollectionEventId(), context->context());
    getEventHub()->triggerEvent(context->widgetCollectionEventId(), data);

    m_view = createUserConfigView();
    m_view->setWidgetHierarchy(data->hierarchyRoot());
    return true;
}

UserConfigPtr UserConfigController::createUserConfigView()
{
    return UserConfig::create();
}

int UserConfigController::exec()
{
    return m_view->exec();
}
