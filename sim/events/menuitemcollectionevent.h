/*
 * menuitemcollectionevent.h
 *
 *  Created on: Aug 14, 2011
 */

#ifndef MENUITEMCOLLECTIONEVENT_H_
#define MENUITEMCOLLECTIONEVENT_H_

#include <QList>
#include <QAction>

#include "ievent.h"
#include "eventdata.h"
#include "simapi.h"

namespace SIM
{

class EXPORT MenuItemCollectionEventData : public EventData
{
public:
    MenuItemCollectionEventData(const QString& menuId);
    virtual ~MenuItemCollectionEventData();
    virtual QString eventId() const;

    QString menuId() const;

    void addActions(const QStringList& actions);
    QStringList actions() const;

    static EventDataPtr create(const QString& menuId);

private:
    QString m_menuId;
    QStringList m_actions;
};

class EXPORT MenuItemCollectionEvent: public IEvent
{
    Q_OBJECT
public:
    MenuItemCollectionEvent();
    virtual ~MenuItemCollectionEvent();

    virtual QString id();
    virtual bool connectTo(QObject* receiver, const char* receiverSlot);

signals:
    void eventTriggered(const QString& menuId, QStringList* actionList);

public slots:
    virtual void triggered(const EventDataPtr& data);
};

} /* namespace SIM */
#endif /* MENUITEMCOLLECTIONEVENT_H_ */
