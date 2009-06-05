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
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);
    sldTransparency->setValue(m_plugin->getTransparency());
    chkInactive->setChecked  (m_plugin->getIfInactive  ());
    chkMainWindow->setChecked(m_plugin->getIfMainWindow());
    chkFloatings->setChecked (m_plugin->getIfFloatings ());
}

void TransparentCfg::apply()
{
    m_plugin->setTransparency(sldTransparency->value());
    m_plugin->setIfInactive  (chkInactive  ->isChecked());
    m_plugin->setIfMainWindow(chkMainWindow->isChecked());
    m_plugin->setIfFloatings (chkFloatings ->isChecked());
    m_plugin->setState();
}


