/*
 * widgetcollectionevent.h
 *
 *  Created on: Aug 29, 2011
 */

#ifndef WIDGETCOLLECTIONEVENT_H_
#define WIDGETCOLLECTIONEVENT_H_

#include "ievent.h"
#include "simapi.h"
#include "widgethierarchy.h"

namespace SIM
{
class WidgetCollectionEventData;
typedef QSharedPointer<WidgetCollectionEventData> WidgetCollectionEventDataPtr;
class WidgetCollectionEventData : public EventData
{
public:
    WidgetCollectionEventData(const QString& id, const QString& context);
    virtual ~WidgetCollectionEventData();

    virtual QString eventId() const;

    WidgetHierarchy* hierarchyRoot() const;
    QString context() const;

    static WidgetCollectionEventDataPtr create(const QString& id, const QString& context = QString());

private:
    QString m_eventId;
    QString m_context;
    WidgetHierarchy* m_root;
};

class EXPORT WidgetCollectionEvent : public SIM::IEvent
{
    Q_OBJECT
public:
    WidgetCollectionEvent(const QString& eventId);
    virtual ~WidgetCollectionEvent();

    virtual QString id();
    virtual bool connectTo(QObject* receiver, const char* receiverSlot);

    static IEventPtr create(const QString& eventId);

signals:
    void eventTriggered(SIM::WidgetHierarchy* hierarchy, const QString& context);

public slots:
    virtual void triggered(const EventDataPtr& data);

private:
    QString m_id;
};

} /* namespace SIM */
#endif /* WIDGETCOLLECTIONEVENT_H_ */
