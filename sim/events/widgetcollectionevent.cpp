/*
 * widgetcollectionevent.cpp
 *
 *  Created on: Aug 29, 2011
 */

#include "widgetcollectionevent.h"

namespace SIM
{

WidgetCollectionEvent::WidgetCollectionEvent()
{
}

WidgetCollectionEvent::~WidgetCollectionEvent()
{
}

QString WidgetCollectionEvent::id()
{
    return QString("widget_collection");
}

bool WidgetCollectionEvent::connectTo(QObject* receiver, const char* receiverSlot)
{
    return false;
}

void WidgetCollectionEvent::triggered(const EventDataPtr& data)
{

}

} /* namespace SIM */
