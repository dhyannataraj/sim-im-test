/***************************************************************************
                          plugincfg.cpp  -  description
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

#include "plugincfg.h"
#include "qchildwidget.h"
#include "simapi.h"

#include <qlayout.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpainter.h>

PluginCfg::PluginCfg(QWidget *parent, pluginInfo *info)
        : PluginCfgBase(parent)
{
    m_info = info;
    if (m_info->plugin){
        QWidget *w = m_info->plugin->createConfigWindow(addWnd);
        if (w){
            QVBoxLayout *lay = new QVBoxLayout(addWnd);
            lay->addWidget(w);
            QObject::connect(parent->topLevelWidget(), SIGNAL(applyChanges()), w, SLOT(apply()));
            addWnd->setMinimumSize(w->sizeHint());
            setMinimumSize(sizeHint());
        }
    }
    tabWnd->setCurrentPage(0);
    tabWnd->changeTab(tabWnd->currentPage(), i18n(m_info->info->title));
    if (m_info->info->description){
        lblDescription->setText(i18n(m_info->info->description));
    }else{
        lblDescription->setText("");
    }
    if (m_info->info && (m_info->info->flags & PLUGIN_NODISABLE)){
        chkEnable->hide();
    }else{
        if (m_info->bNoCreate){
            chkEnable->setEnabled(false);
            chkEnable->setChecked(false);
        }else{
            chkEnable->setEnabled(true);
            chkEnable->setChecked(!m_info->bDisabled);
        }
    }
}

void PluginCfg::apply()
{
}

#ifndef WIN32
#include "plugincfg.moc"
#endif


