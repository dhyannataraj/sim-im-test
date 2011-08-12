/*
 * menuaction.h
 *
 *  Created on: Aug 12, 2011
 */

#ifndef MENUACTION_H_
#define MENUACTION_H_

#include <QPushButton>
#include <QWidgetAction>
#include <QMenu>

#include "simapi.h"

namespace SIM
{

class MenuActionButton : public QPushButton
{
    Q_OBJECT
public:
    MenuActionButton(const QIcon& icon, QWidget* parent = 0);
    virtual ~MenuActionButton();

protected:
    virtual void resizeEvent(QResizeEvent* event);
};

class EXPORT MenuAction: public QWidgetAction
{
    Q_OBJECT
public:
    MenuAction(QObject* parent = 0);
    virtual ~MenuAction();

protected:
    virtual QWidget* createWidget(QWidget* parent);
    virtual void deleteWidget(QWidget* widget);

private slots:
    void popupMenu();

private:
};

} /* namespace SIM */
#endif /* MENUACTION_H_ */
