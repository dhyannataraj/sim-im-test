/*
 * commonstatusevent.cpp
 *
 *  Created on: Aug 11, 2011
 */

#include "commonstatusevent.h"

namespace SIM
{
    CommonStatusEvent::CommonStatusEvent()
    {
    }

    CommonStatusEvent::~CommonStatusEvent()
    {
    }

    QString CommonStatusEvent::id()
    {
        return "common_status";
    }

    bool CommonStatusEvent::connectTo(QObject* receiver, const char* receiverSlot)
    {
        return connect(this, SIGNAL(eventTriggered(QString)), receiver, receiverSlot);
    }

    void CommonStatusEvent::triggered(const EventDataPtr& data)
    {
        if(data->eventId() != id())
        {
            return;
        }

        QSharedPointer<CommonStatusEventData> d = data.staticCast<CommonStatusEventData>();
        emit eventTriggered(d->id());
    }

}
