/*
 * icqconfigwidgetcreator.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef ICQCONFIGWIDGETCREATOR_H_
#define ICQCONFIGWIDGETCREATOR_H_

#include <QObject>
#include "widgethierarchy.h"
#include "../icq_defines.h"

class ICQClient;
class ICQ_EXPORT IcqConfigWidgetCreator: public QObject
{
    Q_OBJECT

public:
    IcqConfigWidgetCreator(ICQClient* client);
    virtual ~IcqConfigWidgetCreator();

signals:
    void fullInfoRequest(const QString& contactScreen);

public slots:
    void contactConfigRequested(SIM::WidgetHierarchy* hierarchy, const QString& context);

private:
    ICQClient* m_client;
};

#endif /* ICQCONFIGWIDGETCREATOR_H_ */
