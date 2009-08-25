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

#include <QCheckBox>
#include <QSlider>

TransparentCfg::TransparentCfg(QWidget *parent, TransparentPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);
    sldTransparency->setValue(m_plugin->property("Transparency").toUInt());
    chkInactive->setChecked(m_plugin->property("IfInactive").toBool());
    chkMainWindow->setChecked(m_plugin->property("IfMainWindow").toBool());
    chkFloatings->setChecked (m_plugin->property("IfFloatings").toBool());
}

void TransparentCfg::apply()
{
    m_plugin->setProperty("Transparency", sldTransparency->value());
    m_plugin->setProperty("IfInactive", chkInactive->isChecked());
    m_plugin->setProperty("IfMainWindow", chkMainWindow->isChecked());
    m_plugin->setProperty("IfFloatings", chkFloatings->isChecked());
    m_plugin->setState();
}


