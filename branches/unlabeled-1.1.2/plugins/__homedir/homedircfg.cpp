/***************************************************************************
                          homedircfg.cpp  -  description
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

#include "homedircfg.h"
#include "homedir.h"

#include "editfile.h"

#include <qcheckbox.h>

HomeDirConfig::HomeDirConfig(QWidget *parent, HomeDirPlugin *plugin)
        : HomeDirConfigBase(parent)
{
    m_plugin = plugin;
    chkDefault->setChecked(plugin->m_bDefault);
    connect(chkDefault, SIGNAL(toggled(bool)), this, SLOT(defaultToggled(bool)));
    defaultToggled(chkDefault->isChecked());
    edtPath->setText(QString::fromLocal8Bit(plugin->m_homeDir.c_str()));
    edtPath->setDirMode(true);
}

void HomeDirConfig::apply()
{
    if (chkDefault->isChecked()){
        m_plugin->m_bDefault = true;
        m_plugin->m_homeDir = m_plugin->defaultPath();
    }else{
        m_plugin->m_bDefault = false;
        m_plugin->m_homeDir = edtPath->text().local8Bit();
    }
}

void HomeDirConfig::defaultToggled(bool bState)
{
    edtPath->setEnabled(!bState);
    if (bState)
        edtPath->setText(QString::fromLocal8Bit(m_plugin->defaultPath().c_str()));
}


