/*
 * icqconfigwidgetcreator.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef ICQCONFIGWIDGETCREATOR_H_
#define ICQCONFIGWIDGETCREATOR_H_

#include <QObject>
#include "widgethierarchy.h"

class ICQClient;
class IcqConfigWidgetCreator: public QObject
{
    Q_OBJECT

public:
    IcqConfigWidgetCreator(ICQClient* client);
    virtual ~IcqConfigWidgetCreator();

public slots:
    void contactConfigRequested(SIM::WidgetHierarchy* hierarchy, const QString& context);

private:
    ICQClient* m_client;
};

#endif /* ICQCONFIGWIDGETCREATOR_H_ */
