/*
 * simtoolbar.h
 *
 *  Created on: Aug 7, 2011
 */

#ifndef SIMTOOLBAR_H_
#define SIMTOOLBAR_H_

#include <QToolBar>
#include <QList>

#include "uicommandlist.h"
#include "uicommand.h"
#include "uicommandcontextprovider.h"
#include "simapi.h"

namespace SIM
{

class EXPORT SimToolbar : public QToolBar
{
    Q_OBJECT
public:
    SimToolbar(QWidget* parent = 0);
    virtual ~SimToolbar();

    void clear();
    bool load(const UiCommandList& list);
    UiCommandList save();

    int commandsCount() const;
    UiCommandPtr command(int i);

    void setContextProvider(UiCommandContextProvider* provider);

protected slots:
    void actionTriggered(bool checked);

private:
    void addCommand(const UiCommandPtr& cmd);
    QList<UiCommandPtr> m_commands;
    UiCommandContextProvider* m_contextProvider;
};

} /* namespace SIM */
#endif /* SIMTOOLBAR_H_ */
