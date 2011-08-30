/*
 * testwidgetcollectionevent.h
 *
 *  Created on: Aug 30, 2011
 */

#ifndef TESTWIDGETCOLLECTIONEVENT_H_
#define TESTWIDGETCOLLECTIONEVENT_H_

#include <QObject>
#include "widgethierarchy.h"

namespace TestHelper
{
    class WidgetCollectionEventReceiver : public QObject
    {
        Q_OBJECT
    public:
        WidgetCollectionEventReceiver();
        virtual ~WidgetCollectionEventReceiver();

        int receviedEventCount() const;
        QString lastContext() const;

    public slots:
        void eventReceived(SIM::WidgetHierarchy* hierarchy, const QString& context);

    private:
        int m_receivedEventCount;
        QString m_context;
    };
}

#endif /* TESTWIDGETCOLLECTIONEVENT_H_ */
