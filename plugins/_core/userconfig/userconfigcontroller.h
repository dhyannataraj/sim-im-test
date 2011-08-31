/*
 * userconfigcontroller.h
 *
 *  Created on: Aug 29, 2011
 */

#ifndef USERCONFIGCONTROLLER_H_
#define USERCONFIGCONTROLLER_H_

#include "userconfigcontext.h"
#include "usercfg.h"

class UserConfigController
{
public:
    UserConfigController();
    virtual ~UserConfigController();

    bool init(const UserConfigContextPtr& context);

protected:
    virtual UserConfigPtr createUserConfigView();

private:
    UserConfigPtr m_view;
};

#endif /* USERCONFIGCONTROLLER_H_ */
