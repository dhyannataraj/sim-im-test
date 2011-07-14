/*
 * icqrequest.h
 *
 *  Created on: Jul 8, 2011
 *      Author: todin
 */

#ifndef ICQREQUEST_H_
#define ICQREQUEST_H_

#include <QSharedPointer>
#include "../icq_defines.h"

class OscarSocket;
class ICQ_EXPORT ICQRequest
{
public:
    virtual ~ICQRequest() {}

    virtual void perform(OscarSocket* socket) = 0;
};

typedef QSharedPointer<ICQRequest> ICQRequestPtr;

#endif /* ICQREQUEST_H_ */
