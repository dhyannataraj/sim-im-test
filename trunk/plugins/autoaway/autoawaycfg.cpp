/***************************************************************************
                          autoawaycfg.cpp  -  description
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

#include "autoawaycfg.h"
#include "autoaway.h"

#include <qcheckbox.h>
#include <qspinbox.h>

AutoAwayConfig::AutoAwayConfig(QWidget *parent, AutoAwayPlugin *plugin)
        : AutoAwayConfigBase(parent)
{
    m_plugin = plugin;
    chkAway->setChecked(m_plugin->getEnableAway());
    chkNA->setChecked(m_plugin->getEnableNA());
    spnAway->setValue(m_plugin->getAwayTime());
    spnNA->setValue(m_plugin->getNATime());
    chkAlert->setChecked(m_plugin->getDisableAlert());
    connect(chkAway, SIGNAL(toggled(bool)), this, SLOT(toggledAway(bool)));
    connect(chkNA, SIGNAL(toggled(bool)), this, SLOT(toggledNA(bool)));
    toggledAway(chkAway->isChecked());
    toggledNA(chkNA->isChecked());
}

void AutoAwayConfig::toggledAway(bool bState)
{
    spnAway->setEnabled(bState);
}

void AutoAwayConfig::toggledNA(bool bState)
{
    spnNA->setEnabled(bState);
}

void AutoAwayConfig::apply()
{
    m_plugin->setDisableAlert(chkAlert->isChecked());
    m_plugin->setEnableAway(chkAway->isChecked());
    m_plugin->setEnableNA(chkNA->isChecked());
    if (m_plugin->getEnableAway())
        m_plugin->setAwayTime(atol(spnAway->text().latin1()));
    if (m_plugin->getEnableNA())
        m_plugin->setNATime(atol(spnAway->text().latin1()));
}

#ifndef WIN32
#include "autoawaycfg.moc"
#endif

