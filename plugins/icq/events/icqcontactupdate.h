/*
 * icqcontactupdate.h
 *
 *  Created on: Sep 14, 2011
 */

#ifndef ICQCONTACTUPDATE_H_
#define ICQCONTACTUPDATE_H_

#include "events/ievent.h"
#include "icq_defines.h"
#include <QSharedPointer>

class IcqContactUpdateData;
typedef QSharedPointer<IcqContactUpdateData> IcqContactUpdateDataPtr;

class ICQ_EXPORT IcqContactUpdateData : public SIM::EventData
{
public:
    static IcqContactUpdateDataPtr create(const QString& id, const QString& screen);
    virtual ~IcqContactUpdateData();

    virtual QString eventId() const;

    QString screen() const;

private:
    IcqContactUpdateData(const QString& id, const QString& screen);
    Q_DISABLE_COPY(IcqContactUpdateData);

    QString m_eventId;
    QString m_screen;
};

class ICQ_EXPORT IcqContactUpdate: public SIM::IEvent
{
    Q_OBJECT
public:
    static SIM::IEventPtr create(const QString& eventId);
    virtual ~IcqContactUpdate();

    virtual QString id();
    virtual bool connectTo(QObject* receiver, const char* receiverSlot);

signals:
    void eventTriggered(const QString& icqContactScreen);

public slots:
    virtual void triggered(const SIM::EventDataPtr& data);

private:
    IcqContactUpdate(const QString& eventId);
    Q_DISABLE_COPY(IcqContactUpdate);

    QString m_id;
};

#endif /* ICQCONTACTUPDATE_H_ */
