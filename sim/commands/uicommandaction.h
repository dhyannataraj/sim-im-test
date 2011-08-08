/*
 * uicommandaction.h
 *
 *  Created on: Aug 7, 2011
 */

#ifndef UICOMMANDACTION_H_
#define UICOMMANDACTION_H_

#include <QAction>
#include <QWidgetAction>

namespace SIM
{
class UiCommand;

class UiCommandHolder
{
public:
    UiCommandHolder(UiCommand* cmd);
    virtual ~UiCommandHolder();

    UiCommand* getUiCommand() const;

private:
    UiCommand* m_command;
};

class UiCommandAction : public QAction, public UiCommandHolder
{
    Q_OBJECT
public:
    UiCommandAction(QWidget* parent, UiCommand* cmd);
    virtual ~UiCommandAction();

};

} /* namespace SIM */
#endif /* UICOMMANDACTION_H_ */
