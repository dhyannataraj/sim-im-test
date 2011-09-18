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
#include <QAction>
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

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);

private:
};

} /* namespace SIM */
#endif /* SIMTOOLBAR_H_ */
