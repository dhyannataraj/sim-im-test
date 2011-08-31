/*
 * widgetcollectioneventreceiver.cpp
 *
 *  Created on: Aug 31, 2011
 */

#include "widgetcollectioneventreceiver.h"

namespace TestHelper
{
    WidgetCollectionEventReceiver::WidgetCollectionEventReceiver(const QString& widgetName) : m_receivedEventCount(0),
            m_widgetName(widgetName), m_hierarchy(nullptr)
    {

    }

    WidgetCollectionEventReceiver::~WidgetCollectionEventReceiver()
    {

    }

    int WidgetCollectionEventReceiver::receviedEventCount() const
    {
        return m_receivedEventCount;
    }

    QString WidgetCollectionEventReceiver::lastContext() const
    {
        return m_context;
    }

    void WidgetCollectionEventReceiver::eventReceived(SIM::WidgetHierarchy* root, const QString& context)
    {
        m_receivedEventCount++;
        m_context = context;
        if(root)
        {
            if(!m_hierarchy)
            {
                SIM::WidgetHierarchy widget;
                widget.nodeName = m_widgetName;
                widget.widget = 0;
                root->children.append(widget);
            }
            else
            {
                root->children.append(*m_hierarchy);
            }
        }
    }
}


