/*
 * commonstatusevent.h
 *
 *  Created on: Aug 11, 2011
 */

#ifndef COMMONSTATUSEVENT_H_
#define COMMONSTATUSEVENT_H_

#include "events/ievent.h"
#include "events/eventdata.h"
#include <QSharedPointer>

namespace SIM
{

    class CommonStatusEventData : public SIM::EventData
    {
    public:
        CommonStatusEventData(const QString& statusId) {}
        virtual ~CommonStatusEventData() {}

        virtual QString eventId() const { return "common_status"; }

        static EventDataPtr create(const QString& statusId) { return EventDataPtr(new CommonStatusEventData(statusId)); }

        QString id() const { return m_id; }

    private:
        QString m_id;
    };

    class CommonStatusEvent: public SIM::IEvent
    {
        Q_OBJECT
    public:
        CommonStatusEvent();
        virtual ~CommonStatusEvent();

        virtual QString id();
        virtual bool connectTo(QObject* receiver, const char* receiverSlot);

    signals:
        void eventTriggered(const QString& statusId);

    public slots:
        virtual void triggered(const EventDataPtr& data);

    private:
        ;
    };

}

#endif /* COMMONSTATUSEVENT_H_ */
