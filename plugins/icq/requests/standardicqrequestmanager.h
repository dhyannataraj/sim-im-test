/*
 * standardicqrequestmanager.h
 *
 *  Created on: Jul 10, 2011
 */

#ifndef STANDARDICQREQUESTMANAGER_H_
#define STANDARDICQREQUESTMANAGER_H_

#include <QQueue>

#include "icqrequestmanager.h"
#include "../icq_defines.h"

class ICQClient;
class OscarSocket;
class ICQ_EXPORT StandardICQRequestManager : public QObject, public ICQRequestManager
{
    Q_OBJECT
public:
    StandardICQRequestManager();
    virtual ~StandardICQRequestManager();

    virtual void enqueue(const ICQRequestPtr& request);
    virtual void clearQueue();
    virtual void setOscarSocket(OscarSocket* socket);

protected slots:
    void socketConnected();

private:
    OscarSocket* m_socket;
    QQueue<ICQRequestPtr> m_requests;
};

#endif /* STANDARDICQREQUESTMANAGER_H_ */
