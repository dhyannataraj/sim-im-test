/*
 * icqrequestmanager.h
 *
 *  Created on: Jul 8, 2011
 *      Author: todin
 */

#ifndef ICQREQUESTMANAGER_H_
#define ICQREQUESTMANAGER_H_

#include "icqrequest.h"

class OscarSocket;
class ICQRequestManager
{
public:
    virtual ~ICQRequestManager();

    virtual void setOscarSocket(OscarSocket* socket) = 0;
    virtual void clearQueue() = 0;
    virtual void enqueue(const ICQRequestPtr& request) = 0;
};

#endif /* ICQREQUESTMANAGER_H_ */
