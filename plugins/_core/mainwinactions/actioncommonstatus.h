/*
 * actioncommonstatus.h
 *
 *  Created on: Aug 9, 2011
 */

#ifndef ACTIONCOMMONSTATUS_H_
#define ACTIONCOMMONSTATUS_H_

#include <QWidgetAction>

class CommonStatus;
class ActionCommonStatus : public QWidgetAction
{
    Q_OBJECT
public:
    ActionCommonStatus(CommonStatus* status);
    virtual ~ActionCommonStatus();

protected:
    virtual QWidget* createWidget(QWidget* parent);
    virtual void deleteWidget(QWidget* widget);

private slots:
    void statusActivated(int index);

private:
    CommonStatus* m_commonStatus;
};

#endif /* ACTIONCOMMONSTATUS_H_ */
