/***************************************************************************
                          transparentcfg.cpp  -  description
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

#include "transparentcfg.h"
#include "transparent.h"

#include <qcheckbox.h>
#include <qslider.h>

TransparentCfg::TransparentCfg(QWidget *parent, TransparentPlugin *plugin)
        : TransparentCfgBase(parent)
{
    m_plugin = plugin;
    sldTransparency->setValue(m_plugin->getTransparency());
#ifdef WIN32
    chkInactive->setChecked(m_plugin->getIfInactive());
#else
    chkInactive->hide();
#endif
}

void TransparentCfg::apply()
{
    m_plugin->setTransparency(sldTransparency->value());
#ifdef WIN32
    m_plugin->setIfInactive(chkInactive->isChecked());
#endif
    m_plugin->setState();
}

#ifndef WIN32
#include "transparentcfg.moc"
#endif

