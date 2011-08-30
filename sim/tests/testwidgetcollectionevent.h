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

    public slots:
        void eventReceived(SIM::WidgetHierarchy* hierarchy);

    private:
        int m_receivedEventCount;
    };
}

#endif /* TESTWIDGETCOLLECTIONEVENT_H_ */
