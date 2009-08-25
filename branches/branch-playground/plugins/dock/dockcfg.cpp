/***************************************************************************
                          dockcfg.cpp  -  description
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

#include "simapi.h"

#include "dockcfg.h"
#include "dock.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#ifdef USE_KDE
#include <kwin.h>
#endif

DockCfg::DockCfg(QWidget *parent, DockPlugin *plugin) : QWidget(parent)
{
    setupUi(this);
	m_plugin = plugin;
    chkAutoHide->setChecked(plugin->property("AutoHide").toBool());
    unsigned interval = plugin->property("AutoHideInterval").toUInt();
    spnAutoHide->setValue(interval);
    connect(chkAutoHide, SIGNAL(toggled(bool)), this, SLOT(autoHideToggled(bool)));
    connect(btnCustomize, SIGNAL(clicked()), this, SLOT(customize()));
    autoHideToggled(plugin->property("AutoHide").toBool());
#ifdef USE_KDE
    spn_desk->setMaxValue(KWin::numberOfDesktops());
    spn_desk->setValue(m_plugin->property("Desktop").toUInt());
#else
    spn_desk->hide();
    TextLabel1_2->hide();
#endif
}

void DockCfg::apply()
{
    m_plugin->setProperty("AutoHide", chkAutoHide->isChecked());
    m_plugin->setProperty("AutoHideInterval", (uint)spnAutoHide->text().toULong());
#ifdef USE_KDE
    m_plugin->setProperty("Desktop", spn_desk->text().toULong());
#endif
}

void DockCfg::autoHideToggled(bool bAutoHide)
{
    spnAutoHide->setEnabled(bAutoHide);
}

void DockCfg::customize()
{
    SIM::EventMenu(m_plugin->DockMenu, SIM::EventMenu::eCustomize).process();
}

