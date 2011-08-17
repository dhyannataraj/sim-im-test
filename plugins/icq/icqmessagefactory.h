/*
 * icqmessagefactory.h
 *
 *  Created on: Aug 16, 2011
 */

#ifndef ICQMESSAGEFACTORY_H_
#define ICQMESSAGEFACTORY_H_

#include "messaging/message.h"
#include "icq_defines.h"

class ICQ_EXPORT ICQMessageFactory
{
public:
    ICQMessageFactory();
    virtual ~ICQMessageFactory();

    virtual SIM::MessagePtr createMessage(const QString& messageTypeId);
};

#endif /* ICQMESSAGEFACTORY_H_ */
