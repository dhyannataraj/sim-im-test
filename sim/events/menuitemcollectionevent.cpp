/*
 * menuitemcollectionevent.cpp
 *
 *  Created on: Aug 14, 2011
 */

#include "menuitemcollectionevent.h"
#include "log.h"

namespace SIM
{

MenuItemCollectionEventData::MenuItemCollectionEventData(const QString& menuId) : m_menuId(menuId)
{

}

MenuItemCollectionEventData::~MenuItemCollectionEventData()
{

}

QString MenuItemCollectionEventData::eventId() const
{
    return "menu_event";
}

QString MenuItemCollectionEventData::menuId() const
{
    return m_menuId;
}

void MenuItemCollectionEventData::addActions(const QStringList& actions)
{
    m_actions.append(actions);
}

QStringList MenuItemCollectionEventData::actions() const
{
    return m_actions;
}

EventDataPtr MenuItemCollectionEventData::create(const QString& menuId)
{
    return EventDataPtr(new MenuItemCollectionEventData(menuId));
}

MenuItemCollectionEvent::MenuItemCollectionEvent()
{
}

MenuItemCollectionEvent::~MenuItemCollectionEvent()
{
}

QString MenuItemCollectionEvent::id()
{
    return "menu_event";
}

bool MenuItemCollectionEvent::connectTo(QObject* receiver, const char* receiverSlot)
{
    return QObject::connect(this, SIGNAL(eventTriggered(QString, QStringList*)), receiver, receiverSlot);
}

void MenuItemCollectionEvent::triggered(const EventDataPtr& data)
{
    if(data->eventId() != id())
    {
        log(L_WARN, "MenuItemCollectionEvent data type mismatch");
        return;
    }

    QSharedPointer<MenuItemCollectionEventData> d = data.staticCast<MenuItemCollectionEventData>();
    QStringList actions;
    emit eventTriggered(d->menuId(), &actions);
    d->addActions(actions);
}

} /* namespace SIM */
