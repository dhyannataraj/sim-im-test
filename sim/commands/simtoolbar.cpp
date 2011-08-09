/*
 * simtoolbar.cpp
 *
 *  Created on: Aug 7, 2011
 */

#include "simtoolbar.h"
#include "commandhub.h"
#include "log.h"

namespace SIM
{

SimToolbar::SimToolbar(QWidget* parent) : QToolBar(parent)
{
}

SimToolbar::~SimToolbar()
{
}

void SimToolbar::clear()
{

}

bool SimToolbar::load(const UiCommandList& list)
{
    QStringList cmdIds = list.allCommandIds();
    foreach(const QString& cmdId, cmdIds)
    {
        QAction* cmd = getCommandHub()->action(cmdId);
        if(!cmd)
        {
            log(L_ERROR, "SimToolbar: unable to load cmd id: %s", qPrintable(cmdId));
            return false;
        }
        addAction(cmd);
    }
    return true;
}

UiCommandList SimToolbar::save()
{
    return UiCommandList();
}

} /* namespace SIM */
