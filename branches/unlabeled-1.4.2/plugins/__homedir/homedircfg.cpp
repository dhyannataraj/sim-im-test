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
#include <qdir.h>

HomeDirConfig::HomeDirConfig(QWidget *parent, HomeDirPlugin *plugin)
        : HomeDirConfigBase(parent)
{
    m_plugin = plugin;
    chkDefault->setChecked(plugin->m_bDefault);
    connect(chkDefault, SIGNAL(toggled(bool)), this, SLOT(defaultToggled(bool)));
    defaultToggled(chkDefault->isChecked());
    edtPath->setText(QFile::decodeName(plugin->m_homeDir.c_str()));
    edtPath->setDirMode(true);
    chkDefault->setChecked(m_plugin->m_bDefault);
}

void HomeDirConfig::apply()
{
    bool bDefault;
    QString d;
    QString defPath = QFile::decodeName(m_plugin->defaultPath().c_str());

    if (chkDefault->isChecked()){
        bDefault = true;
        d = defPath;
    }else{
        bDefault = false;
        d = edtPath->text();
    }
    if (d.isEmpty()) {
        d = defPath;
    }
    QDir dir(d);
    if (!dir.exists()) {
        d = defPath;
        bDefault = true;
    }
    edtPath->setText(d);
    m_plugin->m_bDefault = bDefault;
#ifdef WIN32
	d = d.replace(QRegExp("/"), "\\");
	if (d.length() && (d[(int)(d.length() - 1)] == '\\'))
		d = d.left(d.length() - 1);
#else
	if (d.length() && (d[(int)(d.length() - 1)] == '/'))
		d = d.left(d.length() - 1);
#endif
    m_plugin->m_homeDir  = QFile::encodeName(d);
}

void HomeDirConfig::defaultToggled(bool bState)
{
    edtPath->setEnabled(!bState);
    if (bState)
        edtPath->setText(QFile::decodeName(m_plugin->defaultPath().c_str()));
}


