/*
 * widgethierarchy.h
 *
 *  Created on: Aug 30, 2011
 */

#ifndef WIDGETHIERARCHY_H_
#define WIDGETHIERARCHY_H_

#include <QString>
#include <QWidget>
#include <QList>

#include "simapi.h"

namespace SIM
{
    struct EXPORT WidgetHierarchy
    {
        QString nodeName;
        QWidget* widget;
        QList<WidgetHierarchy> children;
    };
}


#endif /* WIDGETHIERARCHY_H_ */
