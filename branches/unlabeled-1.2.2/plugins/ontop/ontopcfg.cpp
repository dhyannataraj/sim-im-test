/***************************************************************************
                          ontopcfg.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ontopcfg.h"
#include "ontop.h"

#include <qcheckbox.h>

OnTopCfg::OnTopCfg(QWidget *parent, OnTopPlugin *plugin)
        : OnTopCfgBase(parent)
{
    m_plugin = plugin;
    chkInTask->setChecked(m_plugin->getInTask());
}

void OnTopCfg::apply()
{
    m_plugin->setInTask(chkInTask->isChecked());
    m_plugin->setState();
}

#ifndef WIN32
#include "ontopcfg.moc"
#endif

