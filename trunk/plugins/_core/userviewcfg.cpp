/***************************************************************************
                          userviewcfg.cpp  -  description
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
#include "userviewcfg.h"
#include "core.h"

#include <qcheckbox.h>
#include <qlabel.h>

UserViewConfig::UserViewConfig(QWidget *parent)
        : UserViewConfigBase(parent)
{
    chkDblClick->setChecked(CorePlugin::m_plugin->getUseDblClick());
    chkSysColors->setChecked(CorePlugin::m_plugin->getUseSysColors());
    btnOnline->setColor(CorePlugin::m_plugin->getColorOnline());
    btnOffline->setColor(CorePlugin::m_plugin->getColorOffline());
    btnAway->setColor(CorePlugin::m_plugin->getColorAway());
    btnNA->setColor(CorePlugin::m_plugin->getColorNA());
    btnDND->setColor(CorePlugin::m_plugin->getColorDND());
    btnGroup->setColor(CorePlugin::m_plugin->getColorGroup());
    connect(chkSysColors, SIGNAL(toggled(bool)), this, SLOT(colorsToggled(bool)));
    colorsToggled(chkSysColors->isChecked());
}

UserViewConfig::~UserViewConfig()
{
}

void UserViewConfig::apply()
{
    CorePlugin::m_plugin->setUseDblClick(chkDblClick->isChecked());
    CorePlugin::m_plugin->setUseSysColors(chkSysColors->isChecked());
    if (CorePlugin::m_plugin->getUseSysColors()){
        CorePlugin::m_plugin->setColorOnline(0);
        CorePlugin::m_plugin->setColorOffline(0);
        CorePlugin::m_plugin->setColorAway(0);
        CorePlugin::m_plugin->setColorNA(0);
        CorePlugin::m_plugin->setColorDND(0);
        CorePlugin::m_plugin->setColorGroup(0);
    }else{
        CorePlugin::m_plugin->setColorOnline(btnOnline->color().rgb());
        CorePlugin::m_plugin->setColorOffline(btnOffline->color().rgb());
        CorePlugin::m_plugin->setColorAway(btnAway->color().rgb());
        CorePlugin::m_plugin->setColorNA(btnNA->color().rgb());
        CorePlugin::m_plugin->setColorDND(btnDND->color().rgb());
        CorePlugin::m_plugin->setColorGroup(btnGroup->color().rgb());
    }
    Event e(EventRepaintView);
    e.process();
}

void UserViewConfig::colorsToggled(bool state)
{
    if (state){
        QColor textColor = colorGroup().text();
        QColor disabledColor = palette().disabled().text();
        btnOnline->setColor(textColor);
        btnOffline->setColor(disabledColor);
        btnAway->setColor(disabledColor);
        btnNA->setColor(disabledColor);
        btnDND->setColor(disabledColor);
    }
    btnOnline->setEnabled(!state);
    btnOffline->setEnabled(!state);
    btnAway->setEnabled(!state);
    btnNA->setEnabled(!state);
    btnDND->setEnabled(!state);
    lblOnline->setEnabled(!state);
    lblOffline->setEnabled(!state);
    lblAway->setEnabled(!state);
    lblNA->setEnabled(!state);
    lblDND->setEnabled(!state);
    lblColors->setEnabled(!state);
}

#ifndef WIN32
#include "userviewcfg.moc"
#endif

