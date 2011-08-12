/*
 * actionmenucommonstatus.h
 *
 *  Created on: Aug 12, 2011
 */

#ifndef ACTIONMENUCOMMONSTATUS_H_
#define ACTIONMENUCOMMONSTATUS_H_

#include <QAction>

class CommonStatus;
class ActionMenuCommonStatus: public QAction
{
public:
    ActionMenuCommonStatus(CommonStatus* status);
    virtual ~ActionMenuCommonStatus();

private:
    CommonStatus* m_commonStatus;
};

#endif /* ACTIONMENUCOMMONSTATUS_H_ */
