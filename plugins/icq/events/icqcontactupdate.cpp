/*
 * icqcontactupdate.cpp
 *
 *  Created on: Sep 14, 2011
 */

#include "icqcontactupdate.h"

IcqContactUpdateDataPtr IcqContactUpdateData::create(const QString& id, const QString& screen)
{
    return IcqContactUpdateDataPtr(new IcqContactUpdateData(id, screen));
}

IcqContactUpdateData::~IcqContactUpdateData()
{

}

QString IcqContactUpdateData::eventId() const
{
    return m_eventId;
}

QString IcqContactUpdateData::screen() const
{
    return m_screen;
}

IcqContactUpdateData::IcqContactUpdateData(const QString& id, const QString& screen) :
        m_eventId(id),
        m_screen(screen)
{

}

SIM::IEventPtr IcqContactUpdate::create(const QString& eventId)
{
    return SIM::IEventPtr(new IcqContactUpdate(eventId));
}

IcqContactUpdate::IcqContactUpdate(const QString& eventId) : m_id(eventId)
{
}

IcqContactUpdate::~IcqContactUpdate()
{
}

QString IcqContactUpdate::id()
{
    return m_id;
}

bool IcqContactUpdate::connectTo(QObject* receiver, const char* receiverSlot)
{
    return connect(this, SIGNAL(eventTriggered(QString)), receiver, receiverSlot);
}

void IcqContactUpdate::triggered(const SIM::EventDataPtr& data)
{
    IcqContactUpdateDataPtr d = data.dynamicCast<IcqContactUpdateData>();
    if(!d)
        return;

    emit eventTriggered(d->screen());
}

