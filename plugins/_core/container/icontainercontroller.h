/*
 * icontainercontroller.h
 *
 *  Created on: Aug 14, 2011
 */

#ifndef ICONTAINERCONTROLLER_H_
#define ICONTAINERCONTROLLER_H_

#include <QSharedPointer>
#include "userwndcontroller.h"

class IContainerController
{
public:
    virtual ~IContainerController() {}

    virtual int id() const = 0;

    virtual void sendMessage(const SIM::MessagePtr& msg) = 0;
    virtual void addUserWnd(int contactId) = 0;
    virtual IUserWnd* userWndById(int id) = 0;
    virtual UserWndControllerPtr userWndController(int id) = 0;
    virtual void raiseUserWnd(int id) = 0;
};

typedef QSharedPointer<IContainerController> ContainerControllerPtr;


#endif /* ICONTAINERCONTROLLER_H_ */
