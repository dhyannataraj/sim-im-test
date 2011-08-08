/*
 * actionshowoffline.h
 *
 *  Created on: Aug 8, 2011
 */

#ifndef ACTIONSHOWOFFLINE_H_
#define ACTIONSHOWOFFLINE_H_

#include "commands/uicommand.h"
#include "commands/uicommandcontextprovider.h"

class MainWindow;

class ActionShowOffline: public SIM::UiCommand
{
public:
    static SIM::UiCommandPtr create(MainWindow* view);

    virtual ~ActionShowOffline();

    virtual QList<SIM::UiCommandContext::UiCommandContext> getAvailableContexts() const;

    virtual SIM::UiCommandAction* createAction(QWidget* parent);
    virtual void perform(SIM::UiCommandContextProvider* provider);

private:
    ActionShowOffline(MainWindow* view);
    MainWindow* m_view;
};

#endif /* ACTIONSHOWOFFLINE_H_ */
