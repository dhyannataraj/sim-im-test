/***************************************************************************
                          navcfg.cpp  -  description
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

#include "navcfg.h"
#include "navigate.h"

#include <qlineedit.h>

NavCfg::NavCfg(QWidget *parent, NavigatePlugin *plugin)
        : NavCfgBase(parent)
{
    m_plugin = plugin;
    edtBrowser->setText(QString::fromLocal8Bit(plugin->getBrowser()));
    edtMailer->setText(QString::fromLocal8Bit(plugin->getMailer()));
}

void NavCfg::apply()
{
    m_plugin->setBrowser(edtBrowser->text().local8Bit());
    m_plugin->setMailer(edtMailer->text().local8Bit());
}

#ifndef WIN32
#include "navcfg.moc"
#endif

