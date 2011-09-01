/*
 * widgetcollectioneventreceiver.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef WIDGETCOLLECTIONEVENTRECEIVER_H_
#define WIDGETCOLLECTIONEVENTRECEIVER_H_

#include <QObject>
#include "widgethierarchy.h"
#include "misc.h"

namespace TestHelper
{
    class WidgetCollectionEventReceiver : public QObject
    {
        Q_OBJECT
    public:
        WidgetCollectionEventReceiver(const QString& widgetName);
        virtual ~WidgetCollectionEventReceiver();

        int receviedEventCount() const;
        QString lastContext() const;

        void setHierarchy(SIM::WidgetHierarchy* hierarchy);

    public slots:
        void eventReceived(SIM::WidgetHierarchy* hierarchy, const QString& context);

    private:
        int m_receivedEventCount;
        QString m_context;
        QString m_widgetName;
        SIM::WidgetHierarchy* m_hierarchy;
    };
}

#endif /* WIDGETCOLLECTIONEVENTRECEIVER_H_ */
