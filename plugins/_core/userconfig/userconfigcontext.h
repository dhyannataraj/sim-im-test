/*
 * userconfigcontext.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef USERCONFIGCONTEXT_H_
#define USERCONFIGCONTEXT_H_

#include <QSharedPointer>
#include "contacts/contact.h"

class UserConfigContext;
typedef QSharedPointer<UserConfigContext> UserConfigContextPtr;
class UserConfigContext
{
public:
    virtual ~UserConfigContext() {}

    virtual QString widgetCollectionEventId() = 0;
    virtual QString context() = 0;

    static UserConfigContextPtr create(const SIM::ContactPtr& contact);
};

#endif /* USERCONFIGCONTEXT_H_ */
