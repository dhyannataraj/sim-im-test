/*
 * widgetcollectionevent.h
 *
 *  Created on: Aug 29, 2011
 */

#ifndef WIDGETCOLLECTIONEVENT_H_
#define WIDGETCOLLECTIONEVENT_H_

#include "ievent.h"
#include "simapi.h"

namespace SIM
{

class EXPORT WidgetCollectionEvent : public SIM::IEvent
{
    Q_OBJECT
public:
    WidgetCollectionEvent();
    virtual ~WidgetCollectionEvent();

    virtual QString id();
    virtual bool connectTo(QObject* receiver, const char* receiverSlot);

public slots:
    virtual void triggered(const EventDataPtr& data);
};

} /* namespace SIM */
#endif /* WIDGETCOLLECTIONEVENT_H_ */
