/***************************************************************************
                          replacecfg.cpp  -  description
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

#include "replacecfg.h"
#include "replace.h"
#include "listview.h"

ReplaceCfg::ReplaceCfg(QWidget *parent, ReplacePlugin *plugin)
        : ReplaceCfgBase(parent)
{
    m_plugin = plugin;
    lstKeys->addColumn(i18n("You type"));
    lstKeys->addColumn(i18n("You send"));
    lstKeys->setExpandingColumn(1);
    lstKeys->adjustColumn();
}

ReplaceCfg::~ReplaceCfg()
{
}

void ReplaceCfg::apply()
{
}

void ReplaceCfg::resizeEvent(QResizeEvent *e)
{
    ReplaceCfgBase::resizeEvent(e);
    lstKeys->adjustColumn();
}

#ifndef WIN32
#include "replacecfg.moc"
#endif

